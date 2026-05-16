#include <catch2/catch_test_macros.hpp>
#include <array>

import util;

TEST_CASE("Count matching substring size", "count_equal")
{
	std::array<int, 4> arr1{ 0, 2, 4, 12 };
	std::array<int, 6> arr2{ 0, 2, 2, 31, 312, 234 };

	REQUIRE(count_equal(std::span<int>{arr1}, std::span<int>{arr2}, 10) == 2);
	REQUIRE(count_equal(std::span<int>{arr1}, std::span<int>{arr2}, 0) == 0);
	REQUIRE(count_equal(std::span<int>{arr1}, std::span<int>{arr2}, 1) == 1);
}
