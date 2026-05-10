module;
#include <mio/mmap.hpp>
#include <cassert>

export module file_util;

export import util;
import parser;
import std.compat;
namespace fs = std::filesystem;



constexpr std::string_view SIGNATURE = "CTF-1";
	                                //SIGNATURE     +   IDENTIFIER    	
export constexpr size_t FILE_HEADER_SIZE = SIGNATURE.size() + sizeof(uint8_t) + sizeof(uint8_t) + sizeof(size_t);
enum class HEADER_OFFSET
{
	SIGNATURE = 0,
	ID = ::SIGNATURE.size(),
	COMP_TYPE = ID + sizeof(size_t),
	COMP_PRESET = COMP_TYPE + sizeof(uint8_t),
	MAX = COMP_PRESET + sizeof(uint8_t),
};

///									TZF		FILE	HEADER
///	 ______________________________________________________________________________
/// | 		     |							   |			    |				   |
/// |  SIGNATURE | Identifier (file splitting) |  CompType(u8)  |  CompPreset(u8)  |
/// |____________|_____________________________|________________|__________________|
///				 ↑							   ↑                ↑				   ↑
///	HEADER_OFFSET::ID   HEADER_OFFSET::COMP_TYPE  HEADER_OFFSET::COMP_PRESET  HEADER_OFFSET::MAX
               

struct Header
{
	size_t identifier;
	CompType comp_type;
	CompPreset preset;

