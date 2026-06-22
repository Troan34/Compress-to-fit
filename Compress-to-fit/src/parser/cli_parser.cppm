export module parser;

#ifdef __INTELLISENSE__
#include "../../for_intellisense/everything.hpp"
#endif

import util;
import std.compat;
namespace fs = std::filesystem;


namespace parser
{
	namespace fs = std::filesystem;
	using ValueType = std::variant<std::monostate, std::size_t, fs::path, bool>;

	/**
	 * @brief Parser token types.
	 * 
	 * @note Keep the ordering as this, if you must modify it, modify #token_strings too.
	 */
	export enum class TokenType
	{
		FILENAME_IN,
		FILENAME_OUT,
		COMPRESSOR_TYPE,
		COMPRESSION_PRESET,
		N_FILES,
		SIZE_FILES,
		LONG_HELP,
		HELP,
		FORCE_COMPRESSION,
		DELETE_INPUT,
		CONCATENATE,
		DO_DECOMP_AFTER_CONCAT,
		DO_NOT_DECOMP_AFTER_CONCAT,
		NO_TYPE = 696969,//we don't want this to be an index
	};
	//HAS to follow the same order of the TokenType enums
	constexpr std::string_view token_strings[] =
	{
		"-i",
		"-o",
		"-c",
		"-preset",
		"-n_files",
		"-size_files",
		"-help",
		"-h",
		"-fc",
		"-di",
		"-concat",
		"-concat:y",
		"-concat:n",
	};
	

	constexpr std::string_view help_str =
		"\033[1m\033[36mWelcome to Compress To Fit!\033[0m\n"
		"A cli compressor program.\n"
		"Usage: ctf [options]\n"
		"options:\n"
		"  \033[2m-i <path>\033[0m  Take <path> as input path. If <path> contains spaces, make sure to double quote(\"\") around <path>.\n"
		"  \033[2m-o <path>\033[0m  Take <path> as output path. If <path> contains spaces, make sure to double quote(\"\") around <path>.\n"
		"  \033[2m-c <comp_options>\033[0m  Use a specific compressor from a list of compressor options (look for [comp_options] in this page).\n"
		"  \033[2m-fc\033[0m  Force compression of compressed files.\n"
		"  \033[2m-di\033[0m  Delete input file on compression/extraction\n"
		"  \033[2m-preset <n>\033[0m  Compression preset n is from 0 to 9 (included).\n"
		"  \033[2m-n_files <n>\033[0m  In how many files should the output be split in (max 1000).\n"
		"  \033[2m-concat\033[0m  Concatenate the compressed files in the folder specified by '-i'. Will prompt whether you want to decompress the concatenated file.\n"
		"  \033[2m-concat:y\033[0m  Concatenate the compressed files in the folder specified by '-i'. WILL decompress the concatenated file.\n"
		"  \033[2m-concat:n\033[0m  Concatenate the compressed files in the folder specified by '-i'. WILL NOT decompress the concatenated file.\n"
		"  \033[2m-size_files <bytes>\033[0m  Split the output file in files of <bytes> size (min 512).\n"
		"\n\n"
		"[comp_options]:\n"
		"\033[35m\"LZ77\"\033[0m: A dictionary based algorithm, good for repetitive patterns of data. Slow compression and fast decompression.\n"
		"\nAdditional details:\n"
		" - If an option is encountered multiple times, the last occurence shall be considered by the program.";
	
	export class HelpException : public std::exception
	{
	public:
		[[nodiscard]] const char* what() const noexcept override
		{
			return help_str.data();
		}
	};

	export const fs::path DEFAULT_OUT_PATH{ "output.tzf" };



	
	/// <summary>
	/// Represents a cli option
	/// </summary>
	class Token
	{
	public:
	
		Token(TokenType type_, ValueType const &value_);
		Token(const std::string& option);

		[[nodiscard]] auto get_value() const noexcept
		{
			return value;
		}

		[[nodiscard]] auto get_type() const noexcept
		{
			return type;
		}

	private:
		ValueType value;
		TokenType type = TokenType::NO_TYPE;
	};

	//Constructs a Token given a string that looks like this "-<option> <value>"
	std::expected<Token, ErrorType> lex(const std::string& option);


	/// <summary>
	/// All the options available
	/// </summary>
	export struct Options
	{
		fs::path filename_in{};
		fs::path filename_out{ DEFAULT_OUT_PATH };
		size_t compressor = static_cast<size_t>(CompType::LZ77);
		size_t preset = COMP_5;
		size_t n_files = 1;
		size_t size_files = 0;
		bool force_compression = false;
		bool delete_input = false;
		bool concatenate_files = false;
		bool need_help = false;
		std::optional<bool> decomp_after_concat{};
		size_t concurrency = std::thread::hardware_concurrency();
	};
	
	/**
	 * @brief Parse the cli, fully manages error checking.
	 * @return The options received from the cli.
	 * 
	 * @post Paths will be validated ONLY FOR THEIR EXISTANCE AND ACCESSIBILITY.
	 */
	export Options parse(int argc, char* argv[]);

}//namespace parser

/**
 * @brief Show a progress bar in terminal.
 * @param options Receive miscellaneus data.
 * @param progress From 0 to 1
 */
export void show_progress(const parser::Options& options = {}, float progress = 0.f, bool compressing = false)
{
	progress = std::min(1.F, progress);
	constexpr int max_bar_width = 50;

	std::print("\r\033[K");
	std::print("\033[34mProgress on the ");
	if (compressing)
		std::print("compression:\033[0m");
	else
		std::print("extraction:\033[0m");


	std::print("[\033[32m");
	int const filled_width = std::ceil(max_bar_width * progress);
	for (int i = 0; i < filled_width; i++)
	{
		std::print("█");
	}
	for (int i = filled_width; i < max_bar_width; i++)
	{
		std::print("-");
	}


	std::print("\033[0m] {:.1f}%", progress * 100.F);
	std::cout << std::flush;
}