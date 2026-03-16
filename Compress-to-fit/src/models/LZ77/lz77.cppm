export module lz77;

export import util;
import std.compat;

namespace model
{

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

	private:
		const std::span<Sym> data;
		size_t size;
		size_t offset = 0;
	};

	export class Lz77
	{
	public:
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

}//namespace model