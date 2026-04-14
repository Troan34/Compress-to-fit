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


	export enum class TokenType
	{
		FILENAME_IN,
		FILENAME_OUT,
		COMPRESSION_PRESET,
		N_FILES,
		SIZE_FILES,
		EXTRACT,
		NO_TYPE = 696969,//we don't want this to be an index
	};
	//HAS to follow the same order of the TokenType enums
	inline constexpr std::array<std::string_view, 6> token_strings =
	{
		"-i",
		"-o",
		"-preset",
		"-n_files",
		"-size_files",
		"-e",
	};
	
	
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
		bool extract = false;
	};
	
	/// <summary>
	/// Parse the cli, fully manages error checking
	/// </summary>
	/// <returns>An Option type</returns>
	export Options parse(int argc, char* argv[]);

}//namespace parser

/**
 * @brief Show a progress bar in terminal
 * @param options Will check what kind of operation is being done
 * @param progress From 0 to 1
 */
export void show_progress(const parser::Options& options = {}, float progress = 0.f)
{
	const int max_bar_width = 50;

	std::print("\r\033[K");
	std::print("\033[34mProgress on the ");
	if (options.extract)
		std::print("extraction:\033[0m");
	else
		std::print("compression:\033[0m");

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