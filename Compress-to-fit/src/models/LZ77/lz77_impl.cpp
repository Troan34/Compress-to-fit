module lz77;

namespace fs = std::filesystem;

void alg::Rabin::roll_hash(uint32_t num_of_rolls) noexcept
{
	while (num_of_rolls > 0)
	{
		if (position < (data.get_data().size() - MIN_MATCH))//stop if we reached the end
		{
			//roll hash
			hash = (hash + MOD - static_cast<uint32_t>(data[position]) * ROLL_FACTOR % MOD) % MOD;
			hash = (hash * BASE + static_cast<uint32_t>(data[position + MIN_MATCH])) % MOD;

			prev_poss_table[prev_bucket_index(hash)] = poss_table[bucket_index(hash)];
			poss_table[bucket_index(hash)] = position;
		}

		position++;
		num_of_rolls--;
	}
}

Token alg::Rabin::find_pattern()
{
	uint8_t best_length = 0;
	uint16_t best_offset = 0;
	auto candidate = poss_table[bucket_index(hash)];
	int num_of_iter = 0;

	//check the number of times we go down the chain --- check if candidate is not too far from pos --- and that candidate exist (we initialized -1)
	while (num_of_iter <= MAX_CHAIN and position - candidate < data.get_size_search() and candidate >= 0)
	{
		auto temp_len = count_equal(data.get_data().subspan(position), data.get_data().subspan(candidate), std::numeric_limits<uint8_t>::max());
		if (temp_len > best_length)
		{
			best_length = temp_len;
			best_offset = candidate;
		}
		candidate = prev_poss_table[prev_bucket_index(candidate)];
		num_of_iter++;
	}

	if (position + best_length < data.get_data().size())
		return Token{ .offset = best_offset, .length = best_length, .symbol = data[position + best_length] };
	else
		return Token{ .offset = best_offset, .length = --best_length, .symbol = data[position] };
}

std::vector<Token> LZ77::compress()
{
	constexpr int buffer_size = 50;

	std::vector<Token> out_stream{};

	auto data_size = data.size();
	out_stream.reserve(data_size);

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
			show_progress(cli_options, static_cast<double>(pattern_matcher.get_pos()) / static_cast<double>(data_size), true);
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
	show_progress(cli_options, 1.f, true);
	out_stream.insert(out_stream.end(), buffer.begin(), buffer.begin() + buffering_counter);
	std::cout << '\n';

	return out_stream;

}

std::vector<Token> LZ77::compress(size_t max_index)
{
	constexpr int buffer_size = 50;

	std::vector<Token> out_stream{};

	auto data_size = max_index - pattern_matcher.get_pos();
	out_stream.reserve(data_size);

	std::array<Token, buffer_size> buffer{};

	auto buffering_counter = 0;

	//loop over the stream
	while (pattern_matcher.get_pos() < max_index and pattern_matcher.get_pos() < data.size())
	{

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

	}
	out_stream.insert(out_stream.end(), buffer.begin(), buffer.begin() + buffering_counter);

	return out_stream;
}

void LZ77::decompress()
{
}
