/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*/

module;
#include "../../src/common/comp_options.hpp"
#include "../../src/common/error_warn_print.hpp"
export module util.core_utils;

#ifdef __INTELLISENSE__
#include "../../for_intellisense/everything.hpp"
#endif

import std.compat;


/*Export stuff from common*/
export using ::CompType;
export using ::CompPreset;
export using ::COMPRESSOR_STR_OPTIONS;


namespace fs = std::filesystem;


export using ::SIZE_FILES_MIN;
export using ::N_FILES_LIMIT;
export inline constexpr char const* FILE_EXTENSION = ".tzf";
export constexpr size_t SIZE_CHUNK = 4ULL * 1024 * 1024;//4MiB


//All of this is COMPLETELY optional, but why not train our metaprogramming. P.S. also C++26 will add parameters_of, but this is what we have to do for now
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

/*Export error and warning managers from common*/
export using ::ErrorType;
export using ::throw_error;

export using ::WarningType;
export using ::print_warn;


export template <typename Iter>
struct ForwardIterator
{
	using iterator_concept	=	std::forward_iterator_tag;
	using iterator_category =	std::forward_iterator_tag;
	using value_type		=	std::iterator_traits<Iter>::value_type;
	using difference_type	=	std::iterator_traits<Iter>::difference_type;
	using pointer			=	std::iterator_traits<Iter>::pointer;
	using reference			=	std::iterator_traits<Iter>::reference;

	ForwardIterator(Iter begin_, Iter const end_)
		:iterator(begin_), end(end_)
	{

	}
	Iter iterator;
	Iter const end;

	auto operator++() -> ForwardIterator& { ++iterator; return *this; }
	auto operator++(int) -> ForwardIterator { ForwardIterator temp = *this; ++iterator; return temp; }
	auto operator+=(size_t const size) -> ForwardIterator& { iterator += size; return *this; }


	auto operator*() -> reference { return *iterator; }

	[[nodiscard]] auto operator==(ForwardIterator const& other) const -> bool { return iterator == other.iterator; }
	[[nodiscard]] auto operator!=(ForwardIterator const& other) const -> bool { return iterator != other.iterator; }

	[[nodiscard]] auto reached_end() const -> bool { return iterator >= end; }
	[[nodiscard]] size_t distance_to_end() const { return static_cast<size_t>(end - iterator); }

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


/**
* @brief Calculate the power at compile time, to make sure of its correct usage it will be consteval
*/
export [[nodiscard]] consteval auto const_pow(size_t const base, size_t const exponent) noexcept -> size_t
{
	size_t result = 1;
	for (size_t i = 0; i < exponent; i++)
	{
		result *= base;
	}
	return result;
}

export [[nodiscard]] constexpr auto KiB_to_B(size_t const value) -> size_t
{
	if (value * 1024UZ >= std::numeric_limits<size_t>::max())
	{
		throw std::out_of_range("Your value will cause an overflow.");
	}
	return value * 1024UZ;
}

export [[nodiscard]] constexpr auto MiB_to_B(size_t const value) -> size_t
{
	if (value * 1024UZ * 1024UZ >= std::numeric_limits<size_t>::max())
	{
		throw std::out_of_range("Your value will cause an overflow.");
	}
	return value * 1024UZ * 1024UZ;
}

export constexpr auto operator""_MiB(unsigned long long const value) -> unsigned long long
{
	return MiB_to_B(value);
}

export constexpr auto operator""_KiB(unsigned long long const value) -> unsigned long long
{
	return KiB_to_B(value);
}

export constexpr auto operator""_B(unsigned long long const value) -> std::byte {
	return static_cast<std::byte>(value);
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

/**
 * @brief Format a `source_location` as a human readable string
 * @param loc The source_location, better left as default
 * @return Human readable string
 * @details The resulting string is structured in the following way: \n
 *			@code file: {file_name}({line}:{col}) `{fun_name}` @endcode
 */
export [[nodiscard]] constexpr auto format_sourceloc(std::source_location const loc = std::source_location::current())
{
	return std::string{std::format("file: {}({}:{}) `{}`", loc.file_name(), loc.line(), loc.column(), loc.function_name())};
}



/**
 * @brief Turn a primitive into a little endian byte representation
 */
export template <std::integral T>
	requires (not std::same_as<T, bool>)
constexpr auto to_bytes_le(T val) -> std::array<std::byte, sizeof(T)> {
	using U = std::make_unsigned_t<T>;
	U unsigned_val = static_cast<U>(val);
	std::array<std::byte, sizeof(U)> out;
	if constexpr (std::endian::native == std::endian::little) {
		std::memcpy(out.data(), &unsigned_val, sizeof(U));
	} else {
		for (std::size_t i = 0; i < sizeof(U); ++i)
		{
			out[i] = static_cast<std::byte>(unsigned_val >> (8 * (sizeof(U) - 1 - i)));
		}
	}
	return out;
}

/**
 * @brief Turn a primitive into its byte representation
 */
export template <std::integral T>
	requires (not std::same_as<T, bool>)
constexpr auto to_bytes(T val) -> std::array<std::byte, sizeof(T)> {
	using U = std::make_unsigned_t<T>;
	U unsigned_val = static_cast<U>(val);
	std::array<std::byte, sizeof(U)> out;
	std::memcpy(out.data(), &unsigned_val, sizeof(U));
	return out;
}

/**
 * @brief Turn a primitive into its byte representation
 */
export template <std::floating_point T>
constexpr auto to_bytes_le(T val) -> std::array<std::byte, sizeof(T)> {
	static_assert(std::numeric_limits<T>::is_iec559, "IEEE-754 required");
	if constexpr (sizeof(T) == 4)
		return to_bytes_le(std::bit_cast<std::uint32_t>(val));
	else if constexpr (sizeof(T) == 8)
		return to_bytes_le(std::bit_cast<std::uint64_t>(val));
	static_assert("Floating point size greater than 8 bytes");
}

/**
 * @brief Turn a primitive into a little endian byte representation
 */
export template <std::floating_point T>
constexpr auto to_bytes(T val) -> std::array<std::byte, sizeof(T)> {
	static_assert(std::numeric_limits<T>::is_iec559, "IEEE-754 required");
	if constexpr (sizeof(T) == 4)
		return to_bytes(std::bit_cast<std::uint32_t>(val));
	else if constexpr (sizeof(T) == 8)
		return to_bytes(std::bit_cast<std::uint64_t>(val));
	static_assert("Floating point size greater than 8 bytes");
}

