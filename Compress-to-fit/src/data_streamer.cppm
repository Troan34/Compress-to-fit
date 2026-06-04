module;
#include <mio/mmap.hpp>
export module stream;
import std.compat;
import util;
import file_util;

//codecs
import lz77;


namespace fs = std::filesystem;

/*
 * @brief Stream data from a file to a certain compressor
 */
export template<typename In, typename Out>
class DataStreamer
{
public:
    DataStreamer(fs::path const& path, CompType const comp_type, CompPreset const preset, FileOptions const& out_file_options)
        : path_{path}, out_file_options_{out_file_options}, comp_type_{comp_type}, preset_{preset}
    {
        std::error_code err;
        data_ = mio::make_mmap_source(path_.string(), err);
        if (err)
        {
            std::println("\033[41mError\033[31m[{}]: {}."
                         "\n\033[93mNOTE: this is an error caused by the creation of a memory mapped file, search the error code on the internet.\033[0m (vimpunk/mio library)",
                         err.value(), err.message());

            throw std::runtime_error{err.message()};
        }

    }


private:
    mio::mmap_source data_;
    fs::path const& path_;
    FileOptions const& out_file_options_;
    CompType comp_type_;
    CompPreset preset_;


    void process_data()
    {

    }


};