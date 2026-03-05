#include "cli_parser.hpp"
#include <charconv>
#include <fstream>

namespace parser
{

std::expected<Token, ErrorType> tokenize_option(const std::string& option)
{
	auto token_string = option.substr(0, option.find(' ') + 1);
	auto token_value = option.substr(option.find(' ') + 1, std::string::npos);

	if (token_string.size() == option.size())//we have not found a ' ' character <-> find() returned npos
		return std::unexpected(ErrorType::SYNTAX_ERROR);

	TokenType token_type;

	//Find TokenType
	if (token_string == token_strings[static_cast<std::size_t>(TokenType::COMPRESSION_PRESET)])
	{
		token_type = TokenType::COMPRESSION_PRESET;
	}
	else if (token_string == token_strings[static_cast<std::size_t>(TokenType::FILENAME_IN)])
	{
		token_type = TokenType::FILENAME_IN;
	}
	else if (token_string == token_strings[static_cast<std::size_t>(TokenType::FILENAME_OUT)])
	{
		token_type = TokenType::FILENAME_OUT;
	}
	else
		return std::unexpected(ErrorType::SYNTAX_ERROR);

	ValueType value;
	//Find token_value and do error checking
	switch (token_type)
	{
	case TokenType::FILENAME_IN:
	{
		//Test if we can find and access the input file
		auto& value_ref = std::get<fs::path>(value);

		if (!fs::exists(token_value))
			return std::unexpected(ErrorType::PATH_NOT_FOUND);

		if (std::ifstream file(token_value); !file.is_open())
			return std::unexpected(ErrorType::PATH_NOT_ACCESSIBLE);

		value_ref = token_value;
	}
	break;
	case TokenType::FILENAME_OUT:
	{
		//Test if we can write a file to the given dir
		auto value_ref = std::get<fs::path>(value);

		auto test_path = value_ref.parent_path() / ".test_writability";
		std::ofstream test_file{test_path};
		if (!test_file.is_open())
			return std::unexpected(ErrorType::PATH_NOT_ACCESSIBLE);
	}
	break;
	case TokenType::COMPRESSION_PRESET:
	{
		//Convert and check the compression preset
		auto& value_ref = std::get<size_t>(value);
		auto res = std::from_chars(token_value.data(), token_value.data() + token_value.size(), value_ref);

		if (res.ec != std::errc() or (value_ref > CompPreset::COMP_MAX or value_ref < CompPreset::NO_COMP))//if not a number or outside of our range
			return std::unexpected(ErrorType::VALUE_ERROR);
	}
	break;
	}

}

Token::Token(TokenType type_, ValueType value_)
{
}

}//namespace parser
