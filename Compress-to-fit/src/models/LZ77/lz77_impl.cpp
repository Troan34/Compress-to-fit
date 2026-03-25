module lz77;


std::vector<LZ77::Token> LZ77::compress()
{
	auto ahead_span = window.look_ahead_buffer();
	auto search_span = window.search_buffer();
	
	//loop until the ahead buffer has symbols
	while (ahead_span.size())
	{
		for ()
	}
}

void LZ77::decompress()
{
}