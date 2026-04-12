module parser;

import util;
import std;

namespace fs = std::filesystem;

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
	auto token_string = option.substr(0, option.find(' '));
	auto token_value = option.substr(option.find(' ') + 1, std::string::npos);


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
		throw_error(ErrorType::SYNTAX_ERROR, token_string);

	if (token_string.size() == option.size() //we have not found a ' ' character because find() returned npos
		and token_type != TokenType::EXTRACT)
	{
		throw_error(ErrorType::SYNTAX_ERROR, token_string);
	}

	ValueType value;
	//Find token_value and do error checking
	switch (token_type)
	{
	case TokenType::FILENAME_IN:
	{
		//Test if we can find and access the input file
		if (!fs::exists(token_value))
			throw_error(ErrorType::PATH_NOT_FOUND, token_value);

		if (std::ifstream file(token_value); !file.is_open())
			throw_error(ErrorType::PATH_NOT_ACCESSIBLE, token_value);

		value.emplace<fs::path>(token_value);
	}
	break;
	case TokenType::FILENAME_OUT:
	{
		fs::path path{ token_value };
		
		//Test if we can write a file to the given dir
		auto test_path = path.parent_path() / "test_writability";
		std::ofstream test_file{test_path};
		if (!test_file.is_open())
		{
			fs::remove(test_path);
			throw_error(ErrorType::PATH_NOT_ACCESSIBLE, path.string());
		}
		test_file.close();
		fs::remove(test_path);
		
		path = path.stem();
		path += FILE_EXTENSION;

		value.emplace<fs::path>(path);
	}
	break;
	case TokenType::COMPRESSION_PRESET:
	{
		//Convert and check the compression preset
		size_t value_cmpr;
		auto res = std::from_chars(token_value.data(), token_value.data() + token_value.size(), value_cmpr);

		if (res.ec != std::errc() or (value_cmpr > CompPreset::COMP_MAX or value_cmpr < CompPreset::NO_COMP))//if not a number or outside of our range
			throw_error(ErrorType::VALUE_ERROR, token_string + token_value);

		value.emplace<size_t>(value_cmpr);
	}
	break;
	case TokenType::N_FILES:
	{
		int value_temp;
		auto res = std::from_chars(token_value.data(), token_value.data() + token_value.size(), value_temp);

		if (res.ec != std::errc() or (value_temp > N_FILES_LIMIT or value_temp < 1))//if less than 1 or over N_FILES_LIMIT
			throw_error(ErrorType::VALUE_ERROR, token_string + token_value);
		
		value.emplace<size_t>(value_temp);
	}
	break;
	case TokenType::SIZE_FILES:
	{
		int value_temp;
		auto res = std::from_chars(token_value.data(), token_value.data() + token_value.size(), value_temp);

		if (res.ec != std::errc() or (value_temp < SIZE_FILES_MIN))//if negative or under SIZE_FILES_MIN
			throw_error(ErrorType::VALUE_ERROR, token_string + token_value);

		value.emplace<size_t>(value_temp);
	}
	}

	return Token{ token_type, value };
}


Options parse(int argc, char* argv[])
{
	Options options;
	//starting at 1 means we ignore the first (name of program)
	for (int index = 1; index < argc; index++)
	{
		std::string option{ argv[index] };
		option += ' ';
		option += argv[++index];
		
		//If the path has spaces, this makes sure that we take the whole path (we check for ")
		//else we throw
		if (std::count(option.begin(), option.end(), '\"') == 1)
		{
			bool valid = false;

			for(;index < argc - 1; index++)
			{
				std::string option_temp{ argv[index + 1] };
				option += option_temp;
				if (std::count(option_temp.begin(), option_temp.end(), '\"') == 1)
				{
					valid = true;
					break;
				}
			}

			if (!valid)
				throw_error(ErrorType::PATH_INVALID, option);
		}

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
			case TokenType::EXTRACT:
				options.extract = std::get<bool>(token.value().get_value());
			default:
				std::terminate();
				break;
			}
		}
		else
		{
			throw_error(token.error(), option);
		}
	}

	//TODO: When adding decompression capabilities, fix this
	//if filename out is empty, copy filename in
	if (options.filename_out.empty())
	{
		options.filename_out = options.filename_in.stem() += FILE_EXTENSION;
	}
	return options;
}

}//namespace parser
