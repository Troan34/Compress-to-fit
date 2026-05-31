module;
#include <mio/mmap.hpp>
#include <cassert>

export module file_util;

#if defined(__INTELLISENSE__)
#include "../../for_intellisense/everything.hpp"
#endif

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
	COUNT = ID + sizeof(uint16_t),
	COMP_TYPE = COUNT + sizeof(size_t),
	COMP_PRESET = COMP_TYPE + sizeof(uint8_t),
	MAX = COMP_PRESET + sizeof(uint8_t),
};

///									TZF		FILE	HEADER
///	 ________________________________________________________________________________________________
/// | 		     |							   |				  |				  |				     |
/// |  SIGNATURE | Identifier (file splitting) | Count(file splt) |  CompType(u8) |  CompPreset(u8)  |
/// |____________|_____________________________|__________________|_______________|__________________|
///				 ↑							   ↑				  ↑               ↑				     ↑
///	HEADER_OFFSET::ID		HEADER_OFFSET::COUNT   /*...*/::COMP_TYPE  HEADER_OFFSET::COMP_PRESET  HEADER_OFFSET::MAX
		 
//avoid padding in files
#pragma pack(push, 1)
struct Header
{
	size_t identifier;
	std::uint16_t count;
	CompType comp_type;
	CompPreset preset;

};
#pragma pack(pop)

export struct FileOptions
{
	fs::path path;
	std::optional<Header> header;
	bool delete_on_dtor = false;

	[[nodiscard]] bool already_compressed() const noexcept
	{
		return fs::exists(path) and header.has_value();
	}
};

