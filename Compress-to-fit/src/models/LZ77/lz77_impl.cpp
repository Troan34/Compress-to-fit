module lz77;

std::vector<Token> LZ77::compress()
{
	std::vector<Token> out_stream{};//arbitrary pre-allocation
	out_stream.reserve(data.size() / 3);

	//loop over the stream
	while (pattern_matcher.get_pos() < data.size())
	{
		auto token = pattern_matcher.find_pattern();
		out_stream.push_back(token);
		auto num_of_rolls = std::max(static_cast<int>(token.length), 1);
		pattern_matcher.roll_hash(num_of_rolls);
		window.slide(num_of_rolls);
	}

	return out_stream;
}

void LZ77::decompress()
{
}
