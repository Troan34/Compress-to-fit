module file_util;

import util;
import std;

namespace fs = std::filesystem;

namespace file
{
	inline constexpr std::string_view SIGNATURE = "C-T-F";
	inline constexpr size_t FILE_HEADER_SIZE = SIGNATURE.size() + sizeof(size_t);//maybe i'll change this, maybe not

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
	void write_file(std::span<T> buffer, const fs::path& out_path)
	{
		std::ofstream file(out_path, std::ios::app | std::ios::binary);
		
		if (file.tellp() > 0)//not empty
			check_signature(out_path);
		else
			file.write(SIGNATURE, SIGNATURE.size());

		
		file.write(buffer.data(), buffer.size());
	}

	void split_file(const fs::path& path_, size_t portions = 1)
	{
		check_signature(path_);
		
		if ((portions < 1 or portions > N_FILES_LIMIT) or (fs::file_size(path_) / portions) < SIZE_FILES_MIN)
			throw_error(ErrorType::PORTIONS_OUT_OF_RANGE);
		
		std::ifstream source_file{ path_, std::ios::binary};
		source_file.seekg(FILE_HEADER_SIZE);
		auto portions_size = fs::file_size(path_) / portions;
		std::vector<char> buffer(portions_size);

		//Set up a random number such that num + N_FILES_LIMIT < size_t::max
		std::random_device seed;
		std::default_random_engine rng(seed());
		std::uniform_int_distribution<size_t> dist(1, std::numeric_limits<size_t>::max() - N_FILES_LIMIT);
		auto starting_id = dist(rng);

		for (auto file_n = 1u; file_n < portions; file_n++, starting_id++)
		{
			std::ofstream file(path_.string() + '_' + std::to_string(file_n), std::ios::binary | std::ios::trunc);
			file.write(SIGNATURE.data(), SIGNATURE.size());
			file.write(reinterpret_cast<const char*>(&starting_id), sizeof(starting_id));//add the identifying id

			//We are at the last file, may be smaller than portions_size and/or SIZE_FILES_MIN
			if (portions_size > (fs::file_size(path_) - source_file.tellg()))
				buffer.resize(fs::file_size(path_) - source_file.tellg());
			
			source_file.read(buffer.data(), buffer.size());
			file.write(buffer.data(), buffer.size());
		}
	}

	void check_signature(const fs::path& path)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file.is_open())
		{
			throw_error(ErrorType::PATH_NOT_ACCESSIBLE, path.string());
		}

		file.seekg(0);
		std::string buffer(SIGNATURE.size(), ' ');
		file.read(buffer.data(), buffer.size());
		if (!(buffer == SIGNATURE))//not recognized
		{
			throw_error(ErrorType::FILE_INVALID, path.string());
		}
	}

}//namespace file