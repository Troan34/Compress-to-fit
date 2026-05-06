export module file_util;

export import util;
import parser;
import std.compat;
namespace fs = std::filesystem;



constexpr std::string_view SIGNATURE = "CTF-1";
	                                //SIGNATURE     +   IDENTIFIER    	
export constexpr size_t FILE_HEADER_SIZE = SIGNATURE.size() + sizeof(size_t) + sizeof(uint8_t) + sizeof(uint8_t);

///									TZF		FILE	HEADER
///	 ___________________________________________________________________________
/// | 		     |							   |								|
/// |  SIGNATURE | Identifier (file splitting) | CompType(u8)	 CompPreset(u8) |
/// |____________|_____________________________|________________________________|
///				 ↑							   ↑                                ↑	
///	SIGNATURE.size()      +       sizeof(size_t)         +       sizeof(uint16_t)	                

struct Header
{
	size_t identifier;
	CompType comp_type;
	CompPreset preset;
};

export struct FileOptions
{
	fs::path path;
	std::optional<Header> header;
	bool delete_on_dtor = false;

	bool already_compressed() const noexcept
	{
		return fs::exists(path) and header.has_value();
	}
};

/**
 * @brief Interpret a file as to retrieve custom information
 * @details An interpreted file is one where we have the knowledge about it to fill #FileOptions
 */
export class File
{
public:

	/**
	 * @brief Construct member types, such as #FileOptions. This ctor should be used when a file is to be compressed
	 * @param path Path of file
	 * @param comp_type Type of compressor
	 * 
	 * @todo Add a parameter for #FileOptions::delete_on_dtor when the parser is updated to do that
	 */
	File(const fs::path& path, const parser::Options& options)
		:cli_options(options) 
	{
		file_options = extract_info(path);
		create_file();
	}

	/**
	* @brief Apply a certain function to a file.
	*/
	template <typename In, typename Out, auto pred, typename Obj>
		requires std::invocable<decltype(pred), Obj&, CodecInterface<In>&>
	void process_file(Obj& obj, CodecInterface<In>& interface)
	{
		std::ifstream in(cli_options.filename_in, std::ios::binary | std::ios::beg);
		std::ofstream out(cli_options.filename_out, std::ios::binary | std::ios::app);

		constexpr size_t SIZE_CHUNK = 4 * 1024 * 1024; //4MB


		std::mutex mut;
		std::condition_variable cv;
		std::vector<Out> shared_buffer;//Used to write to disk
		std::vector<Out> output_buffer;//Used to receive data from fun()
		bool data_ready = false;

		//write to file when data is ready
		std::jthread writer(
			[&](std::stop_token stop)
			{
				while (true)
				{
					std::unique_lock lock(mut);
					//sleep until data is ready or stop is requested
					cv.wait(lock, [&] { return data_ready or stop.stop_requested(); });

					if (data_ready)
					{
						out.write(reinterpret_cast<char const*>(shared_buffer.data()), shared_buffer.size() * sizeof(Out));
						data_ready = false;
						lock.unlock();
						cv.notify_one(); //tell producer we're done writing
					}
					else if (stop.stop_requested())
					{
						break;
					}
				}
			});

		//get file size
		in.seekg(0, std::ios::end);
		auto file_size = in.tellg() / 4;
		in.seekg(0, std::ios::beg);

		bool compressing = !has_signature();
		size_t index = std::min(static_cast<size_t>(file_size), SIZE_CHUNK);

		if (has_signature())
			index += FILE_HEADER_SIZE;

		for (; index <= file_size; index += SIZE_CHUNK)
		{
			if (static_cast<size_t>(file_size) - index < SIZE_CHUNK)//we are too close to the end
			{
				index = file_size;//so just push it to the end, this will almost certainly cause the call to reserve on the shared_buffer
			}

			std::invoke(pred, obj, interface);

			std::unique_lock lock(mut);
			// If writer is still busy, wait for it to finish the last chunk
			cv.wait(lock, [&] { return !data_ready; });

			std::swap(shared_buffer, output_buffer);//move output_buffer into shared_buffer
			data_ready = true;
			lock.unlock();
			cv.notify_one(); // Wake up the writer

			show_progress(cli_options, static_cast<float>(index) / file_size, compressing);
		}
		writer.request_stop();
		cv.notify_one();
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
			check_signature();
		else
			file.write(SIGNATURE.data(), SIGNATURE.size());


		file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
	}

