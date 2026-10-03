/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*
*	Parses the cli
*/
module;
#include <algorithm>

#include "../common/error_warn_print.hpp"
#include "../common/sink.hpp"
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
		report({.message_ID_or_progress = ErrorType::SYNTAX_ERROR, .failing_option = token_string, .compressing = {}});

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
				report({.message_ID_or_progress = ErrorType::PATH_NOT_FOUND, .failing_option = token_value, .compressing = {}});

			if (std::ifstream file(token_value); !file.is_open())
				report({.message_ID_or_progress = ErrorType::PATH_NOT_ACCESSIBLE, .failing_option = token_value, .compressing = {}});

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
				report({ErrorType::PATH_NOT_ACCESSIBLE, path.string(), {}});
			}
			test_file.close();
			fs::remove(test_path);

			path.replace_extension(FILE_EXTENSION);

			value.emplace<fs::path>(path);
		}
		break;
		case TokenType::COMPRESSOR_TYPE:
		{
			std::ranges::transform(token_value, token_value.begin(), [](char c) { return std::toupper(c); });

			for (int i = 0; i < static_cast<int>(CompType::MAX); i++)
			{
				if (COMPRESSOR_STR_OPTIONS[i] == token_value)
				{
					value.emplace<size_t>(static_cast<size_t>(i));
					break;
				}
			}

			if (std::holds_alternative<std::monostate>(value))
				report({.message_ID_or_progress = ErrorType::SYNTAX_ERROR, .failing_option = token_string + token_value, .compressing = {}});
		}
		break;
		case TokenType::COMPRESSION_PRESET:
		{
			//Convert and check the compression preset
			size_t value_cmpr;
			auto [_, ec] = std::from_chars(token_value.data(), token_value.data() + token_value.size(), value_cmpr);

			if (ec != std::errc() or (value_cmpr > CompPreset::COMP_MAX or value_cmpr < CompPreset::NO_COMP))//if not a number or outside of our range
				report({.message_ID_or_progress = ErrorType::VALUE_ERROR, .failing_option = token_string + token_value, .compressing = {}});

			value.emplace<size_t>(value_cmpr);
		}
		break;
		case TokenType::N_FILES:
		{
			int value_temp;
			auto [_, ec] = std::from_chars(token_value.data(), token_value.data() + token_value.size(), value_temp);

			if (ec != std::errc() or (value_temp > N_FILES_LIMIT or value_temp < 1))//if less than 1 or over N_FILES_LIMIT
				report({.message_ID_or_progress = ErrorType::VALUE_ERROR, .failing_option = token_string + token_value, .compressing = {}});

			value.emplace<size_t>(value_temp);
		}
		break;
		case TokenType::SIZE_FILES:
		{
			int value_temp;
			auto [_, ec] = std::from_chars(token_value.data(), token_value.data() + token_value.size(), value_temp);

			if (ec != std::errc() or (value_temp < SIZE_FILES_MIN))//if negative or under SIZE_FILES_MIN
				report({.message_ID_or_progress = ErrorType::VALUE_ERROR, .failing_option = token_string + token_value, .compressing = {}});

			value.emplace<size_t>(value_temp);
		}
		break;
		case TokenType::CONCURRENCY:
		{
			size_t value_temp;
			auto [_, ec] = std::from_chars(token_value.data(), token_value.data() + token_value.size(), value_temp);

			if (ec != std::errc())
				report({.message_ID_or_progress = ErrorType::VALUE_ERROR, .failing_option = token_string + token_value, .compressing = {}});

			if (value_temp < 0)
				report({.message_ID_or_progress=WarningType::CONCURRENCY_OUT_OF_RANGE_LOWER, .failing_option = token_string + token_value, .compressing = {}});

			if (value_temp > std::thread::hardware_concurrency())
				report({.message_ID_or_progress=WarningType::CONCURRENCY_OUT_OF_RANGE_UPPER, .failing_option = token_string + token_value, .compressing = {}});

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
 * @throws ErrorException for any kind of syntactic or semantic error
 * @throws HelpException if help command was used
 * @todo simplify whatever spaghetti is inside of this
 */
auto parse(int argc, char* argv[]) -> Options
{
	if (argc == 1)
		report({.message_ID_or_progress = ErrorType::SYNTAX_ERROR, .failing_option = "No arguments given.", .compressing = {}});

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
				if (std::ranges::count(option_temp, '\"') == 1)
				{
					valid = true;
					break;
				}
			}

			if (!valid)
				report({.message_ID_or_progress = ErrorType::PATH_INVALID, .failing_option = option, .compressing = {}});
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
				options.compressor = std::get<size_t>(token.value().get_value());
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
			report({.message_ID_or_progress = token.error(), .failing_option = option, .compressing = {}});
		}
	}


	if (fs::is_directory(options.filename_in) and !options.concatenate_files)//tried de/compressing a dir
		report({.message_ID_or_progress = ErrorType::DIR_COMPRESSION, .failing_option = options.filename_in.string(), .compressing = {}});
	else if (options.filename_in.has_filename() and options.concatenate_files)//tried concatenating a file
		report({.message_ID_or_progress = ErrorType::PATH_INVALID, .failing_option = options.filename_in.string(), .compressing = {}});

	//swap ifs
	if (options.filename_in.empty() or options.filename_in.string().find_first_not_of(' ') == std::string::npos)//if filename_in is empty
	{
		if (options.concatenate_files)
			options.filename_in = fs::current_path();
		else if (!options.need_help)
			report({.message_ID_or_progress = ErrorType::MISSING_ARGUMENT, .failing_option = token_strings[static_cast<int>(TokenType::FILENAME_IN)].data(), .compressing = {}});
		else
			throw HelpException{};
	}

	if (options.filename_out == DEFAULT_OUT_PATH)
	{
		options.filename_out = options.filename_in.parent_path();
		options.filename_out /= DEFAULT_OUT_PATH;
		options.filename_out.replace_extension(FILE_EXTENSION);
	}

	if (options.filename_in == options.filename_out)
	{
		options.filename_out.replace_filename(options.filename_out.stem().string() + "_out" + FILE_EXTENSION);
	}

	return options;
}

}//namespace parser
