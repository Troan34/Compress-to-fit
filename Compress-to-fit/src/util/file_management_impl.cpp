module file_util;

import util;
import std;

namespace fs = std::filesystem;

namespace file
{

	template <ptr_size_fun fun>
	void process_file(const fs::path& in_path, const fs::path& out_path, fun op)
	{
		
		std::ifstream in(in_path, std::ios::binary);
		std::ofstream out(out_path, std::ios::binary);

		static constexpr size_t SIZE_CHUNK = 8 * 1024 * 1024; //8MB

		std::vector<char> buffer(SIZE_CHUNK);

		while (true)
		{
			in.read(buffer.data(), SIZE_CHUNK);
			auto bytes_read = in.gcount();//bytes_read may be lower than SIZE_CHUNK

			if (bytes_read == 0)
				break;

			op(buffer.data(), bytes_read);

			out.write(buffer.data(), bytes_read);
		}
	}

	template<typename T>
	void write_file(std::span<T> data, const fs::path& out_path)
	{
		std::ofstream file(out_path, std::ios::app);


	}


	void split_file(const fs::path& path_, size_t portions = 1)
	{
		
	}

}//namespace file