	/*
	Given a file, split it into multiple files according to documentation.
	*/
	void split_file(size_t portions)
	{
		check_signature();

		if (portions == 1) return;

		if ((portions < 1 or portions > N_FILES_LIMIT) or (fs::file_size(file_options.path) / portions) < SIZE_FILES_MIN)
			print_warn(WarningType::PORTIONS_OUT_OF_RANGE);


		std::ifstream source_file{ file_options.path, std::ios::binary };
		source_file.seekg(FILE_HEADER_SIZE);
		auto portions_size = fs::file_size(file_options.path) / portions;
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
			std::ofstream file(out_path / (file_options.path.stem().string() + '_' + std::to_string(file_n) + FILE_EXTENSION), std::ios::binary | std::ios::trunc);
			file.write(SIGNATURE.data(), SIGNATURE.size());
			file.write(reinterpret_cast<const char*>(&starting_id), sizeof(starting_id));//add the identifying id

			//We are at the last file, may be smaller than portions_size and/or SIZE_FILES_MIN
			if (portions_size > (fs::file_size(file_options.path) - source_file.tellg()))
				buffer.resize(fs::file_size(file_options.path) - source_file.tellg());

			source_file.read(buffer.data(), buffer.size());
			file.write(buffer.data(), buffer.size());
		}

		source_file.close();

		fs::remove(file_options.path);
		file_options.path = "";
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

	/**
	 * @brief Check if file has the #SIGNATURE in the header. Non-throwing version of #check_signature.
	 * @return True if file has signature, false if not.
	 *
	 * @param file File to be checked.
	 * @pre The file shall already exist and be accessible.
	 * @post file will be unchanged.
	 */
	static bool has_signature(std::ifstream& file)
	{
		auto save_pos = file.tellg();
		file.seekg(0);
		std::string buffer(SIGNATURE.size(), ' ');
		file.read(buffer.data(), buffer.size());
		file.seekg(save_pos);

		if (!(buffer == SIGNATURE.data()))//not recognized
		{
			return false;
		}
		return true;
	}

private:
	FileOptions file_options;
	const parser::Options& cli_options;

	/**
	* @brief Check if #file_options.path has the #SIGNATURE in the header, throws if not.
	*/
	void check_signature()
	{
		std::ifstream file{ file_options.path, std::ios::binary };

		file.seekg(0);
		std::string buffer(SIGNATURE.size(), ' ');
		file.read(buffer.data(), buffer.size());
		if (!(buffer == SIGNATURE.data()))//not recognized
		{
			throw_error(ErrorType::FILE_INVALID, file_options.path.string());
		}
	}

	/**
	 * @brief Check if #file_options.path has the #SIGNATURE in the header. Non-throwing version of #check_signature.
	 * @return True if #file_options.path has signature, false if not.
	 * 
	 * @pre The file shall already exist and be accessible.
	 */
	bool has_signature() const
	{
		std::ifstream file{ file_options.path, std::ios::binary };

		file.seekg(0);
		std::string buffer(SIGNATURE.size(), ' ');
		file.read(buffer.data(), buffer.size());
		if (!(buffer == SIGNATURE.data()))//not recognized
		{
			return false;
		}
		return true;
	}


	/**
	 * @brief Extract information from the header of the tzf file.
	 * @param path 
	 * @return A FileOptions type constructed with the obtained information
	 * 
	 * @pre path shall exist and be accessible.
	 */
	FileOptions extract_info(const fs::path& path)
	{
		if (!fs::exists(path))
		{
			std::ofstream file{ path };
			return FileOptions{ path };
		}

		std::ifstream file{ path, std::ios::binary };

		//we are dealing with an already compressed file
		if (has_signature(file))
		{
			file.seekg(SIGNATURE.size());
			
			std::byte buffer[sizeof(Header)];
			file.read(reinterpret_cast<char*>(buffer), sizeof(Header));

			Header header = std::bit_cast<Header>(buffer);

			CompType comp_type = header.comp_type;

			if (comp_type >= CompType::MAX)
				//throw_error(ErrorType::FILE_CORRUPTED, path.string());

			return FileOptions{ path, header };
			
		}

		return FileOptions{ path };
	}


	void create_file()
	{
		std::ofstream file(cli_options.filename_out, std::ios::trunc | std::ios::binary);

		if (file.tellp() > 0)//not empty
			check_signature();
		else
			file.write(SIGNATURE.data(), SIGNATURE.size());
	}
};
