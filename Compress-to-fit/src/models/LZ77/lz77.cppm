export module lz77;

export import util;
import std.compat;

/**
* @brief This is the type LZ77 will use for compressed symbols
*/
struct Token
{
	uint16_t offset;
	uint8_t length;
	Sym symbol;
};


inline constexpr int SEARCH_RATIO = 100;//Ratio of the search buf size and la buf
inline constexpr auto MAX_WINDOW_SIZE = KB_to_B(256);

/**
 * @brief [lz77.Window]
 * @brief This class gives a window to data given to the ctor
 * 
 * 
 * @file src/models/LZ77/lz7.cppm
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
			throw std::runtime_error("There has been an exception in the file " + std::string{ __FILE__ } + " at the line " + std::to_string(__LINE__));
			break;
		default:
			throw std::runtime_error("There has been an exception in the file " + std::string{ __FILE__ } + " at the line " + std::to_string(__LINE__));
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

	auto look_ahead_buffer() -> std::span<Sym>
	{
		return data.subspan(offset + size_search, size_look_ahead);
	}

	auto search_buffer() -> std::span<Sym>
	{
		return data.subspan(offset, size_search);
	}

	auto get_size_search() const
	{
		return size_search;
	}

	auto get_size_la_buf() const
	{
		return size_look_ahead;
	}

	auto get_relative_pos() const
	{
		return size_search;
	}

	auto& operator[](size_t index)
	{
		return data[offset + index];
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
	inline consteval uint32_t ROLL_FACTOR = const_pow(BASE, MIN_MATCH - 1);


	/**
	 * @brief [lz77.Karp-Rabin]
	 * @brief This class gives fast string matching.
	 * @brief Makes use of two chained arrays.
	 *
	 * @file src/models/LZ77/lz7.cppm
	 */
	class Rabin
	{
	public:

		Rabin(const Window& data, uint32_t pos)
			:data(data), position(pos)
		{
			for (int i = 0; i < MIN_MATCH and i < position; i++)//initialize the hash
			{
				hash = (hash * BASE * data[pos + i]) % MOD;
			}
			symbol_positions[hash].emplace_back(pos);
		}

		/**
		 * @brief Roll and add the hash to the symbol_positions
		 * @brief This function will check if position has reached the end
		 * @brief The position will be advanced
		 */
		void roll_hash()
		{
			if (position < (data.search_buffer().size() - MIN_MATCH))//stop if we reached the end
			{
				//roll hash
				hash = (hash + MOD - static_cast<uint64_t>(data[position]) * ROLL_FACTOR % MOD) % MOD;
				hash = (hash * BASE + data[position + MIN_MATCH]) % MOD;

				symbol_positions[hash].emplace_back(position);
			}

			position++;
		}

	private:
		const Window& data;
		uint32_t hash = 0;
		uint16_t position;
		uint16_t poss_table[MAX_WINDOW_SIZE];
		uint16_t prev_poss_table[MAX_WINDOW_SIZE];

	};


}//namespace algs

export class LZ77
{
public:
	
	    
	LZ77(std::span<Sym> data_, CompPreset preset_)
		:data(data_), preset(preset_), window(data_, preset_)
	{
		
	}

	std::vector<Token> compress();
	void decompress();

private:
	std::span<Sym> data;
	Window window;
	CompPreset preset;
	alg::Rabin search_table;

	/**
	* @brief Find a match for a pattern in a buffer.
	* @param buffer The searched buffer.
	* @param pattern The pattern.
	* @return An optional index to the start of the match.
	*/
	std::optional<int> search_pattern(const std::span<Sym> buffer, const std::span<Sym> pattern)
	{

	}

};


