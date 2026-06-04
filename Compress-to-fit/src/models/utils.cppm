export module codec_util;
import std.compat;
import util;
import parser;

export template <typename InType, typename OutType>
struct CodecInterface
{
    ForwardIterator<InType const*> in_data;
    std::vector<OutType>& out_data;
};

export class Compressor
{
public:
    virtual ~Compressor() = default;
    virtual size_t compress(std::span<const Sym> in_bytes, std::vector<std::byte>& out_bytes, size_t max_size_chunk) = 0;
};


export class Decompressor
{
public:
    virtual ~Decompressor() = default;
    virtual size_t decompress(std::span<const std::byte> in_bytes, std::vector<Sym>& out_bytes, size_t max_size_chunk) = 0;
};
