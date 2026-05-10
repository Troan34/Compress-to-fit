#include <mio/mmap.hpp>

import parser;
import lz77;
import file_util;


using namespace std;


//TODO: Use a circular array-like to save the elaborated data to disk, MAKE IT ASYNC.

int main(int argc, char* argv[])
{
	auto options = parser::parse(argc, argv);
	
	File file{options.filename_in, options};

	std::error_code error;


	std::ifstream in{ options.filename_in };

	if (!File::has_signature(in))
	{
		auto IO_map = mio::make_mmap_source(options.filename_in.string(), error);

		std::span<Sym const> data{
			reinterpret_cast<Sym const*>(IO_map.data()),
			IO_map.size()
		};

		LZ77 encoder{ data, options };

		file.process_file<Sym, LZ77_Token>(
			[&](CodecInterface<Sym, LZ77_Token>& interface)
			{
				encoder.compress(interface);
			}
		);
	}
	else
	{
		auto IO_map = mio::make_mmap_source(options.filename_in.string(), FILE_HEADER_SIZE, mio::map_entire_file, error);

		std::span<LZ77_Token const> data{
			reinterpret_cast<LZ77_Token const*>(IO_map.data()),
			IO_map.size()
		};
		LZ77 decoder{ data, options };
		file.process_file<LZ77_Token, Sym>(
			[&](CodecInterface<LZ77_Token, Sym>& interface)
			{
				decoder.decompress(interface);
			}
		);
	}


	return 0;
}