void try_throw_IO_error(std::ios_base::iostate const state, fs::path const& path) noexcept(false)
{
	if (((state & std::ios_base::failbit) != 0) and (state & std::ios_base::eofbit) == 0)
		throw_error(ErrorType::FILE_CORRUPTED, path.string());

	if ((state & std::ios_base::badbit) != 0)//we fucked fucked
		throw_error(ErrorType::DRIVE_ERROR, path.string());
}

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
	File(const fs::path& path, parser::Options const& options)
		:cli_options(options) 
	{
		if (options.concatenate_files)
		{
			concatenate_files(cli_options.filename_in);
		}

		file_options = FileOptions{ path, extract_info(path) };
		out_file = create_file(cli_options.filename_out);
	}

	/**
	 * @brief Sequentially apply a certain function to the #cli_options.filename_in file and write the processed data to #out_file
	 * @tparam In Type of the data found in #cli_options.filename_in
	 * @tparam Out Type of the data written to #out_file
	 * @tparam Pred Function type to be applied, must satisfy `std::invocable<Pred, CodecInterface<In, Out>&>`
	 * @param pred Function to be applied
	 * 
	 * @details This function will ignore the file header
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

		CodecInterface<In, Out> codec_interface{
			.in_data{reinterpret_cast<In const*>(IO_map.data()), reinterpret_cast<In const*>(IO_map.end())},
			.out_data = output_buffer
		};


		while (!codec_interface.in_data.reached_end())
		{
			pred(codec_interface);

			show_progress(cli_options, 1.f - (static_cast<float>(codec_interface.in_data.distance_to_end()) / file_size), compressing);
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

	
	
	/**
	 * @brief Split an encoded file into a certain number of encoded (with header) files 
	 * @param path 
	 * @param portions 
	 */
	static void split_file(fs::path const& path, size_t portions)
	{

		if (portions == 1) return;

		if ((portions < 1 or portions > N_FILES_LIMIT) or (fs::file_size(path) / portions) < SIZE_FILES_MIN)
			print_warn(WarningType::PORTIONS_OUT_OF_RANGE, std::to_string(portions));


		std::ifstream source_file{ path, std::ios::binary };
		assert(has_signature(source_file));

		if (!fs::exists(path)) throw_error(ErrorType::PATH_NOT_FOUND, path.string());
		auto Header = extract_info(path);

		//move to the ID section
		source_file.seekg(static_cast<size_t>(HEADER_OFFSET::ID));
		auto portions_size = fs::file_size(path) / portions;

		std::vector<char> buffer(portions_size);

		auto out_path = path.parent_path() / "output";
		fs::create_directory(out_path);


		//Set up a random number such that num < size_t::max
		std::random_device seed;
		std::default_random_engine rng(seed());
		std::uniform_int_distribution<size_t> dist(1, std::numeric_limits<uint16_t>::max());
		auto id = dist(rng);

		for (uint16_t file_n = 0; file_n < portions; file_n++)
		{
			//create a file with same comp_type and preset, but with id as ID. This file ends with _<file_n>
			auto file = create_file(Header.value().comp_type, Header.value().preset, out_path / (path.stem().string() + '_' + std::to_string(file_n) + FILE_EXTENSION), id, file_n);

			//We are at the last file, may be smaller than portions_size and/or SIZE_FILES_MIN
			if (portions_size > (fs::file_size(path) - source_file.tellg()))
				buffer.resize(fs::file_size(path) - source_file.tellg());

			source_file.read(buffer.data(), buffer.size());
			file.write(buffer.data(), buffer.size());
		}

		try
		{
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
		}
		catch (std::runtime_error const&)
		{
			fs::remove_all(out_path);
			std::terminate();//terminate as to not call fs::remove(path), also, if these errors do happen it is a good idea to drop everything
		}
		source_file.close();

		fs::remove(path);
	}

	/**
	* @brief Read a file into a vector. Will call throw_error if file is not found or inaccessible.
	* @param in_path Location of file.
	* @param received_data The vector that will be filled.
	*/
	static void read_file(const fs::path& in_path, std::vector<Sym>& received_data)
	{
		if (!fs::exists(in_path))
		{
			throw_error(ErrorType::PATH_NOT_FOUND, in_path.string());
		}

		std::ifstream in_file{ in_path, std::ios::ate | std::ios::binary };

		if (!in_file.is_open())
			throw_error(ErrorType::PATH_NOT_ACCESSIBLE, in_path.string());

		auto const size_file = in_file.tellg() / sizeof(Sym);

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
	static bool has_signature(std::fstream& file)
	{
		auto save_pos = file.tellg();
		file.seekg(0);
		std::string buffer(SIGNATURE.size(), ' ');
		file.read(buffer.data(), buffer.size());
		file.seekg(save_pos);

		if (buffer != SIGNATURE.data())//not recognized
		{
			return false;
		}
		return true;
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

		if (buffer != SIGNATURE.data())//not recognized
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

		return extract_info(file, path);
	}

	/**
	 * @brief Extract information from the header of the tzf file.
	 * @param path Will not be used to construct a stream
	 * @param file The file the information will be extracted from
	 * @return A FileOptions type constructed with the obtained information
	 *
	 * @pre path shall exist and be accessible.
	 */
	static std::optional<Header> extract_info(std::ifstream& file, fs::path const& path)
	{

		//we are dealing with an already compressed file
		if (has_signature(file))
		{
			file.seekg(static_cast<size_t>(HEADER_OFFSET::ID));

			std::byte buffer[sizeof(Header)] = { std::byte{0} };
			file.read(reinterpret_cast<char*>(buffer), sizeof(Header));

			Header header = std::bit_cast<Header>(buffer);

			CompType comp_type = header.comp_type;
			CompPreset comp_preset = header.preset;

			if (comp_type >= CompType::MAX or comp_preset > COMP_MAX)
				throw_error(ErrorType::FILE_CORRUPTED, path.string());

			try_throw_IO_error(file.rdstate(), path);

			return std::optional<Header>{header};

		}

		return std::optional<Header>{};
	}

	static std::ofstream create_file(CompType comp_type, CompPreset comp_preset, fs::path path, size_t id, uint16_t count)
	{
		std::ofstream file(path, std::ios::binary | std::ios::trunc);

		file.seekp(0);
		file.write(SIGNATURE.data(), SIGNATURE.size());
		file.write(reinterpret_cast<char const*>(&id), sizeof(decltype(id)));
		file.write(reinterpret_cast<char const*>(&count), sizeof(decltype(count)));
		file.seekp(static_cast<int>(HEADER_OFFSET::COMP_PRESET));
		uint8_t temp = static_cast<uint8_t>(comp_type);
		file.write(reinterpret_cast<char const*>(&temp), sizeof(decltype(temp)));
		temp = static_cast<uint8_t>(comp_preset);
		file.write(reinterpret_cast<char const*>(&temp), sizeof(decltype(temp)));

		return file;
	}


private:
	FileOptions file_options;
	parser::Options const& cli_options;
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
		if (buffer != SIGNATURE.data())//not recognized
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
		if (buffer != SIGNATURE.data())//not recognized
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

		file.seekp(static_cast<size_t>(HEADER_OFFSET::SIGNATURE));
		file.write(SIGNATURE.data(), SIGNATURE.size());

		file.seekp(static_cast<size_t>(HEADER_OFFSET::ID));
		size_t id_temp = 0;
		file.write(reinterpret_cast<char const*>(&id_temp), sizeof(size_t));
		uint8_t temp = static_cast<uint8_t>(cli_options.compressor);
		file.write(reinterpret_cast<char const*>(&temp), sizeof(decltype(temp)));
		temp = static_cast<uint8_t>(cli_options.preset);
		file.write(reinterpret_cast<char const*>(&temp), sizeof(decltype(temp)));


		return file;
	}

	/**
	 * @brief Creates a compressed file(header). If file exists, it is cleared.
	 * @param path Where the file is created
	 * @return A stream to the file.
	 */
	[[maybe_unused]] static std::ofstream create_file(fs::path const& path, Header options)
	{
		fs::remove(path);
		std::ofstream file{ path, std::ios::binary | std::ios::trunc };

		file.seekp(static_cast<size_t>(HEADER_OFFSET::SIGNATURE));
		file.write(SIGNATURE.data(), SIGNATURE.size());
		file.seekp(static_cast<size_t>(HEADER_OFFSET::ID));
		size_t id_temp = 0;
		file.write(reinterpret_cast<char const*>(&id_temp), sizeof(size_t));
		uint8_t temp = static_cast<uint8_t>(options.comp_type);
		file.write(reinterpret_cast<char const*>(&temp), sizeof(decltype(temp)));
		temp = static_cast<uint8_t>(options.preset);
		file.write(reinterpret_cast<char const*>(&temp), sizeof(decltype(temp)));

		return file;
	}

	/**
	 * @brief Concatenate files found in #path into a new file, if 
	 * @param path 
	 * @return path to the new concatenated file
	 */
	fs::path concatenate_files(fs::path const& path)
	{
		std::vector<std::pair<fs::path, uint16_t>> files_to_concat{};//uint16_t is here to keep a temporary copy of count (we sacrifice a bit of memory for fewer drive accesses)
		std::vector<size_t> ignored_ids{};

		size_t id{};
		bool id_not_set_yet = true;

		//add the files to concatenate to #files_to_concat
		for (auto const& dir_entry : fs::directory_iterator{ path })
		{
			if (!dir_entry.is_regular_file())
				continue;

			auto info = extract_info(dir_entry);

			if (!info.has_value())//basically if it's not a tzf file
				continue;

			//do this at first, if it is not the first time it's because the user refused both possible ids. Check the following code.
			if (id_not_set_yet and std::find(ignored_ids.begin(), ignored_ids.end(), info.value().identifier) == ignored_ids.end())
			{
				id = info.value().identifier;
				id_not_set_yet = false;
				files_to_concat.push_back({ dir_entry, info.value().count });
				continue;//skip the rest!!!
			}
			
			//if id is different from identifier and identifier was not ignored
			if (id != info.value().identifier and std::find(ignored_ids.begin(), ignored_ids.end(), info.value().identifier) == ignored_ids.end())
			{
				print_warn(WarningType::CONCAT_AMBIGUITY, files_to_concat.back().first.filename().string() + " " + dir_entry.path().filename().string());
				std::println("Choose which of these files are part of the concatenation. Insert '1' or '2' for the respective file. If neither of these is, insert '0', we will find the next possible file.");
				auto choice = 0;
				//get user to resolve the ambiguity
				while (true)
				{
					std::cin >> choice;

					//1 is minimum_entry and 2 is dir_entry and 3 is neither
					if (choice == 1)
					{
						ignored_ids.push_back(id);
						id_not_set_yet = true;
						std::println("You selected {}", files_to_concat.back().first.filename().string());
						files_to_concat.clear();
					}
					else if (choice == 2)
					{
						ignored_ids.push_back(info.value().identifier);
						id_not_set_yet = true;
						std::println("You selected {}", dir_entry.path().filename().string());
						files_to_concat.clear();
					}
					else if (choice == 0)
					{
						ignored_ids.push_back(id);
						ignored_ids.push_back(info.value().identifier);
						id_not_set_yet = true;
						files_to_concat.clear();
					}
					else 
						std::println("Get it right you stoopid. Insert '1', '2', or '0' if neither file is correct.");
				}
			}
			else//everything goes as expected
			{
				files_to_concat.push_back({ dir_entry, info.value().count });
			}

		}

		auto example = extract_info(files_to_concat[0].first);
		example.value().count = 0;
		std::ofstream file{ create_file(cli_options.filename_out, example.value()) };

		//sort the files base on their count
		std::sort(files_to_concat.begin(), files_to_concat.end(),
				[](std::pair<fs::path, uint16_t> const& file1, std::pair<fs::path, uint16_t> const& file2)
				{
					  return file1.second < file2.second;
				});

		//Copy all the files into the new file
		for (auto const& [path_, _] : files_to_concat)//I couldn't get ranges to work, also, the underscore is a placeholder in c++26. However, msvc is slooooow at getting new stuff. I'm looking at you microsoft, not at the developers.
		{
			std::ifstream input{ path_ };
			input.seekg(0, std::ios::end);
			input.seekg(static_cast<int>(HEADER_OFFSET::MAX));
			
			while (!input.eof())//fill input with file at path_
			{
				//TODO: this exception stuff should be abstracted, also, how about making custom exceptions?

				auto buffer = std::make_unique<std::byte[]>(MiB_to_B(1));
				input.read(reinterpret_cast<char*>(buffer.get()), sizeof(buffer));
				try
				{
					try_throw_IO_error(input.rdstate(), path_);
				}
				catch (...)
				{
					fs::remove(cli_options.filename_out);//cleanup
					throw;
				}

				file.write(reinterpret_cast<char const*>(buffer.get()), input.gcount());
				try
				{
					try_throw_IO_error(file.rdstate(), cli_options.filename_out.string());
				}
				catch (...)
				{
					fs::remove(cli_options.filename_out);//cleanup
					throw;
				}
			}
		}


		return cli_options.filename_out;
	}
};
