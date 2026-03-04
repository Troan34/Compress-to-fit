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
		VALUE_MISSING,
		VALUE_INCORRECT,
		SYNTAX_ERROR,
		OPTION_UNAVAILABLE,
	};


	


	class Token
	{
	using ValueType = std::variant<std::size_t, fs::path, bool>;

	public:
		TokenType type;
		
		Token(TokenType type_, ValueType value_);

	private:
		 ValueType value;
	};


	std::expected<Token, ErrorType> tokenize_option(const std::string& option);




}//namespace parser