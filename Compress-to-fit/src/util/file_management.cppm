export module file_util;

export import util;
import std;
namespace fs = std::filesystem;

namespace file
{
	inline constexpr std::string_view SIGNATURE = "CTF-1";
	inline constexpr size_t FILE_HEADER_SIZE = SIGNATURE.size() + sizeof(size_t);//maybe i'll change this, maybe not

	//Check the signature of the file at 'path', throws if check fails (and if it doesn't exist)
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

	/*
	Apply a certain function of type 'fun' to the bytes read in from file 'in_path' and written to 'out_path'.
	The type 'fun' shall take a pointer to the data and an integer for the size of the data, the return of 'fun' is discarded.
	This will not check if a path is accessible or existant, or any other form of checking.
	*/
	export template <ptr_size_pred pred>
	void process_file(const fs::path& in_path, const fs::path& out_path, pred op)
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
	/*
	Given a path, the file will be created there.
	If the file already exists and the file signature is correct (if not: throw), append new data to the end of the file.
	*/
	export template<typename T>
	void write_file(std::span<T> buffer, const fs::path& out_path)
	{
		std::ofstream file(out_path, std::ios::app | std::ios::binary);

		if (file.tellp() > 0)//not empty
			check_signature(out_path);
		else
			file.write(SIGNATURE.data(), SIGNATURE.size());


		file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
	}

	/*
	Given a file, split it into multiple files according to documentation.
	*/
	export void split_file(const fs::path& path_, size_t portions)
	{
		check_signature(path_);

		if ((portions < 1 or portions > N_FILES_LIMIT) or (fs::file_size(path_) / portions) < SIZE_FILES_MIN)
			throw_error(ErrorType::PORTIONS_OUT_OF_RANGE);

		std::ifstream source_file{ path_, std::ios::binary };
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



}//namespace file
