import std.mem;
import std.core;
import std.io;

enum HEADER_OFFSET : u32
{
    SIGNATURE = 0,
    ID = 5,
    COUNT = ID +                sizeof(u64),
    COMP_TYPE = COUNT +         sizeof(u16),
    COMP_PRESET = COMP_TYPE +   sizeof(u8),
    MAX = COMP_PRESET +         sizeof(u8),
};

enum CompType : u8
{
    LZ77,
    LZ78,
    MAX,
};

enum CompPreset : u8
{
    NO_COMP,
    COMP_1,
    COMP_2,
    COMP_3,
    COMP_4,
    COMP_5,
    COMP_6,
    COMP_7,
    COMP_8,
    COMP_MAX,
};

struct CTFHeader
{
    char signature[5];
    u64 ID;
    u16 count;
    CompType comp_type;
    CompPreset comp_preset;
};

#pragma array_limit 0x1000000
#pragma pattern_limit 0x2000000

using Sym = u8;
struct LZ77Token
{
    u16 offset  [[color("FF5555")]];
    u8 len      [[color("55FF55")]];     
    Sym symbol  [[color("5555FF")]];
};
struct LZ77Block
{
    u32 compressed_len;
    u32 uncompressed_len;
    LZ77Token buffer[compressed_len / sizeof(LZ77Token)];
};

struct CTFfile
{
    CTFHeader header;
    match (header.comp_type)
    {
        (CompType::LZ77): LZ77Block blocks[while(!std::mem::eof() && $ + 8 <= std::mem::size())];;
        (_): std::error("Unsupported compression type");
    }
};

struct Pair<T>
{
    T first;
    T second;
};
/*
#pragma loop_limit 0x1000000
fn index(ref CTFfile file, u64 addr)
{
    bool flag = false;
    for (u64 block_count = 0, block_count < std::core::member_count(file.blocks), block_count += 1)
    {
        if (file.blocks[block_count].uncompressed_len < addr)
        {
            addr -= file.blocks[block_count].uncompressed_len;
            continue;
        }
        for (u64 token_count = 0, token_count < std::core::member_count(file.blocks[block_count].buffer), token_count += 1)
        {
            true_addr += file.blocks[block_count].buffer[token_count].len + 1;

            if (addr - file.blocks[block_count].buffer[token_count].len - 1 < 0)
            {
                Pair<u32> p;
                p.first = block_count;
                p.second = token_count;
                return p;
            }
        }
        if (flag) { break; }
    }
    return true_addr;
};



u64 original_address in;
u64 respective_address out;

respective_address = index(file, original_address);
*/
CTFfile file @ 0x0;