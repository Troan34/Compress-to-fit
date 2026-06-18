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
		:data(data_), size_search(0), max_size(std::clamp(window_size, window_size, MAX_WINDOW_SIZE))
	{
		if (max_size > data.size()) max_size = data.size();
		size_look_ahead = std::ceil(max_size / (SEARCH_RATIO + 1));
		max_size_look_ahead = size_look_ahead;
		max_size_search = SEARCH_RATIO * max_size_look_ahead;
	}

	Window(const std::span<Sym const>& data_, CompPreset const preset)
		:data(data_)
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
		size_look_ahead = std::ceil(max_size / static_cast<double>(SEARCH_RATIO));
		max_size_look_ahead = size_look_ahead;
		max_size_search = SEARCH_RATIO * max_size_look_ahead;
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
	[[nodiscard]] auto operator[](size_t const index) const noexcept
	{
		assert(index < max_size);
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
	 * @brief Makes use of two chained arrays.
	 * @brief This class is intertwined with LZ77 and should not be used as separate entity.
	 * @brief This class shall not modify the Window given
	 *
	 * @file lz77.cppm
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

		Rabin& operator=(const Rabin&) = default;

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
class LZ77Block
{
public:

	explicit LZ77Block(std::span<std::byte const> const data_, fs::path const& file) noexcept;
	LZ77Block() = default;

	/**
	 * @brief Convert the block into a byte buffer
	 * @param output output buffer
	 */
	void serialize(std::vector<std::byte>& output);

	void write_to(std::ofstream& output);

	void push_back(LZ77_Token const token)
	{
		compressed_length_ += sizeof(LZ77_Token);
		uncompressed_length_ += token.length + sizeof(decltype(LZ77_Token::symbol));
		block.push_back(token);
	}

	void clear() noexcept
	{
		block.clear();
		compressed_length_ = 0;
		uncompressed_length_ = 0;
	}

	constexpr auto reserve(size_t const size) { block.reserve(size);}

	[[nodiscard]] constexpr auto size() const noexcept { return block.size(); }
	[[nodiscard]] constexpr auto size_bytes() const noexcept -> size_t { return block.size() * sizeof(LZ77_Token); }
	[[nodiscard]] constexpr auto compressed_length() const noexcept {return compressed_length_;}
	[[nodiscard]] constexpr auto uncompressed_length() const noexcept {return uncompressed_length_;}
	[[nodiscard]] constexpr auto const& operator[](size_t const index) const noexcept {return block[index];}
	[[nodiscard]] constexpr auto begin() const noexcept {return block.begin();}
	[[nodiscard]] constexpr auto end() const noexcept {return block.end();}
	//setter
	[[nodiscard]] constexpr auto& operator[](size_t const index) noexcept
	{
		assert(index < block.size());
		return block[index];
	}

private:
	uint32_t compressed_length_{};
	uint32_t uncompressed_length_{};
	std::vector<LZ77_Token> block{};
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
	void compress(LZ77Block& out_data);

private:
	std::span<Sym const> const data_{};
	Window window;
	alg::Rabin pattern_matcher;


};

class LZ77Decompressor
{
public:
	explicit LZ77Decompressor(LZ77Block const& data)
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
				for (auto _ : std::views::iota(0u, token.length))
					out_data.push_back(out_data[out_data.size() - token.offset]);
			}
			out_data.emplace_back(token.symbol);
		}
	}

private:
	LZ77Block const& data_;
};

export class LZ77ConcurrentCompressor
{
public:
	LZ77ConcurrentCompressor(std::span<std::byte const> const data, size_t const concurrency, File& file, CompPreset const preset) noexcept(false)
		: preset_(preset), concurrency_(concurrency), data_(data), file_list_(file), thread_pool(concurrency)
	{
		n_blocks_ = data_.size_bytes() / std::min(data_.size_bytes() / concurrency, MAX_BLOCK_SIZE);
		if (data_.size_bytes() % MAX_BLOCK_SIZE > 0)
			n_blocks_++;

		results.reserve(n_blocks_);
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

			thread_pool.add_task([this, data_for_task, seq_num]//'this' shall be strictly used to access const members, or concurrency capable containers
			{
				auto block = std::make_unique<LZ77Block>(data_for_task, file_list_.file().get_in_file_options().path);
				block->reserve(data_for_task.size());
				LZ77Compressor compressor{{reinterpret_cast<Sym const*>(data_for_task.data()), (data_for_task.size_bytes() / sizeof(Sym))}, preset_};
				compressor.compress(*block);
				file_list_.insert(std::move(block), seq_num);
			});

			check_results();
			show_progress({}, static_cast<double>(n_completed_blocks) / static_cast<double>(n_blocks_), true);
			std::this_thread::sleep_for(1ms);
		}
	}

private:
	CompPreset const preset_;
	size_t const concurrency_;
	std::span<std::byte const> const data_{};
	std::vector<std::future<void>> results{};
	size_t n_blocks_{};
	size_t n_completed_blocks{};
	//These have to stay down here (I just found out how objects are destroyed)
	ConcOrderedFileList<LZ77Block> file_list_;
	ThreadPool thread_pool{};


	void check_results()
	{
		std::erase_if(results,
		[this](std::future<void>& result)
			{
				auto const status = result.wait_for(std::chrono::nanoseconds::zero());
				if (status == std::future_status::ready)
				{
					result.get();
					n_completed_blocks++;
					return true;
				}
				else
					return false;
			}
		);
	}
};

export class LZ77ConcurrentDecompressor
{
public:
	LZ77ConcurrentDecompressor(std::span<std::byte const> const data, File& file, size_t const concurrency)
		: data_(data), concurrency_(concurrency), file_(file), file_list_(file), thread_pool(concurrency)
	{

	}

	/**
	 * @brief Decompress the data.
	 *
	 * @details The decompression is achieved by considering the first index of the data as the start of the first block.
	 *			We then check the compressed_size for this block and advance that much, by doing this we know the start of the new block.
	 */
	void decompress() noexcept(false)
	{
		for (size_t i{}; i < data_.size();)
		{
			auto data_for_task = LZ77Block{data_.subspan(i), file_.get_in_file_options().path};
			i += sizeof(std::invoke_result_t<decltype(&LZ77Block::uncompressed_length), LZ77Block>) +
				sizeof(std::invoke_result_t<decltype(&LZ77Block::compressed_length), LZ77Block>) +
				data_for_task.compressed_length();

			thread_pool.add_task(
				[this, data = std::move(data_for_task)]
				{
					LZ77Decompressor decompressor{data};

					std::pmr::polymorphic_allocator<Sym> allocator{shared_memory_pool.get()};
					std::pmr::vector<Sym> buffer{allocator};
					buffer.reserve(data.uncompressed_length());

					decompressor.decompress(buffer);
				});

		}

	}



private:
	std::span<std::byte const> const data_{};
	size_t const concurrency_{};
	std::vector<std::future<void>> results{};
	size_t n_blocks_{};
	size_t n_completed_blocks{};

	//IN THIS ORDER

	std::shared_ptr<std::pmr::synchronized_pool_resource> shared_memory_pool = std::make_shared<std::pmr::synchronized_pool_resource>();
	File const& file_;
	ConcOrderedFileList<std::byte> file_list_;
	ThreadPool thread_pool{};

	void check_results()
	{
		std::erase_if(results,
		[this](std::future<void>& result)
			{
				auto const status = result.wait_for(std::chrono::nanoseconds::zero());
				if (status == std::future_status::ready)
				{
					result.get();
					n_completed_blocks++;
					return true;
				}
				else
					return false;
			}
		);
	}

};