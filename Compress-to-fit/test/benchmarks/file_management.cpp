#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <fstream>
#include <filesystem>

void create_stream(size_t num)
{
    std::ofstream a_path{ "a_path.txt" };

    for (size_t i = 0; i < num; i++)
    {
        volatile std::ifstream var{ "a_path.txt" };
    }

    std::filesystem::remove("a_path.txt");
}

TEST_CASE("stream creation ")
{
    BENCHMARK("create_stream 500")
    {
        create_stream(500);
    };

    BENCHMARK("create_stream 2000")
    {
        create_stream(2000);
    };

}