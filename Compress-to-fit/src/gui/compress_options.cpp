#include "compress_options.hpp"
import parser;

CompressOptions::CompressOptions()
    : path_out_(parser::DEFAULT_OUT_PATH.c_str())
{

}
void CompressOptions::setPathIn(QUrl const& path)
{
    path_in_ = path;
}
void CompressOptions::setPathOut(QUrl const& path)
{
    path_out_ = path;
}
void CompressOptions::setCompressor(Compressor const comp_type)
{
    comp_type_ = comp_type;
}
void CompressOptions::setCompressorPreset(CompressorPreset const comp_preset)
{
    comp_preset_ = comp_preset;
}
void CompressOptions::setForceCompression(bool const force_compression)
{
    force_compression_ = force_compression;
}
void CompressOptions::setDeleteInput(bool const delete_input)
{
    delete_input_ = delete_input;
}

