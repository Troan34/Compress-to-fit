#include <mio/mmap.hpp>

import parser;
import lz77;
import file_util;


using namespace std;

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
		file.process_file<LZ77_Token>(
			[&encoder](std::vector<LZ77_Token>& output, size_t max_index) -> void
			{
				return encoder.compress(output, max_index);
			}
		);
	}
	else
	{
		auto IO_map = mio::make_mmap_source(options.filename_in.string(), 5, 0, error);

		std::span<LZ77_Token const> data{
			reinterpret_cast<LZ77_Token const*>(IO_map.data()),
			IO_map.size()
		};
		LZ77 decoder{ data, options };
		file.process_file<Sym>(
			[&decoder](std::vector<Sym>& output, size_t max_index) -> void
			{
				return decoder.decompress(output, max_index);
			}
		);
	}


	return 0;
}
