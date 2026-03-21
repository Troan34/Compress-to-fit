export module lz77;

export import util;
import std.compat;

namespace alg
{
	inline constexpr unsigned int MOD = 1e9 + 7;
	inline constexpr unsigned int BASE = 31;

	struct Hash
	{
		int hash_val;

		Hash(int num) : hash_val(num)
		{
		}

		Hash() = default;


		void operator+=(int operand) noexcept
		{
			hash_val += operand;
			if (hash_val >= MOD) hash_val -= MOD;
		}

		void operator-=(int operand) noexcept
		{
			hash_val -= operand;
			if (hash_val < 0) hash_val += MOD;
		}

		void operator*=(int operand) noexcept
		{
			hash_val *= operand;
			hash_val %= MOD;
		}

		auto operator+(int operand) noexcept
		{
			auto num = hash_val + operand;
			if (num >= MOD) num -= MOD;
			return num;
		}

		auto operator-(int operand) noexcept
		{
			auto num = hash_val - operand;
			if (num < 0) num += MOD;
			return num;
		}

		auto operator*(int operand) noexcept
		{
			auto num = hash_val * operand;
			num %= MOD;
			return num;
		}

		auto operator*(const Hash& operand) noexcept
		{
			return hash_val * operand.hash_val % MOD;
		}


		auto operator<=>(const Hash&) const = default;
	};

	class Karp
	{
	private:

		std::vector<Hash> hashes;
		std::vector<Hash> powers;

	public:

		Karp(const std::span<Sym>& data)
		{
			hashes.resize(data.size());
			powers.resize(data.size());

			hashes[0] = data[0].value;
			powers[0] = 1;

			for (int i = 1; i < data.size(); i++)
			{
				hashes[i] = hashes[i - 1] * BASE + data[i].value;
				powers[i] = powers[i - 1] * BASE;
			}
		}

		auto get_hash(int start, int end)
		{
			auto hash = hashes[end];
			if (start > 0)
			{
				hash = hash - (hashes[start - 1] * powers[end - start + 1]);
			}
			return hash;
		}


	};


}//namespace algs

class Window
{
public:

	Window(const std::span<Sym>& data_, size_t window_size) noexcept
		:data(data_), size(window_size), size_search(0), size_look_ahead(std::ceil(window_size / 2))
	{
	}

	Window(const std::span<Sym>& data_, CompPreset preset) noexcept
		:data(data_)
	{
		switch (preset)
		{
		case COMP_MAX:
			size = KB_to_B(256);
			break;
		case COMP_8:
			size = KB_to_B(224);
			break;
		case COMP_7:
			size = KB_to_B(192);
			break;
		case COMP_6:
			size = KB_to_B(160);
			break;
		case COMP_5:
			size = KB_to_B(128);
			break;
		case COMP_4:
			size = KB_to_B(96);
			break;
		case COMP_3:
			size = KB_to_B(64);
			break;
		case COMP_2:
			size = KB_to_B(32);
			break;
		case COMP_1:
			size = KB_to_B(16);
			break;
		case NO_COMP:
			throw std::runtime_error("There has been an exception the file " + std::string{__FILE__} + " at the line " + std::to_string(__LINE__));
			break;
		default:
			throw std::runtime_error("There has been an exception the file " + std::string{ __FILE__ } + " at the line " + std::to_string(__LINE__));
			break;
		}
	}

	auto begin() const noexcept
	{
		return data.begin() + offset;
	}

	auto end() const noexcept
	{
		return data.begin() + offset + size + 1;
	}

	//TODO: remake these three next functions, they should follow the size_search == 0 condition when constructed, and behave accordingly

	void slide(size_t displacement)
	{
		offset += displacement;
		if ((offset + size) > data.size())
		{
			size -= std::clamp(offset + size - data.size(), 0ull, size);
		}
		offset = std::clamp(offset, 0ull, data.size() + 1);
	}

	auto look_ahead_buffer()
	{
		return data.subspan(offset + std::ceil(size / 2), std::ceil(size / 2));
	}

	auto search_buffer()
	{
		return data.subspan(offset, size_search);
	}

private:
	const std::span<Sym> data;
	size_t size_look_ahead;
	size_t size_search;
	size_t size;
	size_t offset = 0;
};

export class Lz77
{
public:
	//to be fine tuned
	struct Entry
	{
		uint16_t offset;
		uint8_t length;
		Sym symbol;
	};
	    
	Lz77(std::span<Sym> data_, CompPreset preset_)
		:data(data_), preset(preset_), window(data_, preset_)
	{
	}

	void compress();
	void decompress();

private:
	std::span<Sym> data;
	Window window;
	CompPreset preset;

	/**
	* @brief Find a match for a patter in a buffer.
	* @param buffer The searched buffer.
	* @param pattern The pattern.
	* @return An optional index to the start of the match.
	*/
	std::optional<int> search_pattern(const std::span<Sym> buffer, const std::span<Sym> pattern)
	{
		auto buf_size = buffer.size(), patt_size = pattern.size();

		alg::Karp stream_hash{ buffer };
		auto pattern_hash = alg::Karp{ pattern }.get_hash(0, patt_size - 1);

		for (int i = 0; i <= buf_size - patt_size; i++)
		{
			auto sub_hash = stream_hash.get_hash(i, i + patt_size - 1);
			if (sub_hash == pattern_hash)
			{
				if (std::equal(buffer.subspan(i, patt_size).begin(), buffer.subspan(i, patt_size).end(), pattern.begin()))
				{
					return i;
				}
			}
		}
		return std::nullopt;
	}

};


