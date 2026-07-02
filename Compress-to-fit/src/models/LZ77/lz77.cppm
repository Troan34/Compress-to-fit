module;
#include <cassert>
#include <cstddef>
export module models:lz77;

#if defined(__INTELLISENSE__)
#include "../../../for_intellisense/everything.hpp"
#endif

export import util;
import std.compat;
import parser;
import containers;

namespace fs = std::filesystem;
using namespace std::chrono_literals;


#pragma pack(push, 1)
/**
* @brief This is the type LZ77 will output
*/
struct Token
{
	uint16_t offset{};
	uint8_t length{};
	Sym symbol{};

	using SerializedBuffer = std::array<std::byte, sizeof(decltype(offset)) + sizeof(decltype(length)) + sizeof(decltype(symbol))>;


	friend std::ostream& operator<<(std::ostream& os, const Token& token)
	{
		os.write(reinterpret_cast<const char*>(&token), sizeof(Token));

		return os;
	}

	void serialize(SerializedBuffer& buffer) noexcept
	{
		std::memcpy(buffer.data(), &offset, sizeof(decltype(offset)));
		std::memcpy(buffer.data() + sizeof(decltype(offset)), &length, sizeof(decltype(length)));
		std::memcpy(buffer.data() + sizeof(decltype(offset)) + sizeof(decltype(length)), &symbol, sizeof(Sym));
	}

};
#pragma pack(pop)

export using LZ77_Token = Token;

inline constexpr int SEARCH_RATIO = 500;//Ratio of the search buf size and la buf
inline constexpr auto MAX_WINDOW_SIZE = KiB_to_B(128);
/**
 * @brief The size of @ref alg::Rabin::poss_table
 */
inline constexpr auto MAX_HASH_SIZE = MAX_WINDOW_SIZE << 1;

/**
 * @brief [lz77.Window]
 * @brief This class gives a window to data given to the ctor
 */
class Window
{
public:
	Window(const std::span<Sym const>& data_, size_t const window_size) noexcept
		: data(data_), size_search(0), max_size(std::clamp(window_size, window_size, MAX_WINDOW_SIZE))
	{
		if (max_size > data.size()) max_size = data.size();
		size_look_ahead = std::floor(max_size / (SEARCH_RATIO + 1));
		max_size_look_ahead = size_look_ahead;
		max_size_search = SEARCH_RATIO * max_size_look_ahead;//TODO: BUG: MAX_SIZE_SEARCH GOES OUTSIDE OF RANGE
	}

	Window(const std::span<Sym const>& data_, CompPreset const preset)
		: data(data_)
	{
		size_search = 0;
		switch (preset)
		{
		case COMP_MAX:
			max_size = MAX_WINDOW_SIZE;
			break;
		case COMP_8:
			max_size = MAX_WINDOW_SIZE >> 1;
			break;
		case COMP_7:
			max_size = MAX_WINDOW_SIZE >> 2;
			break;
		case COMP_6:
			max_size = MAX_WINDOW_SIZE >> 3;
			break;
		case COMP_5:
			max_size = MAX_WINDOW_SIZE >> 4;
			break;
		case COMP_4:
			max_size = MAX_WINDOW_SIZE >> 5;
			break;
		case COMP_3:
			max_size = MAX_WINDOW_SIZE >> 6;
			break;
		case COMP_2:
			max_size = MAX_WINDOW_SIZE >> 7;
			break;
		case COMP_1:
			max_size = MAX_WINDOW_SIZE >> 8;
			break;
		case NO_COMP:
			throw std::runtime_error("There has been an exception in the file " + std::string{ std::source_location::current().file_name() } + " at the line " + std::string{ std::source_location::current().line() });
			break;
		default:
			throw std::runtime_error("There has been an exception in the file " + std::string{ std::source_location::current().file_name() } + " at the line " + std::string{ std::source_location::current().line() });
			break;
		}
		if (max_size > data.size()) max_size = data.size();
		size_look_ahead = std::floor(max_size / (SEARCH_RATIO + 1));
		max_size_look_ahead = size_look_ahead;
		max_size_search = SEARCH_RATIO * max_size_look_ahead;//TODO: BUG: MAX_SIZE_SEARCH GOES OUTSIDE OF RANGE
	}


	auto begin() const noexcept
	{
		return data.begin() + offset;
	}

	auto end() const noexcept
	{
		return data.begin() + offset + size_search + size_look_ahead + 1;
	}

