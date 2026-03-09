#include "utils.hpp"
#include <exception>
#include <cassert>
#include <iostream>

void throw_error(ErrorType error, const std::string& error_option)
{
	switch (error)
	{
	case ErrorType::NO_ERROR:
		break;
	case ErrorType::VALUE_ERROR:
		std::cerr << error_option + " <- " + ERR_STRING::VALUE;
		throw std::runtime_error(error_option + " <- " + ERR_STRING::VALUE);
		break;
	case ErrorType::SYNTAX_ERROR:
		std::cerr << error_option + " <- " + ERR_STRING::SYNTAX;
		throw std::runtime_error(error_option + " <- " + ERR_STRING::SYNTAX);
		break;
	case ErrorType::OPTION_UNAVAILABLE:
		std::cerr << error_option + " <- " + ERR_STRING::OPTION_UNAVAILABLE;
		throw std::runtime_error(error_option + " <- " + ERR_STRING::OPTION_UNAVAILABLE);
		break;
	case ErrorType::PATH_NOT_FOUND:
		std::cerr << error_option + " <- " + ERR_STRING::PATH_NOT_FOUND;
		throw std::runtime_error(error_option + " <- " + ERR_STRING::PATH_NOT_FOUND);
		break;
	case ErrorType::PATH_NOT_ACCESSIBLE:
		std::cerr << error_option + " <- " + ERR_STRING::PATH_NOT_ACCESSIBLE;
		throw std::runtime_error(error_option + " <- " + ERR_STRING::PATH_NOT_ACCESSIBLE);
		break;
	case ErrorType::PATH_INVALID:
		std::cerr << error_option + " <- " + ERR_STRING::PATH_INVALID;
		throw std::runtime_error(error_option + " <- " + ERR_STRING::PATH_INVALID);
		break;
	default:
		break;
	}
}