#include <catch2/catch_test_macros.hpp>

import std.compat;
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
	float f32_lowest = std::numeric_limits<float>::lowest();//1 11111110 11111111111111111111111
	float f32_highest = std::numeric_limits<float>::max();//0x7F7FFFFF
	double f64_lowest = std::numeric_limits<double>::lowest();//0xFFEFFFFFFFFFFFFF
	double f64_highest = std::numeric_limits<double>::max();//0x7FEFFFFFFFFFFFFF
	int i32_lowest = std::numeric_limits<int>::lowest();//0x80000000
	int i32_highest = std::numeric_limits<int>::max();//0x7FFFFFFF
	unsigned char u8_lowest = std::numeric_limits<unsigned char>::lowest();//0x00
	unsigned char u8_highest = std::numeric_limits<unsigned char>::max();//0xFF
	static_assert(sizeof(float) == 4, "float size unexpected");
	static_assert(sizeof(double) == 8, "double size unexpected");
	static_assert(sizeof(int) == 4, "int size unexpected");

	REQUIRE(0xFF_B == static_cast<std::byte>(0xFF));
	REQUIRE(to_bytes(f32_lowest) == std::array<std::byte, 4>{0b1111'1111_B, 0b0111'1111_B, 0b1111'1111_B, 0b1111'1111_B});
	REQUIRE(to_bytes(f32_highest) == std::array<std::byte, 4>{0x7F_B, 0x7F_B, 0xFF_B, 0xFF_B});
	REQUIRE(to_bytes(f64_lowest) == std::array<std::byte, 8>{0xFF_B, 0xEF_B, 0xFF_B, 0xFF_B, 0xFF_B, 0xFF_B, 0xFF_B, 0xFF_B});
	REQUIRE(to_bytes(f64_highest) == std::array<std::byte, 8>{0x7F_B, 0xEF_B, 0xFF_B, 0xFF_B, 0xFF_B, 0xFF_B, 0xFF_B, 0xFF_B});
	REQUIRE(to_bytes(i32_lowest) == std::array<std::byte, 4>{0x80_B, 0x00_B, 0x00_B, 0x00_B});
	REQUIRE(to_bytes(i32_highest) == std::array<std::byte, 4>{0x7F_B, 0xFF_B, 0xFF_B, 0xFF_B});
	REQUIRE(to_bytes(u8_highest) == std::array<std::byte, 1>{0x00_B});
	REQUIRE(to_bytes(u8_highest) == std::array<std::byte, 1>{0xFF_B});
}