/*
cli_parser.cppm - parse and categorize the cli input
*/
export module parser;

import util;
import std;


export inline constexpr size_t N_FILES_LIMIT = 1'000;
export inline constexpr size_t SIZE_FILES_MIN = 512;
inline constexpr const char* FILE_EXTENSION = ".tzf";

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
		NO_TYPE = 696969,//we don't want this to be an index
	};
	//HAS to follow the same order of the TokenType enums
	inline constexpr std::array<std::string_view, 5> token_strings =
	{
		"-i",
		"-o",
		"-preset",
		"-n_files",
		"-size_files",
	};

	

	//unscoped because having to add static_cast becomes annoying
	export enum CompPreset
	{
		NO_COMP,
		COMP_1,
		COMP_2,
		COMP_3,
		COMP_4,
		COMP_5,
		COMP_6,
		COMP_7,
		COMP_8,
		COMP_MAX,
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
	};
	
	/// <summary>
	/// Parse the cli, fully manages error checking
	/// </summary>
	/// <returns>An Option type</returns>
	export Options parse(int argc, char* argv[]);

}//namespace parser