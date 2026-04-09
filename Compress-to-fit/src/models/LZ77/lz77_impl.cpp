module lz77;

std::vector<Token> LZ77::compress()
{
	std::vector<Token> out_stream(data.size() / 3);//arbitrary pre-allocation

	//loop over the stream
	while (pattern_matcher.get_pos() <= window.get_absolute_pos())
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
