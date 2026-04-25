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

			prev_poss_table[prev_bucket_index(position)] = poss_table[bucket_index(hash)];
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

	//can we keep going down the chain -and- check if possible offset close enough to pos -and- candidate exists
	while (num_of_iter <= MAX_CHAIN and position - candidate <= data.get_max_size_search() and candidate > -1)
	{
		auto temp_len = count_equal(data.get_data().subspan(position), data.get_data().subspan(candidate), std::numeric_limits<uint8_t>::max());
		if (temp_len > best_length)
		{
			best_length = temp_len;
			best_offset = position - candidate;
		}
		candidate = prev_poss_table[prev_bucket_index(candidate)];
		num_of_iter++;
	}

	if (position + best_length < data.get_data().size())
		return Token{ .offset = best_offset, .length = best_length, .symbol = data[position + best_length] };
	else
		return Token{ .offset = best_offset, .length = --best_length, .symbol = data[position] };
}





