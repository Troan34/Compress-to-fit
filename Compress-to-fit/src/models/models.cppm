/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*
*   Module interface for the models
*/
export module models;
export import :lz77;
import parser;

export void process_file(parser::Options const& options);