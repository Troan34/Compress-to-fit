/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*/
module;
#include "mio/mmap.hpp"
export module models:core;
import std;
import parser;
import containers;
import util;

export template <typename T>
concept Model = requires(T const &t, std::span<std::byte const> const data, typename T::block_type out_blocks, typename T::data_type out_raw)
                {
                    t.compress(out_blocks);
                    t.decompress(out_raw);
                    T::extract_header(data);
                    requires Serializable<std::remove_cvref_t<typename T::block_type>>;
                    requires Serializable<std::remove_cvref_t<typename T::data_type>>;
                };


export template <Model M, ByteSink S>//huh??
class ConcurrentModel
{
public:
    ConcurrentModel(parser::Options const& options, S& sink)
        : options_{options}, sink_(sink), file_list_{sink_}, thread_pool{options_.concurrency}
    {
    }

private:
    parser::Options options_;
    S& sink_;
    std::vector<std::future<void>> results{};
    std::atomic<size_t> n_blocks_{};
    size_t n_completed_blocks{};
    std::mutex mutex_;//to be used to access results
    std::jthread progress_checker{&ConcurrentModel::check_results, this};
    //These have to stay down here (I just found out how objects are destroyed)
    ConcOrderedFileList<T, S> file_list_;
    ThreadPool thread_pool;


    void check_results(std::stop_token stop);
};
