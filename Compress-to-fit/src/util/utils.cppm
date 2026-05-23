export module util;

import std.compat;
namespace fs = std::filesystem;


export inline constexpr size_t N_FILES_LIMIT = 1'000;
export inline constexpr size_t SIZE_FILES_MIN = 512;
export inline constexpr char const* FILE_EXTENSION = ".tzf";
export constexpr size_t SIZE_CHUNK = 4 * 1024 * 1024;//4MiB


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




template<typename T>
struct is_vector : std::false_type {};

template<typename T, typename A>
struct is_vector<std::vector<T, A>> : std::true_type {};

template<typename T>
concept is_std_vector = is_vector<T>::value;

export template <typename fun>
concept ptr_size_pred = requires
{
	requires (function_traits<fun>::num_args == 2);
	requires std::is_pointer_v<typename function_traits<fun>::template arg<0>>;
	requires std::is_integral_v<typename function_traits<fun>::template arg<1>>;
};

export template <typename fun>
concept size_pred = std::invocable<fun, size_t>;



/**
 * @brief Parser error types.
 */
export enum class ErrorType
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

	const std::string FILE_CORRUPTED =		"\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::FILE_CORRUPTED)) + "]: this file's header is corrupted. Unable to continue decompression properly.\033[0m\n";

	const std::string DRIVE_ERROR =			"\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::DRIVE_ERROR)) + "]: there has been a critical error when accessing this file. Please check your drive.\033[0m\n";

	const std::string INVALID_DECOMPRESSION="\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::INVALID_DECOMPRESSION)) + "]: decompression is invalid on this file, you may have tried to decompress a normal file.\033[0m\n";

	const std::string DIR_COMPRESSION =		"\033[41mError\033[0m\033[31m[" + std::to_string(static_cast<int>(ErrorType::DIR_COMPRESSION)) + "]: folder compression is unvailable. \033[34mTip\033[0m: If you are trying to decompress a folder of files, check the related command in the help page with '-h' or '-help'\n";

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
		std::print("{} <- {}", error_option, ERR_STRING::FILE_INVALID);
		throw std::runtime_error(error_option + " <- " + ERR_STRING::FILE_INVALID);
		break;
	case ErrorType::DRIVE_ERROR:
		std::print("{} <- {}", error_option, ERR_STRING::DRIVE_ERROR);
		throw std::runtime_error(error_option + " <- " + ERR_STRING::DRIVE_ERROR);
		break;
	case ErrorType::INVALID_DECOMPRESSION:
		std::print("{} <- {}", error_option, ERR_STRING::INVALID_DECOMPRESSION);
		throw std::runtime_error(error_option + " <- " + ERR_STRING::INVALID_DECOMPRESSION);
		break;
	}
}

export enum class WarningType
{
	RECOMPRESSION,
	PORTIONS_OUT_OF_RANGE,
	CONCAT_AMBIGUITY,

};

namespace WARN_STRING
{
	const std::string RECOMPRESSION =			"\033[43mWarn[" + std::to_string(static_cast<int>(WarningType::RECOMPRESSION)) + "]\033[0m: the file being compressed has already been compressed."
		+ "\033[34mTip\033[0m: A file recompression gives negligible, if not counter-productive, results.\n";

	const std::string PORTIONS_OUT_OF_RANGE =	"\033[43mWarn\033[0m[" + std::to_string(static_cast<int>(WarningType::PORTIONS_OUT_OF_RANGE)) + "]: the number of file portions is outside of the accepted range."
		+ "\033[34mTip\033[0m: The (not split) output file may have been too small.\nThe number of files created may be different from what you asked.\n";

	const std::string CONCAT_AMBIGUITY = "\033[43mWarn\033[0m[" + std::to_string(static_cast<int>(WarningType::PORTIONS_OUT_OF_RANGE)) + "]: there has been found an ambiguity while concatenating. These files do not come from the same compressed file(same session).\n";
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
	case WarningType::CONCAT_AMBIGUITY:
		std::cout << warn_option << " <- " + WARN_STRING::CONCAT_AMBIGUITY;
	}
}

