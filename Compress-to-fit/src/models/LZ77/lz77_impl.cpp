module lz77;

namespace fs = std::filesystem;

void alg::Rabin::roll_hash(uint32_t num_of_rolls) noexcept
{
	while (num_of_rolls > 0)
	{
		if (position < (data.get_data().size() - MIN_MATCH))//stop if we reached the end
		{
			//roll hash
			hash = (hash + MOD - data[position] * ROLL_FACTOR % MOD) % MOD;
			hash = (static_cast<uint64_t>(hash) * BASE + data[position + MIN_MATCH]) % MOD;

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

	//can we keep going down the chain -and- check if possible offset close enough to pos -and- candidate exists -and- that candidate is not this position (offset == 0)
	while (num_of_iter <= MAX_CHAIN and position - candidate <= data.get_max_size_search() and candidate > -1 and candidate != position)
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
		return Token{ .offset = best_offset, .length = --best_length, .symbol = data[position - 1] };
}



LZ77Block::LZ77Block(std::span<std::byte const> const data_) noexcept
{
	constexpr size_t len1 = sizeof(decltype(compressed_length));
	constexpr size_t len2 = sizeof(decltype(uncompressed_length));

	std::memcpy(&compressed_length, data_.data(), len1);//get compressed_length from data
	std::memcpy(&uncompressed_length, data_.data() + len1, len2);//get uncompressed_length from data

	block.reserve(data_.size());
	block = std::vector{
		reinterpret_cast<LZ77_Token const*>(data_.begin() + len1 + len2),
		reinterpret_cast<LZ77_Token const*>(data_.end())
	};
}

void LZ77Block::serialize(std::vector<std::byte> &output)
{
	constexpr size_t len1 = sizeof(decltype(compressed_length));
	constexpr size_t len2 = sizeof(decltype(uncompressed_length));

	output.resize(len1 + len2 + block.size_bytes());

	std::memcpy(output.data(), &compressed_length, len1);
	std::memcpy(output.data() + len1, &uncompressed_length, len2);

	std::memcpy(output.data() + len1 + len2, block.data(), block.size_bytes());
}



size_t LZ77Compressor::compress(std::span<const Sym> in_data, std::vector<std::byte const>& out_data, size_t const max_size_chunk)
{
	out_data.reserve(SIZE_CHUNK);

	size_t consumed_tokens{};
	//loop over the stream
	for (auto i = in_data.begin(); i != in_data.end() and consumed_tokens < max_size_chunk; ++i)
	{
		auto token = this->pattern_matcher.find_pattern();

		auto const num_of_rolls = static_cast<int>(token.length + 1);
		this->pattern_matcher.roll_hash(num_of_rolls);//roll, updates the hash and position
		this->window.slide(num_of_rolls);
		i += num_of_rolls;
		consumed_tokens += num_of_rolls;

		Token::SerializedBuffer buffer{};
		token.serialize(buffer);
		out_data.append_range(buffer);
	}

	return consumed_tokens;
}

size_t LZ77Decompressor::decompress(std::span<std::byte const> in_data, std::vector<Sym>& out_data, size_t const max_size_chunk)
{
	out_data.reserve(std::min(in_data.size(), max_size_chunk));
	while (!in_data.reached_end())
	{
		if (in_data.iterator->offset != 0)
		{
			for (size_t remaining_syms = in_data.iterator->length; remaining_syms > 0; remaining_syms--)
				out_data.push_back(out_data[out_data.size() - in_data.iterator->offset]);
		}
		out_data.emplace_back(in_data.iterator->symbol);
		++in_data;
	}
}

