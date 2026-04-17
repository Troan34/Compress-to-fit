module;
#include <print>
export module parser;

import util;
import std;
namespace fs = std::filesystem;




namespace parser
{
	namespace fs = std::filesystem;
	using ValueType = std::variant<std::size_t, fs::path, bool>;

	/**
	 * @brief Parser token types.
	 * 
	 * @note Keep the ordering as this, if you must modify it, modify #token_strings too.
	 */
	export enum class TokenType
	{
		FILENAME_IN,
		FILENAME_OUT,
		COMPRESSION_PRESET,
		N_FILES,
		SIZE_FILES,
		LONG_HELP,
		HELP,
		FORCE_COMPRESSION,
		DELETE_INPUT,
		NO_TYPE = 696969,//we don't want this to be an index
	};
	//HAS to follow the same order of the TokenType enums
	constexpr std::string_view token_strings[] =
	{
		"-i",
		"-o",
		"-preset",
		"-n_files",
		"-size_files",
		"-help",
		"-h",
		"-fc",
		"-di",
	};
	
	constexpr std::string_view help_str =
		"\033[1m\033[36mWelcome to Compress To Fit!\033[0m\n"
		"A cli compressor program.\n"
		"Usage: ctf [options]\n"
		"options:\n"
		"\t-i <path>  Take <path> as input. If <path> contains spaces, make sure to double quote(\"\") around <path>.\n"
		"\t-o <path>  Take <path> as output. If <path> contains spaces, make sure to double quote(\"\") around <path>.\n"
		"\t-c <comp_options>  Use a specific compressor from a list of compressor options (look for [comp_options] in this page).\n"
		"\t-fc  Force compression of compressed files.\n"
		"\t-di  Delete input file on compression/extraction\n"
		"\t-preset <n>  Compression preset n is from 0 to 9 (included).\n"
		"\t-n_files <n>  In how many files should the output be split in (max 1000).\n"
		"\t-size_files <bytes>  Split the output file in files of <bytes> size (min 512).\n"
		"\n\n"
		"[comp_options]:"
		"\"LZ77\": A dictionary based algorithm, good for repetitive patterns of data, slow compression and fast decompression.";
	



	
	/// <summary>
	/// Represents a cli option
	/// </summary>
	class Token
	{
	public:
	
		Token(TokenType type_, ValueType value_);
		Token(const std::string& option);

		inline ValueType get_value() const noexcept
		{
			return value;
		}

		inline TokenType get_type() const noexcept
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
		fs::path filename_in;
		fs::path filename_out;
		size_t preset = COMP_5;
		size_t n_files = 1;
		size_t size_files;
		bool force_compression = false;
		bool delete_input = false;
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
	const int max_bar_width = 50;

	std::print("\r\033[K");
	std::print("\033[34mProgress on the ");
	if (compressing)
		std::print("compression:\033[0m");
	else
		std::print("extraction:\033[0m");


	std::print("[\033[32m");
	int filled_width = std::ceil(max_bar_width * progress);
	for (int i = 0; i < filled_width; i++)
	{
		std::print("█");
	}
	for (int i = filled_width; i < max_bar_width; i++)
	{
		std::print("-");
	}


	std::print("\033[0m] {:.1f}%", progress * 100.0f);
	std::cout << std::flush;
}