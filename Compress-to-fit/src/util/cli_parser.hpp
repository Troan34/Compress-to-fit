/*
cli_parser.hpp - parse and categorize the cli input
*/
#pragma once
#include <optional>
#include <filesystem>
#include <string>
#include <variant>
#include <expected>
#include <array>

static constexpr size_t N_FILES_LIMIT = 1'000;
static constexpr size_t SIZE_FILES_MIN = 512;

namespace parser
{
	namespace fs = std::filesystem;
	using ValueType = std::variant<std::size_t, fs::path, bool>;


	enum class TokenType
	{
		FILENAME_IN,
		FILENAME_OUT,
		COMPRESSION_PRESET,
		N_FILES,
		SIZE_FILES,
		NO_TYPE = 696969,//we don't want this to be an index
	};
	//HAS to follow the same order of the TokenType enums
	static constexpr std::array<std::string_view, 5> token_strings =
	{
		"-i",
		"-o",
		"-preset",
		"-n_files",
		"-size_files",

	};

	/// <summary>
	/// Parser error types
	/// </summary>
	enum class ErrorType
	{
		NO_ERROR,
		VALUE_ERROR,
		SYNTAX_ERROR,
		OPTION_UNAVAILABLE,
		PATH_NOT_FOUND,
		PATH_NOT_ACCESSIBLE,
	};
	
	namespace ERR_STRING
	{
		static constexpr const char* VALUE = "\033[31merr\033[0m: this is an incorrect value for this option.";
		static constexpr const char* SYNTAX = "\033[31merr\033[0m: the syntax for this option is incorrect.";
		static constexpr const char* OPTION_UNAVAILABLE = "\033[31merr\033[0m: this option is unavailable in this context.";
		static constexpr const char* PATH_NOT_FOUND = "\033[31merr\033[0m: this path could not be found, make sure to double quote (\") around your path if there are spaces";
		static constexpr const char* PATH_NOT_ACCESSIBLE = "\033[31merr\033[0m: this path could not be accessed, the program may not have some required privileges";
	}

	//unscoped because having to add static_cast becomes annoying
	enum CompPreset
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

	void throw_error(ErrorType error, const std::string& error_option);

	/// <summary>
	/// All the options available
	/// </summary>
	struct Options
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
	Options parse(int argc, char* argv[]);

}//namespace parser