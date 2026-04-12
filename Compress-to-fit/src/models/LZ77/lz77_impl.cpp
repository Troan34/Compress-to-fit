module lz77;

namespace fs = std::filesystem;


void LZ77::compress(const parser::Options& option, std::vector<Token>& out_stream)
{
	constexpr int buffer_size = 50;

	auto data_size = data.size();
	out_stream.reserve(data_size);
	std::cout << out_stream.data() << std::endl;
	std::array<Token, buffer_size> buffer{};

	const auto modulo_for_progress_bar = (data_size / 50);
	auto progress_counter = 0;
	auto buffering_counter = 0;

	//loop over the stream
	while (pattern_matcher.get_pos() < data_size)
	{
		//if we went past modulo, show progress
		if (progress_counter >= modulo_for_progress_bar)
		{
			show_progress(option, static_cast<double>(pattern_matcher.get_pos()) / static_cast<double>(data_size));
			progress_counter = 0;
		}

		auto token = pattern_matcher.find_pattern();

		auto num_of_rolls = std::max(static_cast<int>(token.length), 1);
		pattern_matcher.roll_hash(num_of_rolls);
		window.slide(num_of_rolls);

		buffer[buffering_counter] = token;

		if (buffering_counter >= buffer_size - 1)
		{
			out_stream.append_range(buffer);
			buffering_counter = 0;
		}
		else
			buffering_counter++;

		progress_counter++;
	}

	show_progress(option, 1.f);
	out_stream.insert(out_stream.end(), buffer.begin(), buffer.begin() + buffering_counter);
	std::cout << '\n';

}

void LZ77::decompress()
{
}
