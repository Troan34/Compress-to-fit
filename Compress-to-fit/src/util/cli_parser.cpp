#include "cli_parser.hpp"

namespace parser
{
	std::expected<Token, ErrorType> tokenize_option(const std::string& option)
	{
		auto token_string = option.substr(0, option.find(' ') + 1);
		auto token_value = option.substr(option.find(' ') + 1, std::string::npos);

		if (token_string.size() == option.size())//we have not found a ' ' character, find() returned npos
			return std::unexpected(ErrorType::SYNTAX_ERROR);

		//Find TokenType
		if(token_string)


	}

	Token::Token(TokenType type_, ValueType value_)
	{
	}

}//namespace parser
