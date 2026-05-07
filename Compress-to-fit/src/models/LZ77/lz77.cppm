module;
#include <source_location>
#include <limits>

export module lz77;

export import util;
import std.compat;
import parser;

namespace fs = std::filesystem;


/**
* @brief This is the type LZ77 will output
*/
struct Token
{
	uint16_t offset;
	uint8_t length;
	Sym symbol;

	friend std::ostream& operator<<(std::ostream& os, const Token& token)
	{
		os.write(reinterpret_cast<const char*>(&token), sizeof(Token));

		return os;
	}
};

export using LZ77_Token = Token;

inline constexpr int SEARCH_RATIO = 500;//Ratio of the search buf size and la buf
inline constexpr auto MAX_WINDOW_SIZE = KB_to_B(128);
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
	Window(const std::span<Sym const>& data_, size_t window_size) noexcept
		:data(data_), max_size(std::clamp(window_size, window_size, MAX_WINDOW_SIZE)), size_search(0)
	{
		if (max_size > data.size()) max_size = data.size();
		size_look_ahead = std::ceil(max_size / (SEARCH_RATIO + 1));
		max_size_look_ahead = size_look_ahead;
		max_size_search = SEARCH_RATIO * max_size_look_ahead;
	}

	Window(const std::span<Sym const>& data_, CompPreset preset)
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
		size_look_ahead = std::ceil(max_size / SEARCH_RATIO);
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
	void slide(size_t displacement)
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
			size_look_ahead = std::clamp(size_look_ahead - (offset + max_size - data.size()), 0ull, max_size_look_ahead);
		}

		offset = std::clamp(offset, 0ull, data.size() - max_size_search);
	}

	[[nodiscard]] auto look_ahead_buffer() noexcept -> std::span<Sym const>
	{
		return data.subspan(offset + size_search, size_look_ahead);
	}

	[[nodiscard]] auto search_buffer() noexcept -> std::span<Sym const>
	{
		return data.subspan(offset, size_search);
	}

	[[nodiscard]] auto get_size_search() const noexcept
	{
		return size_search;
	}

	[[nodiscard]] auto get_size_la_buf() const noexcept
	{
		return size_look_ahead;
	}

	[[nodiscard]] auto get_max_size() const noexcept
	{
		return max_size;
	}

	[[nodiscard]] auto get_max_size_search() const noexcept
	{
		return max_size_search;
	}

	[[nodiscard]] auto get_relative_pos() const noexcept
	{
		return size_search;
	}

	[[nodiscard]] auto get_absolute_pos() const noexcept
	{
		return size_search + offset;
	}


	auto operator[](size_t index) const noexcept
	{
		return data[index];
	}

	[[nodiscard]] const auto get_data() const noexcept //peak function signature
	{
		return data;
	}

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
	 * @brief [lz77.Karp-Rabin]
	 * @brief This class enables fast string matching.
	 * @brief Makes use of two chained arrays.
	 * @brief This class is intertwined with LZ77 and should not be used as separate entity.
	 * @brief This class shall not modify the Window given
	 *
	 * @file src/models/LZ77/lz77.cppm
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
			
			for (int i = 0; i < MIN_MATCH and i < data.get_data().size(); i++)//initialize the hash
			{
				hash = (hash * BASE * static_cast<uint32_t>(data[position + i])) % MOD;
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
		 * @details find_pattern checks #poss_table for a pattern match, once, and if, found it will keep searching
		 * down the chain of previous poss_table entries, which have been saved in #prev_poss_table by #roll_hash, an #MAX_CHAIN amount of times.
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

		[[nodiscard]] auto bucket_index(uint32_t hash_) const noexcept -> uint32_t
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

		[[nodiscard]] auto prev_bucket_index(uint32_t hash_) const noexcept -> uint32_t
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

		[[nodiscard]] auto get_max_chain_from_preset(CompPreset preset) const noexcept -> size_t
		{
			switch (preset)
			{
			case COMP_MAX:
				return 5;
				break;
			case COMP_8:
				return 5;
				break;
			case COMP_7:
				return 4;
				break;
			case COMP_6:
				return 4;
				break;
			case COMP_5:
				return 4;
				break;
			case COMP_4:
				return 3;
				break;
			case COMP_3:
				return 3;
				break;
			case COMP_2:
				return 3;
				break;
			case COMP_1:
				return 2;
				break;
			case NO_COMP:
				std::terminate();
				break;
			}
		}
	};


}//namespace algs

