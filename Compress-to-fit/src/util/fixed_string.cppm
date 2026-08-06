export module util:fixed_string;
import std;


/**
 * @brief An owning string with stack duration.
 * @tparam N Size of string
 */
template <std::size_t N>
class FixedString
{
public:
    consteval FixedString(char const (&str)[N])
    {
        std::copy_n(str, N, str_.begin());
    }
    constexpr FixedString() = default;

    [[nodiscard]] constexpr auto size() const noexcept -> std::size_t
    {
        return N - 1;
    }

    [[nodiscard]] constexpr auto data() const noexcept
    {
        return str_.data();
    }

    [[nodiscard]] constexpr auto c_str() const noexcept
    {
        return str_.data();
    }

    [[nodiscard]] constexpr operator std::string_view() const noexcept
    {
        return {str_.data(), str_.size()};
    }

    [[nodiscard]] constexpr auto operator[](std::size_t const index) const noexcept
    {
        return data()[index];
    }

    [[nodiscard]] constexpr auto& operator[](std::size_t const index) noexcept
    {
        return str_[index];
    }

    [[nodiscard]] constexpr auto begin() const noexcept
    {
        return data();
    }

    [[nodiscard]] constexpr auto end() const noexcept
    {
        return data() + size();
    }


private:
    std::array<char,N> str_{};

    template <std::size_t, std::size_t>
    friend constexpr auto operator+(FixedString, FixedString);
};


/**
 * @brief Concatenate two FixedString(s)
 * @param lhs
 * @param rhs
 * @return The new string
 */
export template <std::size_t N1, std::size_t N2>
constexpr auto operator+(FixedString<N1> lhs, FixedString<N2> rhs)
{
    FixedString<N1 + N2 - 1> result;

    for (std::size_t i = 0; i < N1 - 1; ++i)
        result[i] = lhs[i];

    for (std::size_t i = 0; i < N2; ++i)
        result[i + N1 - 1] = rhs[i];

    return result;
}

export template <std::size_t N1, std::size_t N2>
constexpr auto operator+(char const (&lhs)[N1], FixedString<N2> rhs)
{
    return FixedString{lhs} + rhs;
}

export template <std::size_t N1, std::size_t N2>
constexpr auto operator+(FixedString<N1> lhs, char const (&rhs)[N2])
{
    return lhs + FixedString{rhs};
}