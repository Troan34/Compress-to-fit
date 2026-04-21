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

inline constexpr int SEARCH_RATIO = 500;//Ratio of the search buf size and la buf
inline constexpr auto MAX_WINDOW_SIZE = KB_to_B(128);
/**
 * @brief The size of @ref LZ77::poss_table
 */
inline constexpr auto MAX_HASH_SIZE = MAX_WINDOW_SIZE << 1;

/**
 * @brief [lz77.Window]
 * @brief This class gives a window to data given to the ctor
 */
class Window
{
public:
	Window(const std::span<Sym>& data_, size_t window_size) noexcept
		:data(data_), max_size(std::clamp(window_size, window_size, MAX_WINDOW_SIZE)), size_search(0)
	{
		if (max_size > data.size()) max_size = data.size();
		size_look_ahead = std::ceil(max_size / (SEARCH_RATIO + 1));
		max_size_look_ahead = size_look_ahead;
		max_size_search = SEARCH_RATIO * max_size_look_ahead;
	}

	Window(const std::span<Sym>& data_, CompPreset preset)
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
			throw std::runtime_error("There has been an exception in the file " + std::string{ std::source_location::current().file_name() } + " at the line " + std::string{ std::source_location::current().line()});
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

	[[nodiscard]] auto look_ahead_buffer() noexcept -> std::span<Sym>
	{
		return data.subspan(offset + size_search, size_look_ahead);
	}

	[[nodiscard]] auto search_buffer() noexcept -> std::span<Sym>
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

	[[nodiscard]] auto get_relative_pos() const noexcept
	{
		return size_search;
	}

	[[nodiscard]] auto get_absolute_pos() const noexcept
	{
		return size_search + offset;
	}

	auto& operator[](size_t index) noexcept
	{
		return data[offset + index];
	}

	auto operator[](size_t index) const
	{
		return data[index];
	}

	[[nodiscard]] const auto get_data() const noexcept //peak function signature
	{
		return data;
	}

private:
	const std::span<Sym> data;
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
				hash = (hash * BASE * data[position + i]) % MOD;
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

/**
 * @brief Implement the LZ77 algorithm.
 * @details An overview of the algorithm: 
 */
export class LZ77
{
public:
	using Token = Token;//share token type
	
	
	LZ77(std::span<Sym> data_, const parser::Options& options)
		:data(data_), preset(static_cast<CompPreset>(options.preset)), window(data_, static_cast<CompPreset>(options.preset)), pattern_matcher(window, 0, preset), cli_options(options)
	{
	}

	/**
	 * @brief Compress the data.
	 * @return The resulting data.
	 */
	std::vector<Token> compress();

	/**
	 * @brief Compress the data up to max_index
	 * @param max_index The last index to get compressed (included)
	 * @return The resulting data.
	 * 
	 * @details This function shall be used as a compression "in steps". Its use is to stop the return type from being too big.
	 *			The function shall not strictly stop at #max_index, due to the nature of the LZ77 algorithm.
	 */
	std::vector<Token> compress(size_t max_index);

	void decompress();

private:
	const parser::Options& cli_options;
	std::span<Sym> data;
	Window window;
	CompPreset preset;
	alg::Rabin pattern_matcher;
};


