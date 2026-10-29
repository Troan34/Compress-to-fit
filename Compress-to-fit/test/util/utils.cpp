#include <catch2/catch_test_macros.hpp>

import std;
import util;

TEST_CASE("Count matching substring size", "[util]")
{
	std::array<int, 4> arr1{ 0, 2, 4, 12 };
	std::array<int, 6> arr2{ 0, 2, 2, 31, 312, 234 };

	REQUIRE(count_equal(std::span<int>{arr1}, std::span<int>{arr2}, 10) == 2);
	REQUIRE(count_equal(std::span<int>{arr1}, std::span<int>{arr2}, 0) == 0);
	REQUIRE(count_equal(std::span<int>{arr1}, std::span<int>{arr2}, 1) == 1);
}

TEST_CASE("Verify to_bytes[_le] utils", "[util]")
{
	float f32_lowest = std::numeric_limits<float>::lowest();
	float f32_highest = std::numeric_limits<float>::max();
	float f32_min = std::numeric_limits<float>::min();
	double f64_lowest = std::numeric_limits<double>::lowest();
	double f64_highest = std::numeric_limits<double>::max();
	double f64_min = std::numeric_limits<double>::min();
	int i32_lowest = std::numeric_limits<int>::lowest();
	int i32_highest = std::numeric_limits<int>::max();
	int i32_min = std::numeric_limits<int>::min();
	unsigned char u8_lowest = std::numeric_limits<unsigned char>::lowest();
	unsigned char u8_highest = std::numeric_limits<unsigned char>::max();
	unsigned char u8_min = std::numeric_limits<unsigned char>::min();

	//TODO: we have to test operator""_B
	REQUIRE(to_bytes(f32_lowest) == std::array<std::byte, 4>{0b1111'1111_B, 0b0111'1111_B, 0b1111'1111_B, 0b1111'1111_B});//1 11111110 11111111111111111111111
}