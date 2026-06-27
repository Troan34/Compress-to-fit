module parser;

import util;
import std.compat;

#ifdef __INTELLISENSE__
#include "../../for_intellisense/everything.hpp"
#endif

namespace fs = std::filesystem;

namespace parser
{
Token::Token(TokenType const type_, ValueType const &value_)
	:value(value_), type(type_)
{
}

Token::Token(const std::string& option)
{
	if (auto res = lex(option))
	{
		type = res.value().type;
		value = res.value().value;
	}
}

//Only now I realize I fucked up the names for lex and parse.

/**
 * @brief Semantically check an argument (e.g. -preset 8)
 * @param option The argument
 * @return The token
 */
std::expected<Token, ErrorType> lex(const std::string& option)
{
	auto token_string = option.substr(0, option.find(' '));

	std::string token_value{};
	if (auto value_pos = option.find_first_not_of(' ', option.find(' ')); value_pos != std::string::npos)//handles flags
		token_value = option.substr(value_pos, std::string::npos);


	TokenType token_type;

	//Find TokenType by checking if token_string is equal to any command
	if (token_string == token_strings[static_cast<size_t>(TokenType::COMPRESSION_PRESET)]) token_type = TokenType::COMPRESSION_PRESET;
	else if (token_string == token_strings[static_cast<size_t>(TokenType::FILENAME_IN)]) token_type = TokenType::FILENAME_IN;
	else if (token_string == token_strings[static_cast<size_t>(TokenType::FILENAME_OUT)]) token_type = TokenType::FILENAME_OUT;
	else if (token_string == token_strings[static_cast<size_t>(TokenType::N_FILES)]) token_type = TokenType::N_FILES;
	else if (token_string == token_strings[static_cast<size_t>(TokenType::SIZE_FILES)]) token_type = TokenType::SIZE_FILES;
	else if (token_string == token_strings[static_cast<size_t>(TokenType::LONG_HELP)]) token_type = TokenType::LONG_HELP;
	else if (token_string == token_strings[static_cast<size_t>(TokenType::HELP)]) token_type = TokenType::HELP;
	else if (token_string == token_strings[static_cast<size_t>(TokenType::FORCE_COMPRESSION)]) token_type = TokenType::FORCE_COMPRESSION;
	else if (token_string == token_strings[static_cast<size_t>(TokenType::DELETE_INPUT)]) token_type = TokenType::DELETE_INPUT;
	else if (token_string == token_strings[static_cast<size_t>(TokenType::COMPRESSOR_TYPE)]) token_type = TokenType::COMPRESSOR_TYPE;
	else if (token_string == token_strings[static_cast<size_t>(TokenType::CONCATENATE)]) token_type = TokenType::CONCATENATE;
	else if (token_string == token_strings[static_cast<size_t>(TokenType::DO_DECOMP_AFTER_CONCAT)]) token_type = TokenType::DO_DECOMP_AFTER_CONCAT;
	else if (token_string == token_strings[static_cast<size_t>(TokenType::DO_NOT_DECOMP_AFTER_CONCAT)]) token_type = TokenType::DO_NOT_DECOMP_AFTER_CONCAT;
	else if (token_string == token_strings[static_cast<size_t>(TokenType::CONCURRENCY)]) token_type = TokenType::CONCURRENCY;
	else
		throw_error(ErrorType::SYNTAX_ERROR, token_string);

	//handle flags
	switch (token_type)
	{
	case TokenType::DELETE_INPUT:
	case TokenType::FORCE_COMPRESSION:
	case TokenType::CONCATENATE:
	case TokenType::HELP:
	case TokenType::LONG_HELP:
		return Token{ token_type, true };
		break;
	default:
		break;
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
			auto test_path = path.parent_path() / "test_writeability";
			std::ofstream test_file{test_path};
			if (!test_file.is_open())
			{
				fs::remove(test_path);
				throw_error(ErrorType::PATH_NOT_ACCESSIBLE, path.string());
			}
			test_file.close();
			fs::remove(test_path);

			path.replace_extension(FILE_EXTENSION);

			value.emplace<fs::path>(path);
		}
		break;
		case TokenType::COMPRESSOR_TYPE:
		{
			std::transform(token_value.begin(), token_value.end(), token_value.begin(), [](char c) { return std::toupper(c); });

			for (int i = 0; i < static_cast<int>(CompType::MAX); i++)
			{
				if (COMPRESSOR_STR_OPTIONS[i] == token_value)
				{
					value.emplace<size_t>(static_cast<size_t>(i));
					break;
				}
			}

			if (std::holds_alternative<std::monostate>(value))
				throw_error(ErrorType::SYNTAX_ERROR, token_string + token_value);
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
		break;
		case TokenType::CONCURRENCY:
		{
			size_t value_temp;
			auto const res = std::from_chars(token_value.data(), token_value.data() + token_value.size(), value_temp);

			if (res.ec != std::errc())
				throw_error(ErrorType::VALUE_ERROR, token_string + token_value);

			if (value_temp < 0)
				print_warn(WarningType::CONCURRENCY_OUT_OF_RANGE_LOWER, token_string + token_value);

			if (value_temp > std::thread::hardware_concurrency())
				print_warn(WarningType::CONCURRENCY_OUT_OF_RANGE_UPPER, token_string + token_value);

			//I had no fucking idea size_t had suffix 'uz'
			value_temp = std::clamp(value_temp, 0uz, static_cast<size_t>(std::thread::hardware_concurrency()));

			value.emplace<size_t>(value_temp);
		}
		break;
		//the next are all flags handled by the switch just above
		case TokenType::LONG_HELP:
		case TokenType::HELP:
		case TokenType::FORCE_COMPRESSION:
		case TokenType::DELETE_INPUT:
		case TokenType::CONCATENATE:
		case TokenType::DO_DECOMP_AFTER_CONCAT:
		case TokenType::DO_NOT_DECOMP_AFTER_CONCAT:
		case TokenType::NO_TYPE:
			break;
	}

	return Token{ token_type, value };
}

/**
 * @brief Read the cli.
 * @param argc Number of cli arguments, where an argument is a 'word' separated by spaces, e.g. -i or -preset
 * @param argv Pointer to the arguments
 * @return The options obtained from the cli
 * 
 * @throws std::runtime_error for any kind of syntactic or semantic error
 */
Options parse(int argc, char* argv[])
{
	if (argc == 1)
		throw_error(ErrorType::SYNTAX_ERROR, "No arguments given.");

	Options options;
	//Parse the cli
	//starting at 1 means we ignore the first (name of program)
	for (int index = 1; index < argc; index++)
	{
		std::string option{ argv[index] };
		option += ' ';
		if (index + 1 < argc)//stay in bounds
			option += argv[++index];

		//If the path has spaces, this makes sure that we take the whole path (we check for ")
		//else we throw
		if (std::ranges::count(option, '\"') == 1)
		{
			bool valid = false;

			for(;index < argc - 1; index++)//keep parsing arguments until we find an ending double quote
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
			case TokenType::COMPRESSOR_TYPE:
				options.preset = std::get<size_t>(token.value().get_value());
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
			case TokenType::CONCURRENCY:
				options.concurrency = std::get<size_t>(token.value().get_value());
				break;
			//DO index-- FOR FLAGS, it stops us from skipping arguments
			case TokenType::HELP:
			case TokenType::LONG_HELP:
				options.need_help = std::get<bool>(token.value().get_value());
				if (index + 1 < argc)
					index--;
				break;
			case TokenType::FORCE_COMPRESSION:
				options.force_compression = std::get<bool>(token.value().get_value());
				if (index + 1 < argc)
					index--;
				break;
			case TokenType::DELETE_INPUT:
				options.delete_input = std::get<bool>(token.value().get_value());
				if (index + 1 < argc)
					index--;
				break;
			case TokenType::CONCATENATE:
				options.concatenate_files = std::get<bool>(token.value().get_value());
				if (index + 1 < argc)
					index--;
				break;
			case TokenType::DO_DECOMP_AFTER_CONCAT:
				options.decomp_after_concat = std::get<bool>(token.value().get_value());
				if (index + 1 < argc)
					index--;
				break;
			case TokenType::DO_NOT_DECOMP_AFTER_CONCAT:
				if (index + 1 < argc)
					index--;
				break;
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

	if (options.need_help)
		std::println(help_str);

	if (fs::is_directory(options.filename_in) and !options.concatenate_files)//tried de/compressing a dir
		throw_error(ErrorType::DIR_COMPRESSION, options.filename_in.string());
	else if (options.filename_in.has_filename() and options.concatenate_files)//tried concatenating a file
		throw_error(ErrorType::PATH_INVALID, options.filename_in.string());

	//swap ifs
	if (options.filename_in.empty() or options.filename_in.string().find_first_not_of(' ') == std::string::npos)//if filename_in is empty
	{
		if (options.concatenate_files)
			options.filename_in = fs::current_path();
		else if (!options.need_help)
			throw_error(ErrorType::MISSING_ARGUMENT, token_strings[static_cast<int>(TokenType::FILENAME_IN)].data());
		else
		{
			std::terminate();//if you need help but no arguments
		}
	}

	if (options.filename_out == DEFAULT_OUT_PATH)
	{
		options.filename_out = options.filename_in.parent_path();
		options.filename_out /= DEFAULT_OUT_PATH;
		options.filename_out.replace_extension(FILE_EXTENSION);
	}

	return options;
}

}//namespace parser
