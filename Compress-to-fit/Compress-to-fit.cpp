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
	auto IO_map = mio::make_mmap_source(options.filename_in.string(), error);

	std::span<Sym const> data{
		reinterpret_cast<Sym const*>(IO_map.data()),
		IO_map.size()
	};

	LZ77 encoder{data, options};

	file.process_file(
		[&encoder](size_t max_index) -> std::vector<LZ77::Token>
		{
			return encoder.compress(max_index);
		}
	);

	return 0;
}
