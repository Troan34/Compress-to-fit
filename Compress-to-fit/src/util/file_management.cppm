export module file_util;

export import util;
import std;
namespace fs = std::filesystem;


inline constexpr std::string_view SIGNATURE = "CTF-1";
	                                        //SIGNATURE       +    IDENTIFIER  + LENGTH of file info header     	
inline constexpr size_t FILE_HEADER_SIZE = SIGNATURE.size() + sizeof(size_t) + sizeof(size_t);


export struct FileOptions
{
	fs::path file_path;
	std::optional<fs::path> original_path;
	CompType comp_type;
	CompPreset preset;
};

/**
 * @brief Interpret a file as to retrieve custom information
 * @details An interpreted file is one where we have the knowledge about it to fill #FileOptions
 */
export class File
{
public:

	/**
	 * @brief Construct member types, such as #FileOptions
	 * @param path path of file
	 */
	File(fs::path path)
		:path_(path)
	{
		if (!fs::exists(path_))
		{
			throw_error(ErrorType::PATH_NOT_FOUND, path_.string());
		}
	}

	/**
	* @brief Apply a certain function to a file.
	* @param in_path File being read.
	* @param out_path Where the result of the function will be stored.
	* @param op The function to be applied.
	*/
	template <ptr_size_pred pred>
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
	template<typename T>
	void write_file(std::span<T> buffer, const fs::path& out_path)
	{
		std::ofstream file(out_path, std::ios::trunc | std::ios::binary);

		if (file.tellp() > 0)//not empty
			check_signature(out_path);
		else
			file.write(SIGNATURE.data(), SIGNATURE.size());


		file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
	}

	/*
	Given a file, split it into multiple files according to documentation.
	*/
	void split_file(const fs::path& path_, size_t portions)
	{
		check_signature(path_);

		if (portions == 1) return;

		if ((portions < 1 or portions > N_FILES_LIMIT) or (fs::file_size(path_) / portions) < SIZE_FILES_MIN)
			throw_error(ErrorType::PORTIONS_OUT_OF_RANGE);


		std::ifstream source_file{ path_, std::ios::binary };
		source_file.seekg(FILE_HEADER_SIZE);
		auto portions_size = fs::file_size(path_) / portions;
		std::vector<char> buffer(portions_size);
		fs::create_directory(fs::current_path() / "output");

		auto out_path = fs::current_path() / "output";

		//Set up a random number such that num + N_FILES_LIMIT < size_t::max
		std::random_device seed;
		std::default_random_engine rng(seed());
		std::uniform_int_distribution<size_t> dist(1, std::numeric_limits<size_t>::max() - N_FILES_LIMIT);
		auto starting_id = dist(rng);

		for (auto file_n = 0u; file_n < portions; file_n++, starting_id++)
		{
			std::ofstream file(out_path / (path_.stem().string() + '_' + std::to_string(file_n) + FILE_EXTENSION), std::ios::binary | std::ios::trunc);
			file.write(SIGNATURE.data(), SIGNATURE.size());
			file.write(reinterpret_cast<const char*>(&starting_id), sizeof(starting_id));//add the identifying id

			//We are at the last file, may be smaller than portions_size and/or SIZE_FILES_MIN
			if (portions_size > (fs::file_size(path_) - source_file.tellg()))
				buffer.resize(fs::file_size(path_) - source_file.tellg());

			source_file.read(buffer.data(), buffer.size());
			file.write(buffer.data(), buffer.size());
		}

		source_file.close();

		fs::remove(path_);
	}

	/**
	* @brief Read a file into a vector. Will call throw_error if file is not found or inaccessible.
	* @param in_path Location of file.
	* @param received_data The vector that will be filled.
	*/
	void read_file(const fs::path& in_path, std::vector<Sym>& received_data)
	{
		if (!fs::exists(in_path))
		{
			throw_error(ErrorType::PATH_NOT_FOUND, in_path.string());
		}

		std::ifstream in_file{ in_path, std::ios::ate | std::ios::binary };

		if (!in_file.is_open())
			throw_error(ErrorType::PATH_NOT_ACCESSIBLE, in_path.string());

		auto size_file = in_file.tellg() / sizeof(Sym);

		received_data.resize(size_file);

		in_file.seekg(0);

		in_file.read(reinterpret_cast<char*>(received_data.data()), size_file * sizeof(Sym));
	}


private:
	FileOptions file_options;
	fs::path path_;


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
		if (!(buffer == SIGNATURE.data()))//not recognized
		{
			throw_error(ErrorType::FILE_INVALID, path.string());
		}
	}

	/**
	 * @brief Non-throwing version of #check_signature
	 * @return True if #path_ has signature, false if not
	 * 
	 * @pre The file shall already exist and be accessible.
	 */
	bool has_signature()
	{

	}
};
