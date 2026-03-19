export module lz77;

export import util;
import std.compat;


class Window
{
public:

	Window(const std::span<Sym>& data_, size_t window_size) noexcept
		:data(data_), size(window_size)
	{
	}

	auto begin() const noexcept
	{
		return data.begin() + offset;
	}

	auto end() const noexcept
	{
		return data.begin() + offset + size + 1;
	}

	void slide(size_t displacement)
	{
		offset += displacement;
		if ((offset + size) > data.size())
		{
			size -= std::clamp(offset + size - data.size(), 0ull, size);
		}
		offset = std::clamp(offset, 0ull, data.size() + 1);
	}

	auto look_ahead_buffer() const //const but the data of the span 'data' is not const protected (correct behaviour)
	{
		return std::span<Sym>{data.begin() + offset + (size / 2), size / 2};
	}

	auto search_buffer() const //const but the data of the span 'data' is not const protected (correct behaviour)
	{
		return std::span<Sym>{data.begin() + offset, size / 2};
	}

private:
	const std::span<Sym> data;
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
		:data(data_), preset(preset_)
	{
	}

	void compress();
	void decompress();

private:
	std::span<Sym> data;
	CompPreset preset;


};

namespace algs
{
	inline constexpr unsigned int MOD = 1e9 + 7;
	inline constexpr unsigned int BASE = 31;

	class Karp
	{
	private:
		
		struct Hash
		{
			Hash(int num) : hash_val(num)
			{ }

			int hash_val;

			void operator+=(int operand) noexcept
			{
				hash_val += operand;
				if (hash_val >= MOD) hash_val -= MOD;
			}

			void operator-=(int operand) noexcept
			{
				hash_val -= operand;
				if (hash_val < 0) hash_val + MOD;
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
		};

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

		auto get_hash(int pos, int size)
		{
			auto hash = hashes[pos + size];
			if (pos > 0)
			{
				hash = hash - (hashes[pos - 1] * powers[size + 1].hash_val);
			}

			return hash;
		}
	};


}//namespace algs
