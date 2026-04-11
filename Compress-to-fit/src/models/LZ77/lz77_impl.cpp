module lz77;

namespace fs = std::filesystem;


std::vector<Token> LZ77::compress(const fs::path& file, const parser::Options& option)
{
	std::vector<Token> out_stream{};//arbitrary pre-allocation
	auto data_size = data.size();
	out_stream.reserve(data_size / 3);

	const auto modulo_for_progress_bar = (data_size / 50);
	auto counter = 0;

	//loop over the stream
	while (pattern_matcher.get_pos() < data_size)
	{
		if (counter >= modulo_for_progress_bar)
		{
			show_progress(file, option, static_cast<double>(pattern_matcher.get_pos()) / static_cast<double>(data_size));
			counter = 0;
		}

		auto token = pattern_matcher.find_pattern();
		out_stream.push_back(token);
		auto num_of_rolls = std::max(static_cast<int>(token.length), 1);
		pattern_matcher.roll_hash(num_of_rolls);
		window.slide(num_of_rolls);

		counter++;
	}

	std::cout << '\n';
	return out_stream;
}

void LZ77::decompress()
{
}
