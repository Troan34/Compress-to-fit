#pragma once
#include <stdexcept>
#include <string>
#include <print>

/**
 * @brief Parser error types.
 */
enum class ErrorType
{
	NO_ERROR,
	VALUE_ERROR,
	MISSING_ARGUMENT,
	SYNTAX_ERROR,
	OPTION_UNAVAILABLE,
	PATH_NOT_FOUND,
	PATH_NOT_ACCESSIBLE,
	PATH_INVALID,
	FILE_INVALID,
	FILE_CORRUPTED,
	DRIVE_ERROR,
	INVALID_DECOMPRESSION,
	DIR_COMPRESSION,
};

namespace ERR_STRING
{
	const std::string SYNTAX =				"\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::SYNTAX_ERROR)) + "]: the syntax for this option is incorrect. \033[34mTip\033[0m: '-h' or '-help' for help\n";

	const std::string VALUE =				"\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::VALUE_ERROR)) + "]: this is an incorrect value for this option.\033[0m\n";

	const std::string MISSING_ARGUMENT =	"\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::MISSING_ARGUMENT)) + "]: this argument is missing.\033[0m\n";

	const std::string OPTION_UNAVAILABLE =	"\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::OPTION_UNAVAILABLE)) + "]: this option is unavailable in this context.\033[0m\n";

	const std::string PATH_NOT_FOUND =		"\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::PATH_NOT_FOUND)) + "]: this path could not be found, make sure to double quote (\") around your path if there are spaces in it\033[0m\n";

	const std::string PATH_NOT_ACCESSIBLE = "\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::PATH_NOT_ACCESSIBLE)) + "]: this path could not be accessed, the program may not have some required privileges.\033[34mTip\033[0m:Such an error has multiple causes. Check path, folder and other things as such.\033[0m\n";

	const std::string PATH_INVALID =		"\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::PATH_INVALID)) + "]: this path is invalid for the selected operation (i.e. compressing a folder or concatenating a single file). Make sure to double quote (\" \") around your path if there are spaces in it.\033[0m\n";

	const std::string FILE_INVALID =		"\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::FILE_INVALID)) + "]: this file is invalid. i.e. it is the wrong type or it is unrecognizable.\033[0m\n";

	const std::string FILE_CORRUPTED =		"\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::FILE_CORRUPTED)) + "]: this file is corrupted. Unable to continue decompression properly.\033[0m\n";

	const std::string DRIVE_ERROR =			"\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::DRIVE_ERROR)) + "]: there has been a critical error when accessing this file. Please check your drive.\033[0m\n";

	const std::string INVALID_DECOMPRESSION="\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::INVALID_DECOMPRESSION)) + "]: decompression is invalid on this file, you may have tried to decompress a normal file.\033[0m\n";

	const std::string DIR_COMPRESSION =		"\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::DIR_COMPRESSION)) + "]: folder compression is unavailable. \033[34mTip\033[0m: If you are trying to decompress a folder of files, check the related command in the help page with '-h' or '-help'\n";

}

/**
 * @brief throw_error is a custom error logger to the terminal. This will WILL throw.
 * @param error Your type of error.
 * @param error_option An optional string to be added at the start, could be a path, text...
 * @throw runtime_exception
 */
inline void throw_error(ErrorType const error, const std::string& error_option = "")
{
	switch (error)
	{
	case ErrorType::NO_ERROR:
		break;
	case ErrorType::VALUE_ERROR:
		std::print("{} <- {}", error_option, ERR_STRING::VALUE);
		throw std::runtime_error(error_option + " <- " + ERR_STRING::VALUE);
		break;
	case ErrorType::SYNTAX_ERROR:
		std::print("{} <- {}", error_option, ERR_STRING::SYNTAX);
		throw std::runtime_error(error_option + " <- " + ERR_STRING::SYNTAX);
		break;
	case ErrorType::OPTION_UNAVAILABLE:
		std::print("{} <- {}", error_option, ERR_STRING::OPTION_UNAVAILABLE);
		throw std::runtime_error(error_option + " <- " + ERR_STRING::OPTION_UNAVAILABLE);
		break;
	case ErrorType::PATH_NOT_FOUND:
		std::print("{} <- {}", error_option, ERR_STRING::PATH_NOT_FOUND);
		throw std::runtime_error(error_option + " <- " + ERR_STRING::PATH_NOT_FOUND);
		break;
	case ErrorType::PATH_NOT_ACCESSIBLE:
		std::print("{} <- {}", error_option, ERR_STRING::PATH_NOT_ACCESSIBLE);
		throw std::runtime_error(error_option + " <- " + ERR_STRING::PATH_NOT_ACCESSIBLE);
		break;
	case ErrorType::PATH_INVALID:
		std::print("{} <- {}", error_option, ERR_STRING::PATH_INVALID);
		throw std::runtime_error(error_option + " <- " + ERR_STRING::PATH_INVALID);
		break;
	case ErrorType::FILE_INVALID:
		std::print("{} <- {}", error_option, ERR_STRING::FILE_INVALID);
		throw std::runtime_error(error_option + " <- " + ERR_STRING::FILE_INVALID);
		break;
	case ErrorType::FILE_CORRUPTED:
		std::print("{} <- {}", error_option, ERR_STRING::FILE_CORRUPTED);
		throw std::runtime_error(error_option + " <- " + ERR_STRING::FILE_CORRUPTED);
		break;
	case ErrorType::DRIVE_ERROR:
		std::print("{} <- {}", error_option, ERR_STRING::DRIVE_ERROR);
		throw std::runtime_error(error_option + " <- " + ERR_STRING::DRIVE_ERROR);
		break;
	case ErrorType::INVALID_DECOMPRESSION:
		std::print("{} <- {}", error_option, ERR_STRING::INVALID_DECOMPRESSION);
		throw std::runtime_error(error_option + " <- " + ERR_STRING::INVALID_DECOMPRESSION);
		break;
	case ErrorType::MISSING_ARGUMENT:
		std::print("{} <- {}", error_option, ERR_STRING::MISSING_ARGUMENT);
		throw std::runtime_error(error_option + " <- " + ERR_STRING::MISSING_ARGUMENT);
		break;
	case ErrorType::DIR_COMPRESSION:
		std::print("{} <- {}", error_option, ERR_STRING::DIR_COMPRESSION);
		throw std::runtime_error(error_option + " <- " + ERR_STRING::DIR_COMPRESSION);
		break;
	default:
		assert(false);
		break;
	}
}

enum class WarningType
{
	RECOMPRESSION,
	PORTIONS_OUT_OF_RANGE,
	CONCAT_AMBIGUITY,
	CONCURRENCY_OUT_OF_RANGE_UPPER,
	CONCURRENCY_OUT_OF_RANGE_LOWER,
};

namespace WARN_STRING
{
	const std::string RECOMPRESSION =			"\033[43mWarn[" + std::to_string(static_cast<int>(WarningType::RECOMPRESSION)) +
		"]\033[0m: the file being compressed has already been compressed."
		+ "\033[34mTip\033[0m: A file recompression gives negligible, if not counter-productive, results.\n";

	const std::string PORTIONS_OUT_OF_RANGE =	"\033[43mWarn\033[0m[" + std::to_string(static_cast<int>(WarningType::PORTIONS_OUT_OF_RANGE)) +
		"]: the number of file portions is outside of the accepted range."
		+ "\033[34mTip\033[0m: The (not split) output file may have been too small.\nThe number of files created may be different from what you asked.\n";

	const std::string CONCAT_AMBIGUITY =		"\033[43mWarn\033[0m[" + std::to_string(static_cast<int>(WarningType::PORTIONS_OUT_OF_RANGE)) +
		"]: there has been found an ambiguity while concatenating. These files do not come from the same compressed file(same session).\n";

	const std::string CONCURRENCY_OUT_OF_RANGE_UPPER ="\033[43mWarn\033[0m[" + std::to_string(static_cast<int>(WarningType::CONCURRENCY_OUT_OF_RANGE_UPPER)) +
		"]: you have selected more threads than available on your machine, will default to using all your threads.";

	const std::string CONCURRENCY_OUT_OF_RANGE_LOWER ="\033[43mWarn\033[0m[" + std::to_string(static_cast<int>(WarningType::CONCURRENCY_OUT_OF_RANGE_UPPER)) +
		"]: you have selected zero or a negative number of threads, will default to use only one thread.";
}

inline void print_warn(WarningType const warn, const std::string& warn_option = "")
{
	switch (warn)
	{
		case WarningType::RECOMPRESSION:
		std::print("{}", WARN_STRING::RECOMPRESSION);
		break;
	case WarningType::PORTIONS_OUT_OF_RANGE:
		std::print("{} <- {}", warn_option, WARN_STRING::PORTIONS_OUT_OF_RANGE);
		break;
	case WarningType::CONCAT_AMBIGUITY:
		std::print("{} <- {}", warn_option, WARN_STRING::CONCAT_AMBIGUITY);
		break;
	case WarningType::CONCURRENCY_OUT_OF_RANGE_UPPER:
		std::print("{} <- {}", warn_option, WARN_STRING::CONCURRENCY_OUT_OF_RANGE_UPPER);
		break;
	case WarningType::CONCURRENCY_OUT_OF_RANGE_LOWER:
		std::print("{} <- {}", warn_option, WARN_STRING::CONCURRENCY_OUT_OF_RANGE_LOWER);
		break;
	}
}