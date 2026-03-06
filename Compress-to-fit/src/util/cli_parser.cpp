#include "cli_parser.hpp"
#include <charconv>
#include <fstream>
#include <cassert>

namespace parser
{

Token::Token(TokenType type_, ValueType value_)
	:type(type_), value(value_)
{
}

Token::Token(const std::string& option)
{
	auto res = lex(option);
	if (res)
	{
		type = res.value().type;
		value = res.value().value;
	}
}

std::expected<Token, ErrorType> lex(const std::string& option)
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
	else if (token_string == token_strings[static_cast<std::size_t>(TokenType::N_FILES)])
	{
		token_type = TokenType::N_FILES;
	}
	else if (token_string == token_strings[static_cast<std::size_t>(TokenType::SIZE_FILES)])
	{
		token_type = TokenType::SIZE_FILES;
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
	case TokenType::N_FILES:
	{
		int value_temp;
		auto res = std::from_chars(token_value.data(), token_value.data() + token_value.size(), value_temp);

		if (res.ec != std::errc() or (value_temp > N_FILES_LIMIT or value_temp < 0))//if negative or over N_FILES_LIMIT
			return std::unexpected(ErrorType::VALUE_ERROR);
		
		std::get<size_t>(value) = value_temp;
	}
	break;
	case TokenType::SIZE_FILES:
	{
		int value_temp;
		auto res = std::from_chars(token_value.data(), token_value.data() + token_value.size(), value_temp);

		if (res.ec != std::errc() or (value_temp < SIZE_FILES_MIN))//if negative or under SIZE_FILES_MIN
			return std::unexpected(ErrorType::VALUE_ERROR);

		std::get<size_t>(value) = value_temp;
	}
	}

	return Token{ token_type, token_value };
}

void throw_error(ErrorType error, const std::string& error_option)
{
	switch (error)
	{
	case parser::ErrorType::NO_ERROR:
		break;
	case parser::ErrorType::VALUE_ERROR:
		throw std::runtime_error(error_option + "< " + ERR_STRING::VALUE);
		break;
	case parser::ErrorType::SYNTAX_ERROR:
		throw std::runtime_error(error_option + "< " + ERR_STRING::SYNTAX);
		break;
	case parser::ErrorType::OPTION_UNAVAILABLE:
		throw std::runtime_error(error_option + "< " + ERR_STRING::OPTION_UNAVAILABLE);
		break;
	case parser::ErrorType::PATH_NOT_FOUND:
		throw std::runtime_error(error_option + "< " + ERR_STRING::PATH_NOT_FOUND);
		break;
	case parser::ErrorType::PATH_NOT_ACCESSIBLE:
		throw std::runtime_error(error_option + "< " + ERR_STRING::PATH_NOT_ACCESSIBLE);
		break;
	default:
		break;
	}
}

Options parse(int argc, char* argv[])
{
	Options options;
	//starting at 1 means we ignore the first (name of program)
	for (int index = 1; index < argc; index += 2)
	{
		std::string option{ argv[index] };
		option += argv[index + 1];
		
		auto token = lex(option);

		if (token)
		{
			switch (token.value().get_type())
			{
			case TokenType::FILENAME_IN:
				options.filename_in = std::get<fs::path>(token.value().get_value());
				break;
			case TokenType::FILENAME_OUT:
				options.filename_out = std::get<fs::path>(token.value().get_value());
				break;
			case TokenType::COMPRESSION_PRESET:
				options.preset = std::get<size_t>(token.value().get_value());
				break;
			case TokenType::N_FILES:
				options.n_files = std::get<size_t>(token.value().get_value());
				break;
			case TokenType::SIZE_FILES:
				options.size_files = std::get<size_t>(token.value().get_value());
				break;
			default:
				assert(false == true);
				break;
			}
		}
	}
}

}//namespace parser
