/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*
*   Module interface for the models
*/
module;
#include "mio/mmap.hpp"
#include "../common/sink.hpp"
#include <cassert>
export module models;
export import :lz77;
import util.serializable;

export void process_file(parser::Options const& options,  StatusSink& status_sink)
{
    File file{options};

    if (!file.get_in_file_options().header or options.force_compression)//it's a normal file or -fc option
    {
        switch (options.compressor)
        {
            case static_cast<size_t>(CompType::LZ77):
            {
                byte_sinks::StreamSink sink{ .out = file.get_ref_out_stream() };
                ConcurrentModel<LZ77Model<>, decltype(sink)> comp{options, sink, status_sink, file};
                comp.compress();
                break;
            }
            default:
                auto error_string = std::string{"Unreachable branch, "} + format_sourceloc();
                report({ErrorType::LOGIC_ERROR, error_string, {}});
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
                ConcurrentModel<LZ77Model<>, decltype(sink)> decomp{options, sink, status_sink, file};
                decomp.decompress();
                break;
            }
            default:
                auto const location = std::source_location::current();
                auto error_string = std::string{"Unreachable branch, "} + format_sourceloc();
                report({ErrorType::LOGIC_ERROR, error_string, {}});
                break;
        }
    }
}