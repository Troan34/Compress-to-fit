module;
#include <cassert>
#include "../../src/common/comp_options.hpp"
#include "../../src/common/error_warn_print.hpp"
export module util:core_utils;

#ifdef __INTELLISENSE__
#include "../../for_intellisense/everything.hpp"
#endif

import std.compat;


/*Export stuff from common*/
export using ::CompType;
export using ::CompPreset;
export using ::COMPRESSOR_STR_OPTIONS;


namespace fs = std::filesystem;


export inline constexpr size_t N_FILES_LIMIT = 1'000;
export inline constexpr size_t SIZE_FILES_MIN = 512;
export inline constexpr char const* FILE_EXTENSION = ".tzf";
export constexpr size_t SIZE_CHUNK = 4ULL * 1024 * 1024;//4MiB


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
export [[nodiscard]] consteval size_t const_pow(size_t const base, size_t const exponent) noexcept
{
	size_t result = 1;
	for (size_t i = 0; i < exponent; i++)
	{
		result *= base;
	}
	return result;
}

export [[nodiscard]] constexpr size_t KiB_to_B(size_t const value)
{
	return value * 1024;
}

export [[nodiscard]] constexpr size_t MiB_to_B(size_t const value)
{
	return value * 1024 * 1024;
}

export constexpr unsigned long long operator""_MiB(unsigned long long const value)
{
	if (value * 1024ULL * 1024ULL >= std::numeric_limits<unsigned long long>::max()) {
		throw std::out_of_range("Your value will cause an overflow.");
	}
	return MiB_to_B(value);
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


template <typename T>
concept MemberWriteable = requires(T const& a, std::ofstream& file)
{
	a.write_to(file);
};

template <typename T>
concept FreeWriteable = requires(T const& a, std::ofstream& file)
{
	write_to(a, file);
};

export template<typename T>
concept SerializableToDisk = std::is_trivially_copyable_v<T> or MemberWriteable<T> or FreeWriteable<T>;

template<SerializableToDisk T>
void do_write_to(T const& t, std::ofstream& file)
{
	if constexpr (std::is_trivially_copyable_v<T> and !MemberWriteable<T> and !FreeWriteable<T>)
	{
		file.write(reinterpret_cast<char const *>(&t), sizeof(T));
	}
	else if constexpr (MemberWriteable<T>)
	{
		t.write_to(file);
	}
	else
	{
		write_to(t, file);
	}
}


export template<std::ranges::input_range R>
	requires SerializableToDisk<std::ranges::range_value_t<R>> or SerializableToDisk<R>
void write_to_disk(R const& range, std::ofstream& file)
{
	if constexpr (SerializableToDisk<R>)
	{
		do_write_to(range, file);
	}
	else
	{
		for (auto const& elem : range)
		{
			do_write_to(elem, file);
		}
	}
}