//unscoped because having to add static_cast becomes annoying
export enum CompPreset : uint8_t
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
 * @note You must not reorder the enums or (especially) remove MAX.
 *		 For every new enum, MAX shall always stay the max enum possible.
 *		 DO NOT CUSTOMIZE THE VALUES. YOU WILL MAKE PREEXISTING .tzf FILES INVALID.
 */
export enum class CompType : uint8_t
{
	LZ77,
	MAX,
};

export constexpr std::string_view COMPRESSOR_STR_OPTIONS[] =
{
	"LZ77",
};

template <typename Iter>
struct ForwardIterator
{
	using iterator_concept = std::forward_iterator_tag;
	using iterator_category = std::forward_iterator_tag;
	using value_type = typename std::iterator_traits<Iter>::value_type;
	using difference_type = typename std::iterator_traits<Iter>::difference_type;
	using pointer = typename std::iterator_traits<Iter>::pointer;
	using reference = typename std::iterator_traits<Iter>::reference;

	ForwardIterator(Iter begin_, Iter const end_)
		:iterator(begin_), end(end_)
	{

	}
	Iter iterator;
	Iter const end;

	ForwardIterator& operator++() { ++iterator; return *this; }
	ForwardIterator operator++(int) { ForwardIterator temp = *this; ++iterator; return temp; }
	ForwardIterator& operator+=(size_t const size) { iterator += size; return *this; }


	reference operator*() { return *iterator; }

	[[nodiscard]] bool operator==(ForwardIterator const& other) const { return iterator == other.iterator; };
	[[nodiscard]] bool operator!=(ForwardIterator const& other) const { return iterator != other.iterator; };

	[[nodiscard]] bool reached_end() const { return iterator >= end; };
	[[nodiscard]] size_t distance_to_end() const { return static_cast<size_t>(end - iterator); };

};

export template <typename InType, typename OutType>
struct CodecInterface
{
	ForwardIterator<InType const*> in_data;
	std::vector<OutType>& out_data;
};


//definition of the symbol type for compression
export struct Sym
{
	unsigned char value;


	operator unsigned char() const
	{
		return value;
	}


	friend std::istream& operator>>(std::istream& is, Sym& sym)
	{
		is >> sym.value;
		return is;
	}

	auto operator<=>(Sym const&) const = default;

	[[nodiscard]] static constexpr auto max() noexcept
	{
		return std::numeric_limits<decltype(value)>::max();
	}
	[[nodiscard]] static constexpr auto alphabet_size() noexcept
	{
		return max() + 1;
	}

};

export [[nodiscard]] constexpr size_t KiB_to_B(size_t value)
{
	return value * 1024;
}

export [[nodiscard]] constexpr size_t MiB_to_B(size_t value)
{
	return value * 1024 * 1024;
}

/**
* @brief Calculate the power at compile time, to make sure of its correct usage it will be consteval
*/
export [[nodiscard]] consteval size_t const_pow(size_t base, size_t exponent) noexcept
{
	size_t result = 1;
	for (int i = 0; i < exponent; i++)
	{
		result *= base;
	}
	return result;
}


/**
 * @brief Counts the number of consecutive equal elements from the start of two spans, up to a specified maximum.
 * @tparam Type The element type in the spans, Type must support equality comparison.
 * @param str1 The first sequence to compare.
 * @param str2 The second sequence to compare.
 * @param max_match Maximum number of elements to compare. Comparison stops when this limit is reached.
 * @return The count of initial consecutive elements that are equal in both spans, constrained by the shorter span length and max_match.
 */
export template <typename Type>
[[nodiscard]] constexpr auto count_equal(std::span<Type> const str1, std::span<Type> const str2, size_t const max_match)
{
	auto cond = std::min(str1.size(), str2.size());
	size_t i = 0;
	for (; i < cond and i < max_match; i++)
	{
		if (str1[i] != str2[i])
			break;
	}
	return i;
}


