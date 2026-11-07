/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*
*   implementation for process_file
*/
module;
#include "mio/mmap.hpp"
#include <cassert>

module models;
import util.serializable;

void process_file(parser::Options const& options)
{
    File file{options};


    if (!file.get_in_file_options().header or options.force_compression)//it's a normal file or -fc option
    {
        switch (options.compressor)
        {
            case static_cast<size_t>(CompType::LZ77):
            {
                byte_sinks::StreamSink sink{ .out = file.get_ref_out_stream() };
                ConcurrentModel<LZ77Model<>, decltype(sink)> comp{options, sink, file};
                comp.compress();
                break;
            }
            default:
                auto const location = std::source_location::current();
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
                byte_sinks::StreamSink sink{ .out = file.get_ref_out_stream() };
                ConcurrentModel<LZ77Model<>, decltype(sink)> decomp{options, sink, file};
                decomp.decompress();
                break;
            }
            default:
                auto const location = std::source_location::current();
                auto error_string = std::string{"Unreachable branch, file: "} + location.file_name() + std::string{"\nline: "} + std::to_string(location.line());
                assert(false && error_string.c_str());
                break;
        }
    }
}
