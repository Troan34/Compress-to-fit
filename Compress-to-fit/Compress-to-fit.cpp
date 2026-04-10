import parser;
import lz77;
import file_util;

using namespace std;

int main(int argc, char* argv[])
{
	auto options = parser::parse(argc, argv);

	std::ifstream file{ options.filename_in, std::ios::binary };
	std::vector<Sym> stream{ (std::istream_iterator<Sym>{file}), std::istream_iterator<Sym>{} };
	std::span<Sym> data{ stream };

	LZ77 encoder{ data, static_cast<CompPreset>(options.preset)};

	auto output_vec = encoder.compress();
	std::span<const LZ77::Token> output{ output_vec };
	
	file::write_file(output, options.filename_out);

	return 0;
}
