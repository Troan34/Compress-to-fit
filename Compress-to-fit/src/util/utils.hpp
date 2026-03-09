#pragma once
#include <type_traits>
#include <string>
#include <utility>

//All of this is COMPLETELY optional, but why not train our metaprogramming
template <typename not_a_fun>
struct function_traits;//throws if not a fun

template <typename fun, typename... args>
struct function_traits<fun(*)(args...)>
{
	using return_type = fun;

	static constexpr size_t num_args = sizeof...(args);

	template <size_t N>
	using arg = std::tuple_element_t<N, std::tuple<args...>>;//arg will be a "triple tuple", if you will
};

template <typename fun>
concept ptr_size_fun = requires
{
	requires (function_traits<fun>::num_args == 2);
	requires std::is_pointer_v<typename function_traits<fun>::template arg<0>>;
	requires std::is_integral_v<typename function_traits<fun>::template arg<1>>;
};

/// <summary>
	/// Parser error types
	/// </summary>
enum class ErrorType
{
	NO_ERROR,
	VALUE_ERROR,
	SYNTAX_ERROR,
	OPTION_UNAVAILABLE,
	PATH_NOT_FOUND,
	PATH_NOT_ACCESSIBLE,
	PATH_INVALID,
};

namespace ERR_STRING
{
	static constexpr const char* VALUE = "\033[31merr\033[0m: this is an incorrect value for this option.";
	static constexpr const char* SYNTAX = "\033[31merr\033[0m: the syntax for this option is incorrect.";
	static constexpr const char* OPTION_UNAVAILABLE = "\033[31merr\033[0m: this option is unavailable in this context.";
	static constexpr const char* PATH_NOT_FOUND = "\033[31merr\033[0m: this path could not be found, make sure to double quote (\") around your path if there are spaces in it";
	static constexpr const char* PATH_NOT_ACCESSIBLE = "\033[31merr\033[0m: this path could not be accessed, the program may not have some required privileges";
	static constexpr const char* PATH_INVALID = "\033[31merr\033[0m: this path is invalid, make sure to double quote (\") around your path if there are spaces in it";
}

void throw_error(ErrorType error, const std::string& error_option);