	/**
	 * @brief Slide the window
	 * @param displacement The offset(or sizes) will moved by this amount
	 */
	void slide(size_t const displacement)
	{
		if (size_search < max_size_search)//size_search should grow
		{
			size_search += displacement;
			if (size_search > max_size_search)//size_search has grown past maximum
			{
				offset += size_search - max_size_search;
				size_search = max_size_search;
			}
		}
		else
		{
			offset += displacement;
		}

		if (offset + max_size > data.size())//we have to cut off size_look_ahead
		{
			size_look_ahead = std::clamp(size_look_ahead - (offset + max_size - data.size()), 0ul, max_size_look_ahead);
		}

		offset = std::clamp(offset, 0ul, data.size() - max_size_search);
	}

	[[nodiscard]] auto look_ahead_buffer() const noexcept -> std::span<Sym const>{return data.subspan(offset + size_search, size_look_ahead);}
	[[nodiscard]] auto search_buffer() const noexcept -> std::span<Sym const>{return data.subspan(offset, size_search);}
	[[nodiscard]] auto get_size_search() const noexcept{return size_search;}
	[[nodiscard]] auto get_size_la_buf() const noexcept{return size_look_ahead;}
	[[nodiscard]] auto get_max_size() const noexcept{return max_size;}
	[[nodiscard]] auto get_max_size_search() const noexcept{return max_size_search;}
	[[nodiscard]] auto get_relative_pos() const noexcept{return size_search;}
	[[nodiscard]] auto get_absolute_pos() const noexcept{return size_search + offset;}
	[[nodiscard]] auto get_offset() const noexcept{return offset;}
	[[nodiscard]] auto operator[](size_t const index) const noexcept
	{
		assert(index < data.size());
		return data[index];
	}

	[[nodiscard]] auto get_data() const noexcept{return data;}

private:
	const std::span<const Sym> data;
	size_t size_look_ahead;
	size_t max_size_look_ahead;
	size_t size_search;
	size_t max_size_search;
	size_t max_size;
	size_t offset = 0;
};

namespace alg
{
	inline constexpr uint32_t MOD = (1u << 31) - 1;
	inline constexpr uint32_t BASE = Sym::alphabet_size();
	inline constexpr uint32_t MIN_MATCH = 3;
	inline constexpr uint32_t MAX_MATCH = 250;
	inline constexpr uint32_t PRIME = 31;
	inline constexpr uint32_t ROLL_FACTOR = const_pow(BASE, MIN_MATCH - 1);


	/**
	 * @brief This class enables fast string matching.
	 * 		  Makes use of two chained arrays.
	 * 		  This class is intertwined with LZ77 and should not be used as separate entity.
	 * 		  This class shall not modify the Window given
	 */
	class Rabin
	{
	public:

		/**
		 * @pre @p data.max_size() > #MAX_HASH_SIZE
		 */
		Rabin(const Window& data, uint32_t pos, CompPreset preset)
			:data(data), position(pos), MAX_CHAIN(get_max_chain_from_preset(preset))
		{
			//compiler will probably use a memset-like optimization
			for (int i = 0; i < MAX_HASH_SIZE; i++)
				poss_table[i] = -1;

			for (int i = 0; i < MAX_WINDOW_SIZE; i++)
				prev_poss_table[i] = -1;
			
			hash = data[0];
			for (int i = 1; i < MIN_MATCH and i < data.get_data().size(); i++)//initialize the hash
			{
				hash = (static_cast<uint64_t>(hash) * BASE + data[position + i]) % MOD;
			}
			poss_table[bucket_index(hash)] = position;
			
		}

		Rabin& operator=(const Rabin&) = delete;

		/**
		 * @brief Roll and add the hash to the symbol_positions
		 * @brief This function will check if position has reached the end
		 * @warning The position will be increased. THE WINDOW WILL NOT SLIDE
		 * @param num_of_rolls How many times it should roll, defaults to 1
		 * 
		 * @details find_pattern checks #poss_table for a pattern match, if found it will keep searching down the chain of previous poss_table entries,
		 *			which have been saved in #prev_poss_table by #roll_hash. This is repeated a #MAX_CHAIN amount of times.
		 */
		void roll_hash(uint32_t num_of_rolls = 1) noexcept;


		/**
		 * @brief Find the biggest pattern between the look ahead buffer and search buffer
		 */
		Token find_pattern();


		[[nodiscard]] auto get_pos() const noexcept
		{
			return position;
		}

	private:
		const Window& data;
		uint32_t hash = 0;
		uint32_t position;
		int32_t poss_table[MAX_HASH_SIZE];
		int32_t prev_poss_table[MAX_WINDOW_SIZE];
		const size_t MAX_CHAIN;

