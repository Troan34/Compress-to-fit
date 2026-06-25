module models;

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



LZ77Block::LZ77Block(std::span<std::byte const> const data_, fs::path const& file) noexcept
{
	constexpr size_t len1 = sizeof(decltype(compressed_length_));
	constexpr size_t len2 = sizeof(decltype(uncompressed_length_));

	std::memcpy(&compressed_length_, data_.data(), len1);//get compressed_length from data
	std::memcpy(&uncompressed_length_, data_.data() + len1, len2);//get uncompressed_length from data

	if (len1 + len2 + compressed_length_ > data_.size())
		throw_error(ErrorType::FILE_CORRUPTED, "An lz77 header is corrupted in file \"" + file.string() + "\"");

	block.reserve(data_.size());
	block = std::vector<LZ77_Token>{
		reinterpret_cast<LZ77_Token const*>(data_.data() + len1 + len2),
		reinterpret_cast<LZ77_Token const*>(data_.data() + compressed_length_)
	};
}

void LZ77Block::serialize(std::vector<std::byte> &output)
{
	constexpr size_t len1 = sizeof(decltype(compressed_length_));
	constexpr size_t len2 = sizeof(decltype(uncompressed_length_));

	output.resize(len1 + len2 + block.size() * sizeof(LZ77_Token));

	std::memcpy(output.data(), &compressed_length_, len1);
	std::memcpy(output.data() + len1, &uncompressed_length_, len2);

	std::memcpy(output.data() + len1 + len2, block.data(), block.size() * sizeof(LZ77_Token));
}

void LZ77Block::write_to(std::ofstream& output)
{
	std::vector<std::byte> block;
	this->serialize(block);
	output.write(reinterpret_cast<char const*>(block.data()), block.size());
}


void LZ77Compressor::compress(LZ77Block& out_data)
{
	out_data.clear();
	out_data.reserve(data_.size() / sizeof(LZ77_Token));

	size_t consumed_tokens{};
	//loop over the stream
	for (auto i = data_.begin(); i != data_.end();)
	{
		auto const token = this->pattern_matcher.find_pattern();

		auto const num_of_rolls = token.length + 1;
		this->pattern_matcher.roll_hash(num_of_rolls);//roll, updates the hash and position
		this->window.slide(num_of_rolls);
		i += num_of_rolls;
		consumed_tokens += num_of_rolls;

		out_data.push_back(token);
	}
}



