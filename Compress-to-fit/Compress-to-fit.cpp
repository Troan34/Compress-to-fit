import parser;
import lz77;
import file_util;

using namespace std;

int main(int argc, char* argv[])
{

	auto options = parser::parse(argc, argv);
	
	std::vector<Sym> stream{};
	File file{};
	file.read_file(options.filename_in, stream);

	std::span<Sym> data{ stream };

	LZ77 encoder{ data, static_cast<CompPreset>(options.preset)};

	std::vector<LZ77::Token> output_vec{};
	encoder.compress(options, output_vec);

	std::cout << output_vec.data();
	std::span<const LZ77::Token> output{ output_vec };
	
	file.write_file(output, options.filename_out);
	file.split_file(options.filename_out, options.n_files);

	return 0;
}