		[[nodiscard]] static auto bucket_index(uint32_t const hash_) noexcept -> uint32_t
		{
			if constexpr (std::popcount(MAX_HASH_SIZE) == 1)
			{
				return hash_ & (MAX_HASH_SIZE - 1);
			}
			else
			{
				return hash_ % MAX_HASH_SIZE;
			}
		}

		[[nodiscard]] static auto prev_bucket_index(uint32_t const hash_) noexcept -> uint32_t
		{
			if constexpr (std::popcount(MAX_WINDOW_SIZE) == 1)
			{
				return hash_ & (MAX_WINDOW_SIZE - 1);
			}
			else
			{
				return hash_ % MAX_WINDOW_SIZE;
			}
		}

		[[nodiscard]] static auto get_max_chain_from_preset(CompPreset const preset) noexcept -> size_t
		{
			switch (preset)
			{
			case COMP_MAX:
			case COMP_8:
				return 5;
				break;
			case COMP_7:
			case COMP_6:
			case COMP_5:
				return 4;
				break;
			case COMP_4:
			case COMP_3:
			case COMP_2:
				return 3;
				break;
			case COMP_1:
				return 2;
				break;
			case NO_COMP:
				assert(false);
				break;
			}
		}
	};


}


constexpr size_t MAX_BLOCK_SIZE = 4_MiB;

/**
 * @brief Represents a block of data encoded by LZ77.
 */
template <typename Alloc = std::allocator<LZ77_Token>>
class LZ77Block
{
public:
	using value_type = LZ77_Token;

	explicit LZ77Block(std::span<std::byte const> const data_, fs::path const& file, Alloc const& alloc = Alloc())
		: buffer_(alloc)
	{
		constexpr size_t len1 = sizeof(decltype(compressed_length_));
		constexpr size_t len2 = sizeof(decltype(uncompressed_length_));

		std::memcpy(&compressed_length_, data_.data(), len1);//get compressed_length from data
		std::memcpy(&uncompressed_length_, data_.data() + len1, len2);//get uncompressed_length from data

		if (len1 + len2 + compressed_length_ > data_.size())
			throw_error(ErrorType::FILE_CORRUPTED, "An lz77 header is corrupted in file \"" + file.string() + "\"");

		buffer_.reserve(data_.size());
		buffer_ = std::vector<LZ77_Token>{
			reinterpret_cast<LZ77_Token const*>(data_.data() + len1 + len2),
			reinterpret_cast<LZ77_Token const*>(data_.data() + len1 + len2 +  compressed_length_)
		};
	}

	LZ77Block(Alloc const& alloc = Alloc()) : buffer_(alloc) {}

	/**
	 * @brief Convert the block into a byte buffer
	 * @param output output buffer
	 */
	void serialize(std::vector<std::byte>& output)
	{
		constexpr size_t len1 = sizeof(decltype(compressed_length_));
		constexpr size_t len2 = sizeof(decltype(uncompressed_length_));

		output.resize(len1 + len2 + buffer_.size() * sizeof(LZ77_Token));

		std::memcpy(output.data(), &compressed_length_, len1);
		std::memcpy(output.data() + len1, &uncompressed_length_, len2);

		std::memcpy(output.data() + len1 + len2, buffer_.data(), buffer_.size() * sizeof(LZ77_Token));
	}

	void write_to(std::ofstream& output) const
	{
		constexpr size_t len1 = sizeof(decltype(compressed_length_));
		constexpr size_t len2 = sizeof(decltype(uncompressed_length_));

		output.write(reinterpret_cast<char const*>(&compressed_length_), len1);
		output.write(reinterpret_cast<char const*>(&uncompressed_length_), len2);
		output.write(reinterpret_cast<char const*>(buffer_.data()), buffer_.size() * sizeof(typename decltype(buffer_)::value_type));
	}

	void push_back(LZ77_Token const token)
	{
		compressed_length_ += sizeof(LZ77_Token);
		uncompressed_length_ += token.length + sizeof(decltype(LZ77_Token::symbol));
		buffer_.push_back(token);
	}

	void clear() noexcept
	{
		buffer_.clear();
		compressed_length_ = 0;
		uncompressed_length_ = 0;
	}

	constexpr auto reserve(size_t const size) { buffer_.reserve(size);}

