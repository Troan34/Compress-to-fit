export module util;

import std.compat;
namespace fs = std::filesystem;


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

/**
 * @brief Parser error types.
 */
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
	FILE_CORRUPTED,
};

namespace ERR_STRING
{
	const std::string SYNTAX =				"\033[41mError [" + std::to_string(static_cast<int>(ErrorType::SYNTAX_ERROR)) + "]\033[31m: the syntax for this option is incorrect. \033[31mTip\033[0m: [-h] or [-help] ";

	const std::string VALUE =				"\033[41mError \033[0m ["+ std::to_string(static_cast<int>(ErrorType::VALUE_ERROR)) + "]\033[31m: this is an incorrect value for this option.\033[0m";

	const std::string OPTION_UNAVAILABLE = "\033[41mError\033[0m [" + std::to_string(static_cast<int>(ErrorType::OPTION_UNAVAILABLE)) + "]\033[31m: this option is unavailable in this context.\033[0m";

	const std::string PATH_NOT_FOUND = "\033[41mError\033[0m [" + std::to_string(static_cast<int>(ErrorType::PATH_NOT_FOUND)) + "]\033[31m: this path could not be found, make sure to double quote (\") around your path if there are spaces in it\033[0m";

	const std::string PATH_NOT_ACCESSIBLE = "\033[41mError\033[0m [" + std::to_string(static_cast<int>(ErrorType::PATH_NOT_ACCESSIBLE)) + "]\033[31m: this path could not be accessed, the program may not have some required privileges.\nSuch an error has multiple causes. Check path, folder and other things as such.\033[0m";

	const std::string PATH_INVALID = "\033[41mError\033[0m [" + std::to_string(static_cast<int>(ErrorType::PATH_INVALID)) + "]\033[31m: this path is invalid, make sure to double quote (\" \") around your path if there are spaces in it.\033[0m";

	const std::string FILE_INVALID = "\033[41mError\033[0m [" + std::to_string(static_cast<int>(ErrorType::FILE_INVALID)) + "]\033[31m: this file is invalid. i.e. it is the wrong type or it is unrecognizable.\033[0m";

	const std::string FILE_CORRUPTED = "\033[41mError\033[0m [" + std::to_string(static_cast<int>(ErrorType::FILE_CORRUPTED)) + "]\033[31m: this file's header is corrupted. Unable to continue decompression properly.\033[0m";
}

/**
 * @brief throw_error is a custom error logger to the terminal. This will WILL throw.
 * @param error Your type of error.
 * @param error_option An optional string to be added at the start, could be a path, text...
 * @throw runtime_exception
 */
export void throw_error(ErrorType error, const std::string& error_option = "")
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
		std::cout << error_option + " <- " + ERR_STRING::SYNTAX;
		throw std::runtime_error(error_option + " <- " + ERR_STRING::SYNTAX);
		break;
	case ErrorType::OPTION_UNAVAILABLE:
		std::cerr << error_option + " <- " + ERR_STRING::OPTION_UNAVAILABLE;
		throw std::runtime_error(error_option + " <- " + ERR_STRING::OPTION_UNAVAILABLE);
		break;
	case ErrorType::PATH_NOT_FOUND:
		std::cout << error_option + " <- " + ERR_STRING::PATH_NOT_FOUND;
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
	case ErrorType::FILE_INVALID:
		std::cerr << error_option + " <- " + ERR_STRING::FILE_INVALID;
		throw std::runtime_error(error_option + " <- " + ERR_STRING::FILE_INVALID);
		break;
	case ErrorType::FILE_CORRUPTED:
		std::cerr << error_option + " <- " + ERR_STRING::FILE_INVALID;
		throw std::runtime_error(error_option + " <- " + ERR_STRING::FILE_INVALID);
		break;
	}
}

export enum class WarningType
{
	RECOMPRESSION,
	PORTIONS_OUT_OF_RANGE,

};

namespace WARN_STRING
{
	const std::string RECOMPRESSION = "\033[43mWarn [" + std::to_string(static_cast<int>(WarningType::RECOMPRESSION)) + "]\033[0m: the file being compressed has already been compressed.\n"
		+ "\033[31mTip\033[0m: A file recompression gives negligible, if not counter-productive, results.\n";

	const std::string PORTIONS_OUT_OF_RANGE = "\033[43mWarn\033[0m [" + std::to_string(static_cast<int>(WarningType::PORTIONS_OUT_OF_RANGE)) + "]: the number of file portions is outside of the accepted range.\n"
	+ "\033[31mTip\033[0m: The output file (not split) may have been too small.\nThe number of files created may be different from what you asked.\n";

}

export void print_warn(WarningType warn, const std::string& warn_option = "")
{
	switch (warn)
	{
	case WarningType::RECOMPRESSION:
		std::cout << WARN_STRING::RECOMPRESSION;
		break;
	case WarningType::PORTIONS_OUT_OF_RANGE:
		std::cout << warn_option << " <- " + WARN_STRING::PORTIONS_OUT_OF_RANGE;
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

/**
 * @brief Identify compressor used
 * 
 * @note You must not reorder the enums or (and especially) remove MAX.
 *		 For every new enum, MAX shall always stay the max enum possible.
 *		 DO NOT CUSTOMIZE THE VALUES.
 */
export enum class CompType : uint32_t
{
	LZ77,
	MAX,
};

constexpr std::string_view comp_strings[] =
{
	"LZ77",
};


//definition of the symbol type for compression
export struct Sym
{
	unsigned char value;

	operator int() const
	{
		return static_cast<int>(value);
	}

	friend std::istream& operator>>(std::istream& is, Sym& sym)
	{
		is >> sym.value;
		return is;
	}

	auto operator<=>(const Sym&) const = default;

	[[nodiscard]] static constexpr auto max() noexcept
	{
		return std::numeric_limits<decltype(value)>::max();
	}
	[[nodiscard]] static constexpr auto alphabet_size() noexcept
	{
		return max() + 1;
	}

};

export [[nodiscard]] constexpr size_t KB_to_B(size_t value)
{
	return value * 1024;
}

export [[nodiscard]] constexpr size_t MB_to_B(size_t value)
{
	return value * 1024 * 1024;
}

/**
* @brief Calculate the power at compile time, to make sure of its correct usage it will be consteval
*/
export [[nodiscard]] constexpr size_t const_pow(size_t base, size_t exponent) noexcept
{
	size_t result = 1;
	for (int i = 0; i < exponent; i++)
	{
		result *= base;
	}
	return result;
}

export template <typename Type>
[[nodiscard]] constexpr auto count_equal(const std::span<Type> str1, const std::span<Type> str2)
{
	auto cond = std::min(str1.size(), str2.size());
	size_t i = 0;
	for (; i < cond; i++)
	{
		if (str1[i] != str2[i])
			break;
	}
	return i;
}

