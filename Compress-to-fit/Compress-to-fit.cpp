import parser;
import lz77;

using namespace std;

int main(int argc, char* argv[])
{
	auto options = parser::parse(argc, argv);

	std::ifstream file{ options.filename_in };
	std::vector<Sym> stream{ (std::istream_iterator<Sym>{file}), std::istream_iterator<Sym>{} };
	std::span<Sym> data{ stream };
	LZ77 encoder{ data, static_cast<CompPreset>(options.preset)};
	auto output = encoder.compress();
	std::ofstream outFile{options.filename_out};
	for (auto& e : output) outFile << e;

	return 0;
}