	static constexpr size_t size_bytes() noexcept
	{
		return sizeof(CompType) + sizeof(CompPreset) + sizeof(size_t);
	}
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
		file_options = FileOptions{ path, extract_info(path) };
		out_file = create_file(cli_options.filename_out);
	}

	/**
	* @brief Apply a certain function to a file.
	*/
	template <typename In, typename Out, typename Pred>
	void process_file(Pred pred) requires std::invocable<Pred, CodecInterface<In, Out>&>
	{
		std::ifstream in(cli_options.filename_in, std::ios::binary);

		size_t offset = 0;
		bool compressing = true;
		if constexpr (std::is_same<Out, Sym>::value)
		{
			offset = FILE_HEADER_SIZE;
			compressing = false;
		}


		in.seekg(0, std::ios::end);
		auto file_size = (static_cast<size_t>(in.tellg()) - offset) / sizeof(In) ;
		in.seekg(0, std::ios::beg);
		std::error_code error;
		auto IO_map = mio::make_mmap_source(cli_options.filename_in.string(), offset, mio::map_entire_file, error);

		//if header is empty and we are decompressing, throw
		if (!file_options.header.has_value() and std::is_same<Out, Sym>::value)
		{
			fs::remove(cli_options.filename_out);
			throw_error(ErrorType::INVALID_DECOMPRESSION, file_options.path.string());
		}

		std::vector<Out> output_buffer;//Used to receive data from fun()

		CodecInterface<In, Out> interface{
			.in_data{reinterpret_cast<In const*>(IO_map.data()), reinterpret_cast<In const*>(IO_map.end())},
			.out_data = output_buffer
		};


		while (!interface.in_data.reached_end())
		{
			pred(interface);

			show_progress(cli_options, 1.f - (static_cast<float>(interface.in_data.distance_to_end()) / file_size), compressing);
		}
		out_file.write(reinterpret_cast<char const*>(output_buffer.data()), output_buffer.size() * sizeof(Out));
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
	static void split_file(fs::path const& path, size_t portions)
	{

		if (portions == 1) return;

		if ((portions < 1 or portions > N_FILES_LIMIT) or (fs::file_size(path) / portions) < SIZE_FILES_MIN)
			print_warn(WarningType::PORTIONS_OUT_OF_RANGE);


		std::ifstream source_file{ path, std::ios::binary };
		assert(has_signature(source_file));

		if (!fs::exists(path)) throw_error(ErrorType::PATH_NOT_FOUND, path.string());
		auto Header = extract_info(path);

		source_file.seekg(static_cast<size_t>(HEADER_OFFSET::ID));
		auto portions_size = fs::file_size(path) / portions;
		std::vector<char> buffer(portions_size);
		fs::create_directory(fs::current_path() / "output");

		auto out_path = fs::current_path() / "output";

		//Set up a random number such that num + N_FILES_LIMIT < size_t::max
		std::random_device seed;
		std::default_random_engine rng(seed());
		std::uniform_int_distribution<size_t> dist(1, std::numeric_limits<size_t>::max() - N_FILES_LIMIT);
		auto id = dist(rng);

		for (auto file_n = 0u; file_n < portions; file_n++, id++)
		{
			auto file = create_file(Header.value().comp_type, Header.value().preset, out_path / (path.stem().string() + '_' + std::to_string(file_n) + FILE_EXTENSION), id);

			//We are at the last file, may be smaller than portions_size and/or SIZE_FILES_MIN
			if (portions_size > (fs::file_size(path) - source_file.tellg()))
				buffer.resize(fs::file_size(path) - source_file.tellg());

			source_file.read(buffer.data(), buffer.size());
			file.write(buffer.data(), buffer.size());
		}

		switch (errno)
		{
		case ENOSPC:
		case EIO:
		case ENFILE:
		case EMFILE:
			throw_error(ErrorType::DRIVE_ERROR, path.string());
		default:
			break;
		}
		source_file.close();

		fs::remove(path);
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
	
	/**
	 * @brief Extract information from the header of the tzf file.
	 * @param path
	 * @return A FileOptions type constructed with the obtained information
	 *
	 * @pre path shall exist and be accessible.
	 */
	static std::optional<Header> extract_info(const fs::path& path)
	{

		std::ifstream file{ path, std::ios::binary };

		//we are dealing with an already compressed file
		if (has_signature(file))
		{
			file.seekg(static_cast<size_t>(HEADER_OFFSET::ID));

			std::byte buffer[sizeof(Header)] = { std::byte{0} };
			file.read(reinterpret_cast<char*>(buffer), Header::size_bytes());

			Header header = std::bit_cast<Header>(buffer);

			CompType comp_type = header.comp_type;
			CompPreset comp_preset = header.preset;

			if (comp_type >= CompType::MAX or comp_preset > COMP_MAX)
				throw_error(ErrorType::FILE_CORRUPTED, path.string());

			return std::optional<Header>{header};

		}

		return std::optional<Header>{};
	}

	static std::ofstream create_file(CompType comp_type, CompPreset comp_preset, fs::path path, size_t id)
	{
		std::ofstream file(path, std::ios::binary | std::ios::trunc);

		file.seekp(0);
		file.write(SIGNATURE.data(), SIGNATURE.size());
		file.seekp(static_cast<int>(HEADER_OFFSET::COMP_PRESET));
		uint8_t temp = static_cast<uint8_t>(comp_type);
		file.write(reinterpret_cast<char const*>(&temp), sizeof(decltype(temp)));
		temp = static_cast<uint8_t>(comp_preset);
		file.write(reinterpret_cast<char const*>(&temp), sizeof(decltype(temp)));
		file.write(reinterpret_cast<char const*>(&id), sizeof(size_t));

		return file;
	}


private:
	FileOptions file_options;
	const parser::Options& cli_options;
	std::ofstream out_file;

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
	 * @brief Creates a compressed file(header) with the details of #cli_options. If file exists, it is cleared.
	 * @param path Where the file is created
	 * @return A stream to the file.
	 */
	[[maybe_unused]] std::ofstream create_file(fs::path path) const
	{
		fs::remove(path);
		std::ofstream file{ path, std::ios::binary | std::ios::trunc };

		if (!has_signature() or cli_options.force_compression)
		{
			file.seekp(static_cast<size_t>(HEADER_OFFSET::SIGNATURE));
			file.write(SIGNATURE.data(), SIGNATURE.size());

			file.seekp(static_cast<size_t>(HEADER_OFFSET::ID));
			size_t id_temp = 0;
			file.write(reinterpret_cast<char const*>(&id_temp), sizeof(size_t));
			uint8_t temp = static_cast<uint8_t>(cli_options.compressor);
			file.write(reinterpret_cast<char const*>(&temp), sizeof(decltype(temp)));
			temp = static_cast<uint8_t>(cli_options.preset);
			file.write(reinterpret_cast<char const*>(&temp), sizeof(decltype(temp)));

		}

		return file;
	}

};
