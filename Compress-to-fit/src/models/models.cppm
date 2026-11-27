/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*
*   Module interface for the models
*/
module;
#include "mio/mmap.hpp"
#include "../common/sink.hpp"
export module models;
export import :lz77;
import util.serializable;

/**
 * @brief The actual "main" of the program
 * @param options passed in options
 * @param status_sink sink for logging, which too is passed
 */
export void process_file(parser::Options const& options, StatusSink& status_sink)
{
    File file{options};
    byte_sinks::StreamSink sink{ .out = file.get_ref_out_stream() };

    if (!file.get_in_file_options().header or options.force_compression)//it's a normal file or -fc option
    {
        switch (options.compressor)
        {
            case std::to_underlying(CompType::LZ77):
            {
                ConcurrentModel<LZ77Model<>, decltype(sink)> comp{options, sink, status_sink, file};
                comp.compress();
                break;
            }
            default:
                auto error_string = std::string{"Unreachable branch, "} + format_sourceloc();
                report({.message_ID_or_progress=ErrorType::LOGIC_ERROR, .failing_option=error_string, .compressing={}});
                break;
        }
    }
    else//it's a compressed file
    {
        switch (file.get_in_file_options().header.value().comp_type)
        {
            case CompType::LZ77:
            {
                ConcurrentModel<LZ77Model<>, decltype(sink)> decomp{options, sink, status_sink, file};
                decomp.decompress();
                break;
            }
            default:
                auto error_string = std::string{"Unreachable branch, "} + format_sourceloc();
                report({.message_ID_or_progress=ErrorType::LOGIC_ERROR, .failing_option=error_string, .compressing={}});
                break;
        }
    }
}