template <typename T>
struct CompressionData {};

template <>
struct CompressionData<Sym>
{
	Window window;
	CompPreset preset;
	alg::Rabin pattern_matcher;

	CompressionData(std::span<Sym const> data_, CompPreset preset_)
		: window(data_, preset_),
		preset(preset_),
		pattern_matcher(window, 0, preset)
	{
	}
};

template <typename T>
concept IsSym = std::is_same_v<T, Sym>;

template <typename T>
concept IsToken = std::is_same_v<T, Token>;

/**
 * @brief Implement the LZ77 algorithm.
 * @details An overview of the algorithm: 
 */
export template <typename DataType>
class LZ77 : protected CompressionData<DataType>
{
public:
	//extracting version
	LZ77(std::span<DataType const> data_, const parser::Options& options) requires IsToken<DataType>
		:data(data_), cli_options(options)
	{
	}

	LZ77(std::span<DataType const> data_, const parser::Options& options) requires IsSym<DataType>
		: CompressionData<DataType>(data_, static_cast<CompPreset>(options.preset)),
		cli_options(options),
		data(data_)
	{
	}


	/**
	 * @brief Compress the data up to max_index
	 * @param out_stream Where the output will go
	 * @param max_index The last index to get compressed (included)
	 * 
	 * @warning #out_stream will be cleared and overwritten
	 * 
	 * @details This function shall be used as a compression "in steps". Its use is to stop the #out_stream from being too big.
	 *			The function shall not strictly stop at #max_index, due to the nature of the LZ77 algorithm.
	 */
	void compress(CodecInterface<Sym, LZ77_Token>& interface) requires IsSym<DataType>;

	/**
	 * @brief Decompress #data.
	 * @param out_stream Where the output will go
	 * @param max_index The last index to get decompressed (included)
	 * 
	 * @warning #out_stream will be cleared and overwritten
	 * 
	 * @details This function shall be used as a compression "in steps". Its use is to stop the #out_stream from being too big.
	 */
	void decompress(CodecInterface<LZ77_Token, Sym>& interface) requires IsToken<DataType>;

private:
	const parser::Options& cli_options;
	std::span<DataType const> data;
};

template <typename DataType>
void LZ77<DataType>::compress(CodecInterface<Sym, LZ77_Token>& interface) requires IsSym<DataType>
{

	interface.out_data.clear();
	interface.out_data.reserve(SIZE_CHUNK);

	size_t local_index = 0;
	//loop over the stream
	while (!interface.in_data.reached_end() and local_index < SIZE_CHUNK)
	{
		auto token = this->pattern_matcher.find_pattern();

		auto num_of_rolls = std::max(static_cast<int>(token.length), 1);
		this->pattern_matcher.roll_hash(num_of_rolls);//roll, updates the hash and position
		this->window.slide(num_of_rolls);
		interface.in_data += num_of_rolls;
		local_index += num_of_rolls;

		interface.out_data.push_back(token);
	}
}

template<typename DataType>
void LZ77<DataType>::decompress(CodecInterface<LZ77_Token, Sym>& interface) requires IsToken<DataType>
{
	interface.out_data.reserve(interface.in_data.distance_to_end());
	while (!interface.in_data.reached_end())
	{
		if (interface.in_data.iterator->offset != 0)
		{
			for (size_t remaining_syms = interface.in_data.iterator->length; remaining_syms > 0; remaining_syms--)
				interface.out_data.push_back(interface.out_data[interface.out_data.size() - interface.in_data.iterator->offset]);
		}
		interface.out_data.emplace_back(interface.in_data.iterator->symbol);
		interface.in_data++;
	}
}