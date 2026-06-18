module;
#include "mio/mmap.hpp"
#include <cassert>

export module models;
export import :lz77;

export void process_file(parser::Options const& options)
{
    File file{options};
    mio::basic_mmap_source<std::byte> data_map{file.get_in_file_options().path.string()};
    std::span data{data_map};

    if (!file.get_in_file_options().header)//it's a normal file
    {
        switch (options.compressor)
        {
            case static_cast<size_t>(CompType::LZ77):
            {
                LZ77ConcurrentCompressor comp{data, options.concurrency, file, static_cast<CompPreset>(options.preset)};
                comp.compress();
                break;
            }
            default:
                auto location = std::source_location::current();
                auto error_string = std::string{"Unreachable branch, file: "} + location.file_name() + std::string{"\nline: "} + std::to_string(location.line());
                assert(false && error_string.c_str());
                break;
        }
    }
    else//it's a compressed file
    {
        switch (file.get_in_file_options().header.value().comp_type)
        {
            case CompType::LZ77:
            {
                LZ77ConcurrentDecompressor decomp{data, file, options.concurrency};
                decomp.decompress();
                break;
            }
            default:
                auto location = std::source_location::current();
                auto error_string = std::string{"Unreachable branch, file: "} + location.file_name() + std::string{"\nline: "} + std::to_string(location.line());
                assert(false && error_string.c_str());
                break;
        }

    }
}