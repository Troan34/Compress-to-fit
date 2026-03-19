export module util;

import std;

export inline constexpr size_t N_FILES_LIMIT = 1'000;
export inline constexpr size_t SIZE_FILES_MIN = 512;
export inline constexpr const char* FILE_EXTENSION = ".tzf";

//All of this is COMPLETELY optional, but why not train our metaprogramming
export template <typename not_a_fun>
struct function_traits;//throws if not a fun

export template <typename fun, typename... args>
struct function_traits<fun(*)(args...)>
{
	using return_type = fun;

	static constexpr size_t num_args = sizeof...(args);

	template <size_t N>
	using arg = std::tuple_element_t<N, std::tuple<args...>>;//arg will be a "triple tuple", if you will
};

export template <typename fun>
concept ptr_size_pred = requires
{
	requires (function_traits<fun>::num_args == 2);
	requires std::is_pointer_v<typename function_traits<fun>::template arg<0>>;
	requires std::is_integral_v<typename function_traits<fun>::template arg<1>>;
};

/// <summary>
/// Parser error types
/// </summary>
export enum class ErrorType
{
	NO_ERROR,
	VALUE_ERROR,
	SYNTAX_ERROR,
	OPTION_UNAVAILABLE,
	PATH_NOT_FOUND,
	PATH_NOT_ACCESSIBLE,
	PATH_INVALID,
	FILE_INVALID,
	PORTIONS_OUT_OF_RANGE,
};

namespace ERR_STRING
{
	inline constexpr std::string_view VALUE = "\033[31merr\033[0m: this is an incorrect value for this option.";
	inline constexpr std::string_view SYNTAX = "\033[31merr\033[0m: the syntax for this option is incorrect.";
	inline constexpr std::string_view OPTION_UNAVAILABLE = "\033[31merr\033[0m: this option is unavailable in this context.";
	inline constexpr std::string_view PATH_NOT_FOUND = "\033[31merr\033[0m: this path could not be found, make sure to double quote (\") around your path if there are spaces in it";
	inline constexpr std::string_view PATH_NOT_ACCESSIBLE = "\033[31merr\033[0m: this path could not be accessed, the program may not have some required privileges";
	inline constexpr std::string_view PATH_INVALID = "\033[31merr\033[0m: this path is invalid, make sure to double quote (\") around your path if there are spaces in it";
	inline constexpr std::string_view FILE_INVALID = "\033[31merr\033[0m: this file is invalid. i.e. it is the wrong type or it is unrecognizable.";
	inline constexpr std::string_view PORTIONS_OUT_OF_RANGE = "\033[31merr\033[0m: the number of file portion is outside of the accepted range.";
}

export void throw_error(ErrorType error, const std::string& error_option = "")
{
	switch (error)
	{
	case ErrorType::NO_ERROR:
		break;
	case ErrorType::VALUE_ERROR:
		std::cerr << error_option + " <- " + ERR_STRING::VALUE.data();
		throw std::runtime_error(error_option + " <- " + ERR_STRING::VALUE.data());
		break;
	case ErrorType::SYNTAX_ERROR:
		std::cerr << error_option + " <- " + ERR_STRING::SYNTAX.data();
		throw std::runtime_error(error_option + " <- " + ERR_STRING::SYNTAX.data());
		break;
	case ErrorType::OPTION_UNAVAILABLE:
		std::cerr << error_option + " <- " + ERR_STRING::OPTION_UNAVAILABLE.data();
		throw std::runtime_error(error_option + " <- " + ERR_STRING::OPTION_UNAVAILABLE.data());
		break;
	case ErrorType::PATH_NOT_FOUND:
		std::cerr << error_option + " <- " + ERR_STRING::PATH_NOT_FOUND.data();
		throw std::runtime_error(error_option + " <- " + ERR_STRING::PATH_NOT_FOUND.data());
		break;
	case ErrorType::PATH_NOT_ACCESSIBLE:
		std::cerr << error_option + " <- " + ERR_STRING::PATH_NOT_ACCESSIBLE.data();
		throw std::runtime_error(error_option + " <- " + ERR_STRING::PATH_NOT_ACCESSIBLE.data());
		break;
	case ErrorType::PATH_INVALID:
		std::cerr << error_option + " <- " + ERR_STRING::PATH_INVALID.data();
		throw std::runtime_error(error_option + " <- " + ERR_STRING::PATH_INVALID.data());
		break;
	case ErrorType::FILE_INVALID:
		std::cerr << error_option + " <- " + ERR_STRING::FILE_INVALID.data();
		throw std::runtime_error(error_option + " <- " + ERR_STRING::FILE_INVALID.data());
		break;
	case ErrorType::PORTIONS_OUT_OF_RANGE:
		std::cerr << ERR_STRING::PORTIONS_OUT_OF_RANGE.data();
		throw std::runtime_error(ERR_STRING::PORTIONS_OUT_OF_RANGE.data());
		break;
	default:
		break;
	}
}

//unscoped because having to add static_cast becomes annoying
export enum CompPreset
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

//definition of the symbol type for compression
export struct Sym
{
	unsigned char value;

	operator int() const
	{
		return value;
	}
};