	[[nodiscard]] constexpr auto size() const noexcept { return buffer_.size(); }
	[[nodiscard]] constexpr auto size_bytes() const noexcept -> size_t { return buffer_.size() * sizeof(LZ77_Token); }
	[[nodiscard]] constexpr auto compressed_length() const noexcept {return compressed_length_;}
	[[nodiscard]] constexpr auto uncompressed_length() const noexcept {return uncompressed_length_;}
	[[nodiscard]] constexpr auto const& operator[](size_t const index) const noexcept {return buffer_[index];}
	[[nodiscard]] constexpr auto begin() const noexcept {return buffer_.begin();}
	[[nodiscard]] constexpr auto end() const noexcept {return buffer_.end();}

	template <typename Self>
	auto&& buffer(this Self& self) noexcept
	{
		return std::forward<Self>(self).buffer_;
	}


	//setter
	[[nodiscard]] constexpr auto& operator[](size_t const index) noexcept
	{
		assert(index < buffer_.size());
		return buffer_[index];
	}

private:
	uint32_t compressed_length_{};
	uint32_t uncompressed_length_{};
	std::vector<LZ77_Token, Alloc> buffer_;
};


class LZ77Compressor
{
public:
	LZ77Compressor(std::span<Sym const> const data, CompPreset const preset)
		:data_(data), window(data, preset), pattern_matcher(window, 0, preset)
	{}

	/**
	 * @brief Compress the data received in the constructor
	 * @param out_data Where the output will go
	 *
	 * @note Will clear out_data block
	 */
	template <typename Alloc>
	void compress(LZ77Block<Alloc>& out_data)
	{
		out_data.clear();
		out_data.reserve(data_.size() / sizeof(LZ77_Token));

		size_t consumed_tokens{};
		//loop over the stream
		for (auto i = data_.begin(); i != data_.end();)
		{
			auto const token = this->pattern_matcher.find_pattern();

			auto const num_of_rolls = token.length + 1;
			this->pattern_matcher.roll_hash(num_of_rolls);//roll, updates the hash and position
			this->window.slide(num_of_rolls);
			i += num_of_rolls;
			consumed_tokens += num_of_rolls;

			out_data.push_back(token);
		}
	}

private:
	std::span<Sym const> const data_{};
	Window window;
	alg::Rabin pattern_matcher;


};

template<typename Alloc = std::allocator<LZ77Block<>::value_type>>
class LZ77Decompressor
{
public:
	explicit LZ77Decompressor(LZ77Block<Alloc> const& data)
		: data_(data) {}

	/**
	 * @brief Decompress the data received in the constructor
	 * @param out_data Where the output will go
	 *
	 * @note Will clear out_data buffer
	 */
	template <typename Allocator = std::allocator<Sym>>
	void decompress(std::vector<Sym, Allocator>& out_data) const
	{
		out_data.reserve(data_.size() / sizeof(Sym));
		for (auto const token : data_)
		{
			if (token.offset != 0)
			{
				auto size = out_data.size();
				for (auto _ : std::views::iota(0u, token.length))
					out_data.push_back(out_data[size - token.offset]);
			}
			out_data.emplace_back(token.symbol);
		}
		assert(out_data.capacity() == out_data.size());
	}

private:
	LZ77Block<Alloc> const& data_;
};



export class LZ77ConcurrentCompressor
{
public:
	LZ77ConcurrentCompressor(std::span<std::byte const> const data, size_t const concurrency, File& file, CompPreset const preset) noexcept(false)
		: preset_(preset), concurrency_(concurrency), data_(data), file_list_(file), thread_pool(concurrency)
	{
		n_blocks_ = data_.size_bytes() / std::min(data_.size_bytes() / concurrency, MAX_BLOCK_SIZE);
		if (data_.size_bytes() % MAX_BLOCK_SIZE > 0)
			++n_blocks_;

		results.reserve(n_blocks_);
	}

	~LZ77ConcurrentCompressor()
	{
		std::unique_lock lock{mutex_};
		for (auto& result : results)
		{
			result.wait();
			result.get();
		}
	}

