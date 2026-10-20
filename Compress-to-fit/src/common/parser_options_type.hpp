/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*   Forward the Options type
*/
#pragma once

#include <optional>
#include <filesystem>
#include <thread>
#include "comp_options.hpp"

namespace fs = std::filesystem;

namespace parser
{
    inline const fs::path DEFAULT_OUT_PATH{ "output.tzf" };

    /**
     * @brief All the options available
     */
    struct Options
    {
        fs::path filename_in{};
        fs::path filename_out{ DEFAULT_OUT_PATH };
        size_t compressor = static_cast<size_t>(CompType::LZ77);
        size_t preset = COMP_5;
        size_t n_files = 1;
        size_t size_files = 0;
        bool force_compression = false;
        bool delete_input = false;
        bool concatenate_files = false;
        bool need_help = false;
        std::optional<bool> decomp_after_concat;
        size_t concurrency = std::thread::hardware_concurrency();
    };
}