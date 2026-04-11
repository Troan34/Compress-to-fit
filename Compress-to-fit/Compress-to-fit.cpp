import parser;
import lz77;
import file_util;
#include <Windows.h>

using namespace std;

int main(int argc, char* argv[])
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
#endif

	auto options = parser::parse(argc, argv);
	
	std::vector<Sym> stream{};
	file::read_file(options.filename_in, stream);

	std::span<Sym> data{ stream };

	LZ77 encoder{ data, static_cast<CompPreset>(options.preset)};

	auto output_vec = encoder.compress();
	std::span<const LZ77::Token> output{ output_vec };
	
	file::write_file(output, options.filename_out);
	file::split_file(options.filename_out, options.n_files);

	return 0;
}
