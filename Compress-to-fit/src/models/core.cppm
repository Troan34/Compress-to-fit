/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*/
module;
#include "mio/mmap.hpp"
#include "../common/sink.hpp"
export module models:core;
import std;
import parser;
import containers;
import util;
namespace fs = std::filesystem;
using namespace std::chrono_literals;

export template <typename T>
concept Model = requires(T const &t, std::span<std::byte const> const data, typename T::block_type& out_blocks, typename T::data_type& out_raw)
                {
                    t.compress(out_blocks);
                    t.decompress(out_raw);
                    T::extract_header(data);
                    requires Serializable<std::remove_cvref_t<typename T::block_type>>;
                    requires Serializable<std::remove_cvref_t<typename T::data_type>>;
                };

constexpr size_t MAX_BLOCK_SIZE = 4_MiB;

export template <Model M, ByteSink S>
class ConcurrentModel
{
public:
    ConcurrentModel(parser::Options const& options, S& sink, File& file)
        : options_{options}, out_data_{sink}, thread_pool{options_.concurrency}, file_{file}
    {
    }

    auto compress() noexcept(false) -> void
    {
        auto const file_size_ = fs::file_size(file_.get_in_file_options().path);
        auto const partition_size = std::min(file_size_ / options_.concurrency, MAX_BLOCK_SIZE);

        for (size_t completed_size = 0, seq_num{}; completed_size < file_size_; completed_size += partition_size, seq_num++)
        {
            //get the true partition size, i.e. the partition can be cut off at the eof
            auto const true_partition_size = [completed_size, partition_size, this]
            {
                if (completed_size + partition_size > file_size_)
                    return file_size_ - completed_size;
                return partition_size;
            }();


            auto future = thread_pool.add_task([this, completed_size, true_partition_size, seq_num]//'this' shall be strictly used to access const members, or concurrency capable containers
            {
                std::vector<Sym> data_for_task;
                data_for_task.resize(true_partition_size);

                file_.read(data_for_task, 0, true_partition_size);

                auto out_buffer = std::make_unique<typename M::template block_type<>>();
                out_buffer->reserve(true_partition_size);

                M compressor{ data_for_task, options_.preset };
                compressor.compress(*out_buffer);
                out_data_.insert(std::move(out_buffer), seq_num);
            });

            std::unique_lock lock{mutex_};
            results.emplace_back(std::move(future));
        }
    }

    /**
     * @brief Decompress the data.
     *
     * @details The decompression is achieved by considering the first index of the data as the start of the first block.
     *			We then check the compressed_size (i.e. `.first`) for this block and advance that much, by doing this we know the start of the new block.
     */
    auto decompress() noexcept(false) -> void
    {
        mio::basic_mmap_source<std::byte> data_{file_.get_in_file_options().path, FILE_HEADER_SIZE};
        std::span const data{reinterpret_cast<Sym const*>(data_.begin()), reinterpret_cast<Sym const*>(data_.end()) };


        for (size_t i{}, seq_n{}; i < data_.size(); seq_n++)
        {
            auto block_info = M::template block_type<>::extract_header(data);

            auto future = thread_pool.add_task(
                [this, data, block_info, i, seq_n]
                {
                    M decompressor{data.subspan(i)};
                    auto buffer = std::make_unique<typename M::data_type>();
                    buffer->reserve(block_info.second);

                    decompressor.decompress(*buffer);

                    out_raw_.insert(std::move(buffer), seq_n);
                });

            i += sizeof(std::invoke_result_t<decltype(&typename M::template block_type<>::uncompressed_length), typename M::template block_type<>>) +
                sizeof(std::invoke_result_t<decltype(&typename M::template block_type<>::compressed_length), typename M::template block_type<>>) +
                block_info.first;

            std::unique_lock lock{mutex_};
            results.emplace_back(std::move(future));
        }
    }

private:
    parser::Options options_;
    std::vector<std::future<void>> results{};
    std::atomic<size_t> n_blocks_{};
    File& file_;
    size_t n_completed_blocks{};
    std::mutex mutex_;//to be used to access results
    std::jthread progress_checker{&ConcurrentModel::check_results, this};
    ConcOrderedFileList<typename M::block_type, S> out_data_;
    ConcOrderedFileList<typename M::data_type, S> out_raw_;
    ThreadPool thread_pool;



    void check_results(std::stop_token stop)
    {
        while (!stop.stop_requested())
        {
            std::this_thread::sleep_for(2ms);
            std::unique_lock lock{mutex_};
            std::erase_if(results,
            [this](std::future<void>& result)
                {
                    if (result.valid())
                    {
                        result.get();
                        n_completed_blocks++;
                        return true;
                    }
                    else
                        return false;
                }
            );
            report({ProgressType{static_cast<float>(n_completed_blocks) / static_cast<float>(n_blocks_)}, {}, true});
        }

        report({ProgressType{1.F}, {}, true});
    }
};
