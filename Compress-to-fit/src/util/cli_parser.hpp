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


namespace parser
{
	namespace fs = std::filesystem;
	using ValueType = std::variant<std::size_t, fs::path, bool>;
	

	enum class TokenType
	{
		FILENAME_IN,
		FILENAME_OUT,
		COMPRESSION_PRESET,
	};
	//HAS to follow the same order of the TokenType enums
	static constexpr std::array<std::string_view, 3> token_strings =
	{
		"-i",
		"-o",
		"-preset"
	};

	/// <summary>
	/// Parser error types
	/// </summary>
	enum class ErrorType
	{
		VALUE_ERROR,
		SYNTAX_ERROR,
		OPTION_UNAVAILABLE,
		PATH_NOT_FOUND,
		PATH_NOT_ACCESSIBLE,
	};

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
		TokenType type;
		
		Token(TokenType type_, ValueType value_);

	private:
		 ValueType value;
	};

	/// <summary>
	/// Constructs a Token given a string that looks like this "-<option> <value>"
	/// </summary>
	/// <returns>an std::expected with E as parser::ErrorType</returns>
	std::expected<Token, ErrorType> tokenize_option(const std::string& option);


}//namespace parser