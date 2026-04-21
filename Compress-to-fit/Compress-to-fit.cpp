#include <mio/mmap.hpp>

import parser;
import lz77;
import file_util;


using namespace std;

int main(int argc, char* argv[])
{

	auto options = parser::parse(argc, argv);
	
	std::vector<Sym> stream{};
	File file{options.filename_in, options};
	//file.read_file(options.filename_in, stream);
	mio::mmap_source IO_map{ options.filename_in };

	std::span<Sym> data{ IO_map.begin(), IO_map.end() };

	LZ77 encoder{data, options};

	file.process_file(
		[&encoder](size_t max_index) -> std::vector<LZ77::Token>
		{
			return encoder.compress(max_index);
		}
	);
	//std::vector<LZ77::Token> output_vec{ encoder.compress() };
	
	
	//std::span<const LZ77::Token> output{ output_vec };
	
	//file.write_file(output, options.filename_out);
	//file.split_file(options.n_files);

	return 0;
}