	/**
	 * @brief Run the compression
	 */
	void compress() noexcept(false)
	{
		auto const partition_size = std::min(data_.size() / concurrency_, MAX_BLOCK_SIZE);
		for (size_t completed_size = 0, seq_num{}; completed_size < data_.size(); completed_size += partition_size, seq_num++)
		{
			//get the true partition size, i.e. the partition can be cut off at the eof
			auto const true_partition_size = [completed_size, partition_size, this]
			{
				if (completed_size + partition_size > data_.size())
					return data_.size() - completed_size;
				else
					return partition_size;
			}();

			auto const data_for_task = data_.subspan(completed_size, true_partition_size);

			auto future = thread_pool.add_task([this, data_for_task, seq_num]//'this' shall be strictly used to access const members, or concurrency capable containers
			{
				auto block = std::make_unique<LZ77Block<>>();
				block->reserve(data_for_task.size());
				LZ77Compressor compressor{{reinterpret_cast<Sym const*>(data_for_task.data()), (data_for_task.size_bytes() / sizeof(Sym))}, preset_};
				compressor.compress(*block);
				file_list_.insert(std::move(block), seq_num);
			});

			std::unique_lock lock{mutex_};
			results.emplace_back(std::move(future));
		}
	}

private:
	CompPreset const preset_;
	size_t const concurrency_;
	std::span<std::byte const> const data_{};
	std::vector<std::future<void>> results{};
	std::atomic<size_t> n_blocks_{};
	size_t n_completed_blocks{};
	std::mutex mutex_;//to be used to access results
	std::jthread progress_checker{&LZ77ConcurrentCompressor::check_results, this};
	//These have to stay down here (I just found out how objects are destroyed)
	ConcOrderedFileList<LZ77Block<>> file_list_;
	ThreadPool thread_pool{};


	void check_results(std::stop_token stop)
	{
		while (!stop.stop_requested())
		{
			std::this_thread::sleep_for(2ms);
			std::unique_lock lock{mutex_};
			std::erase_if(results,
			[this](std::future<void>& result)
				{
					if (result.valid())
					{
						result.get();
						n_completed_blocks++;
						return true;
					}
					else
						return false;
				}
			);
			show_progress({}, static_cast<float>(n_completed_blocks) / static_cast<float>(n_blocks_), true);
		}

		show_progress({}, 1.F, true);
	}
};



export class LZ77ConcurrentDecompressor
{
public:
	LZ77ConcurrentDecompressor(std::span<std::byte const> const data, File& file, size_t const concurrency)
		: data_(data), concurrency_(concurrency), file_(file), file_list_(file), thread_pool(concurrency)
	{

	}

	~LZ77ConcurrentDecompressor()
	{
		std::unique_lock lock{mutex_};
		for (auto& result : results)
		{
			result.wait();
			result.get();
		}
	}

	/**
	 * @brief Decompress the data.
	 *
	 * @details The decompression is achieved by considering the first index of the data as the start of the first block.
	 *			We then check the compressed_size for this block and advance that much, by doing this we know the start of the new block.
	 */
	void decompress() noexcept(false)
	{
		for (size_t i{}, seq_n{}; i < data_.size(); seq_n++)
		{
			auto data_for_task = LZ77Block{data_.subspan(i), file_.get_in_file_options().path};
			i += sizeof(std::invoke_result_t<decltype(&LZ77Block<>::uncompressed_length), LZ77Block<>>) +
				sizeof(std::invoke_result_t<decltype(&LZ77Block<>::compressed_length), LZ77Block<>>) +
				data_for_task.compressed_length();

			auto future = thread_pool.add_task(
				[this, data = std::move(data_for_task), seq_n]
				{
					LZ77Decompressor decompressor{data};

					auto buffer = std::make_unique<std::vector<Sym>>();
					buffer->reserve(data.uncompressed_length());

					decompressor.decompress(*buffer);

					file_list_.insert(std::move(buffer), seq_n);
				});

			std::unique_lock lock{mutex_};
			results.emplace_back(std::move(future));
		}

	}



private:
	std::span<std::byte const> const data_{};
	size_t const concurrency_{};
	std::vector<std::future<void>> results{};
	size_t n_blocks_{};
	size_t n_completed_blocks{};
	std::mutex mutex_;//to be used to access results

	//IN THIS ORDER
	std::jthread progress_checker{&LZ77ConcurrentDecompressor::check_results, this};
	File const& file_;
	ConcOrderedFileList<std::vector<Sym>> file_list_;
	ThreadPool thread_pool{};


	void check_results(std::stop_token stop)
	{
		while (!stop.stop_requested())
		{
			std::this_thread::sleep_for(2ms);
			std::unique_lock lock{mutex_};
			std::erase_if(results,
			[this](std::future<void>& result)
				{
					if (result.valid())
					{
						result.get();
						n_completed_blocks++;
						return true;
					}
					else
						return false;
				}
			);
			show_progress({}, static_cast<float>(n_completed_blocks) / static_cast<float>(n_blocks_), false);
		}

		show_progress({}, 1.F, false);
	}

};