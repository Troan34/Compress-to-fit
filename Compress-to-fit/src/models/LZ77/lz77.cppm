/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*
*   Implement the LZ77 compression model
*/

module;
#include <cassert>
#include <cstddef>
#include <mio/mmap.hpp>
#include "../../common/sink.hpp"
export module models:lz77;

#ifdef __INTELLISENSE__
#include "../../../for_intellisense/everything.hpp"
#endif

export import util;
import std.compat;
import parser;
import containers;
import :core;


namespace fs = std::filesystem;
using namespace std::chrono_literals;


#pragma pack(push, 1)
/**
* @brief This is the type LZ77 will output
*/
struct Token
{
	uint16_t offset{};
	uint8_t length{};
	Sym symbol{};

	using SerializedBuffer = std::array<std::byte, sizeof(decltype(offset)) + sizeof(decltype(length)) + sizeof(decltype(symbol))>;


	friend std::ostream& operator<<(std::ostream& os, const Token& token)
	{
		os.write(reinterpret_cast<const char*>(&token), sizeof(Token));

		return os;
	}

	void serialize(SerializedBuffer& buffer) noexcept
	{
		std::memcpy(buffer.data(), &offset, sizeof(decltype(offset)));
		std::memcpy(buffer.data() + sizeof(decltype(offset)), &length, sizeof(decltype(length)));
		std::memcpy(buffer.data() + sizeof(decltype(offset)) + sizeof(decltype(length)), &symbol, sizeof(Sym));
	}

};
#pragma pack(pop)

export using LZ77_Token = Token;

inline constexpr int SEARCH_RATIO = 500;//Ratio of the search buf size and la buf
inline constexpr auto MAX_WINDOW_SIZE = KiB_to_B(128);
/**
 * @brief The size of @ref alg::Rabin::poss_table
 */
inline constexpr auto MAX_HASH_SIZE = MAX_WINDOW_SIZE << 1;

/**
 * @brief [lz77.Window]
 * @brief This class gives a window to data given to the ctor
 */
class Window
{
public:
	Window(const std::span<Sym const>& data_, size_t const window_size) noexcept
		: data(data_), size_search(0), max_size(std::clamp(window_size, window_size, MAX_WINDOW_SIZE))
	{
		if (max_size > data.size()) max_size = data.size();
		size_look_ahead = std::floor(max_size / (SEARCH_RATIO + 1));
		max_size_look_ahead = size_look_ahead;
		max_size_search = SEARCH_RATIO * max_size_look_ahead;//TODO: BUG: MAX_SIZE_SEARCH GOES OUTSIDE OF RANGE
	}

	Window(const std::span<Sym const>& data_, CompPreset const preset)
		: data(data_)
	{
		size_search = 0;
		switch (preset)
		{
		case COMP_MAX:
			max_size = MAX_WINDOW_SIZE;
			break;
		case COMP_8:
			max_size = MAX_WINDOW_SIZE >> 1;
			break;
		case COMP_7:
			max_size = MAX_WINDOW_SIZE >> 2;
			break;
		case COMP_6:
			max_size = MAX_WINDOW_SIZE >> 3;
			break;
		case COMP_5:
			max_size = MAX_WINDOW_SIZE >> 4;
			break;
		case COMP_4:
			max_size = MAX_WINDOW_SIZE >> 5;
			break;
		case COMP_3:
			max_size = MAX_WINDOW_SIZE >> 6;
			break;
		case COMP_2:
			max_size = MAX_WINDOW_SIZE >> 7;
			break;
		case COMP_1:
			max_size = MAX_WINDOW_SIZE >> 8;
			break;
		case NO_COMP:
			throw std::runtime_error("There has been an exception in the file " + std::string{ std::source_location::current().file_name() } + " at the line " + std::string{ std::source_location::current().line() });
			break;
		default:
			throw std::runtime_error("There has been an exception in the file " + std::string{ std::source_location::current().file_name() } + " at the line " + std::string{ std::source_location::current().line() });
			break;
		}
		if (max_size > data.size()) max_size = data.size();
		size_look_ahead = std::floor(max_size / (SEARCH_RATIO + 1));
		max_size_look_ahead = size_look_ahead;
		max_size_search = SEARCH_RATIO * max_size_look_ahead;//TODO: BUG: MAX_SIZE_SEARCH GOES OUTSIDE OF RANGE
	}


	auto begin() const noexcept
	{
		return data.begin() + offset;
	}

	auto end() const noexcept
	{
		return data.begin() + offset + size_search + size_look_ahead + 1;
	}

	/**
	 * @brief Slide the window
	 * @param displacement The offset(or sizes) will moved by this amount
	 */
	void slide(size_t const displacement)
	{
		if (size_search < max_size_search)//size_search should grow
		{
			size_search += displacement;
			if (size_search > max_size_search)//size_search has grown past maximum
			{
				offset += size_search - max_size_search;
				size_search = max_size_search;
			}
		}
		else
		{
			offset += displacement;
		}

		if (offset + max_size > data.size())//we have to cut off size_look_ahead
		{
			size_look_ahead = std::clamp(size_look_ahead - (offset + max_size - data.size()), 0ul, max_size_look_ahead);
		}

		offset = std::clamp(offset, 0ul, data.size() - max_size_search);
	}

	[[nodiscard]] auto look_ahead_buffer() const noexcept -> std::span<Sym const>{return data.subspan(offset + size_search, size_look_ahead);}
	[[nodiscard]] auto search_buffer() const noexcept -> std::span<Sym const>{return data.subspan(offset, size_search);}
	[[nodiscard]] auto get_size_search() const noexcept{return size_search;}
	[[nodiscard]] auto get_size_la_buf() const noexcept{return size_look_ahead;}
	[[nodiscard]] auto get_max_size() const noexcept{return max_size;}
	[[nodiscard]] auto get_max_size_search() const noexcept{return max_size_search;}
	[[nodiscard]] auto get_relative_pos() const noexcept{return size_search;}
	[[nodiscard]] auto get_absolute_pos() const noexcept{return size_search + offset;}
	[[nodiscard]] auto get_offset() const noexcept{return offset;}
	[[nodiscard]] auto operator[](size_t const index) const noexcept
	{
		assert(index < data.size());
		return data[index];
	}

	[[nodiscard]] auto get_data() const noexcept{return data;}

private:
	const std::span<const Sym> data;
	size_t size_look_ahead;
	size_t max_size_look_ahead;
	size_t size_search;
	size_t max_size_search;
	size_t max_size;
	size_t offset = 0;
};

namespace alg
{
	inline constexpr uint32_t MOD = (1u << 31) - 1;
	inline constexpr uint32_t BASE = Sym::alphabet_size();
	inline constexpr uint32_t MIN_MATCH = 3;
	inline constexpr uint32_t MAX_MATCH = 250;
	inline constexpr uint32_t PRIME = 31;
	inline constexpr uint32_t ROLL_FACTOR = const_pow(BASE, MIN_MATCH - 1);


	/**
	 * @brief This class enables fast string matching.
	 * 		  Makes use of two chained arrays.
	 * 		  This class is intertwined with LZ77 and should not be used as separate entity.
	 * 		  This class shall not modify the Window given
	 */
	class Rabin
	{
	public:

		/**
		 * @pre @p data.max_size() > #MAX_HASH_SIZE
		 */
		Rabin(const Window& data, uint32_t pos, CompPreset preset)
			:data(data), position(pos), MAX_CHAIN(get_max_chain_from_preset(preset))
		{
			//compiler will probably use a memset-like optimization
			for (int i = 0; i < MAX_HASH_SIZE; i++)
				poss_table[i] = -1;

			for (int i = 0; i < MAX_WINDOW_SIZE; i++)
				prev_poss_table[i] = -1;
			
			hash = data[0];
			for (int i = 1; i < MIN_MATCH and i < data.get_data().size(); i++)//initialize the hash
			{
				hash = (static_cast<uint64_t>(hash) * BASE + data[position + i]) % MOD;
			}
			poss_table[bucket_index(hash)] = position;
			
		}

		Rabin& operator=(const Rabin&) = delete;

		/**
		 * @brief Roll and add the hash to the symbol_positions
		 * @brief This function will check if position has reached the end
		 * @warning The position will be increased. THE WINDOW WILL NOT SLIDE
		 * @param num_of_rolls How many times it should roll, defaults to 1
		 * 
		 * @details find_pattern checks #poss_table for a pattern match, if found it will keep searching down the chain of previous poss_table entries,
		 *			which have been saved in #prev_poss_table by #roll_hash. This is repeated a #MAX_CHAIN amount of times.
		 */
		void roll_hash(uint32_t num_of_rolls = 1) noexcept;


		/**
		 * @brief Find the biggest pattern between the look ahead buffer and search buffer
		 */
		Token find_pattern();


		[[nodiscard]] auto get_pos() const noexcept
		{
			return position;
		}

	private:
		const Window& data;
		uint32_t hash = 0;
		uint32_t position;
		int32_t poss_table[MAX_HASH_SIZE];
		int32_t prev_poss_table[MAX_WINDOW_SIZE];
		const size_t MAX_CHAIN;

		[[nodiscard]] static auto bucket_index(uint32_t const hash_) noexcept -> uint32_t
		{
			if constexpr (std::popcount(MAX_HASH_SIZE) == 1)
			{
				return hash_ & (MAX_HASH_SIZE - 1);
			}
			else
			{
				return hash_ % MAX_HASH_SIZE;
			}
		}

		[[nodiscard]] static auto prev_bucket_index(uint32_t const hash_) noexcept -> uint32_t
		{
			if constexpr (std::popcount(MAX_WINDOW_SIZE) == 1)
			{
				return hash_ & (MAX_WINDOW_SIZE - 1);
			}
			else
			{
				return hash_ % MAX_WINDOW_SIZE;
			}
		}

		[[nodiscard]] static auto get_max_chain_from_preset(CompPreset const preset) noexcept -> size_t
		{
			switch (preset)
			{
			case COMP_MAX:
			case COMP_8:
				return 5;
			case COMP_7:
			case COMP_6:
			case COMP_5:
				return 4;
			case COMP_4:
			case COMP_3:
			case COMP_2:
				return 3;
			case COMP_1:
				return 2;
			case NO_COMP:
				assert(false);
				break;
			}
		}
	};


}


/**
 * @brief Represents a block of data encoded by LZ77.
 */
template <typename Alloc = std::allocator<LZ77_Token>>
class LZ77Block
{
private:
	uint32_t compressed_length_{};
	uint32_t uncompressed_length_{};
	std::vector<LZ77_Token, Alloc> buffer_;

public:
	using value_type = LZ77_Token;
	static constexpr auto HEADER_SIZE = sizeof(decltype(compressed_length_)) + sizeof(decltype(uncompressed_length_));
	/**
	 * @brief Grab the LZ77 header
	 * @param data the raw data
	 * @return A pair of the values @code { compressed_length, uncompressed_length } @endcode
	 */
	static auto extract_header(std::span<std::byte const> const data)
	{
		constexpr size_t len1 = sizeof(decltype(compressed_length_));
		constexpr size_t len2 = sizeof(decltype(uncompressed_length_));

		if (data.size() < len1 + len2)
			throw std::out_of_range("LZ77 block size is too small");

		decltype(compressed_length_) compressed_length;
		decltype(uncompressed_length_) uncompressed_length;

		std::memcpy(&compressed_length, data.data(), len1);//get compressed_length from data
		std::memcpy(&uncompressed_length, data.data() + len1, len2);//get uncompressed_length from data

		return std::make_pair(compressed_length, uncompressed_length);
	}

	/**
	 * @brief Grab the LZ77 header
	 * @param data the Sym data
	 * @return A pair of the values @code { compressed_length, uncompressed_length } @endcode
	 */
	static auto extract_header(std::span<Sym const> const data)
	{
		return extract_header({reinterpret_cast<std::span<std::byte const>::pointer>(data.data()),
								reinterpret_cast<std::span<std::byte const>::pointer>(data.data() + data.size())});
	}

	/**
	 * @param data Bytes interpreted as the block
	 * @return The size of the block, including the header
	 */
	static auto compressed_size(std::span<std::byte const> const data)
	{
		return HEADER_SIZE + extract_header(data).first;
	}

	/**
	 * @param data Bytes interpreted as the block
	 * @return The size of the uncompressed block, including the header
	 */
	static auto uncompressed_size(std::span<std::byte const> const data)
	{
		return HEADER_SIZE + extract_header(data).second;
	}

	explicit LZ77Block(std::span<std::byte const> const data, Alloc const& alloc = Alloc())
		: buffer_(alloc)
	{
		constexpr size_t len1 = sizeof(decltype(compressed_length_));
		constexpr size_t len2 = sizeof(decltype(uncompressed_length_));

		auto const [compressed_length__, uncompressed_length__] = extract_header(data);
		compressed_length_ = compressed_length__;
		uncompressed_length_ = uncompressed_length__;

		if (HEADER_SIZE + compressed_length_ > data.size())
		{
			report({.message_ID_or_progress = ErrorType::FILE_CORRUPTED, .failing_option = "An lz77 header is corrupted."});
		}

		buffer_.reserve(data.size());
		buffer_ = std::vector<LZ77_Token>{
			reinterpret_cast<LZ77_Token const*>(data.data() + HEADER_SIZE),
			reinterpret_cast<LZ77_Token const*>(data.data() + HEADER_SIZE +  compressed_length_)
		};
	}

	LZ77Block(Alloc const& alloc = Alloc()) : buffer_(alloc) {}

	/**
	 * @brief Convert the block into a byte buffer
	 */
	template <ByteSink Sink = byte_sinks::VectorSink<>>
	void serialize(Sink& sink) const noexcept
	{
		sink.append(as_bytes_le(compressed_length_));
		sink.append(as_bytes_le(uncompressed_length_));
		sink.append({ reinterpret_cast<std::byte const*>(buffer_.data()), buffer_.size() * sizeof(typename decltype(buffer_)::value_type) });
	}

	void write_to(std::ofstream& output) const
	{
		constexpr size_t len1 = sizeof(decltype(compressed_length_));
		constexpr size_t len2 = sizeof(decltype(uncompressed_length_));

		output.write(reinterpret_cast<char const*>(&compressed_length_), len1);
		output.write(reinterpret_cast<char const*>(&uncompressed_length_), len2);
		output.write(reinterpret_cast<char const*>(buffer_.data()), buffer_.size() * sizeof(typename decltype(buffer_)::value_type));
	}

	void push_back(LZ77_Token const token)
	{
		compressed_length_ += sizeof(LZ77_Token);
		uncompressed_length_ += token.length + sizeof(decltype(LZ77_Token::symbol));
		buffer_.push_back(token);
	}

	void clear() noexcept
	{
		buffer_.clear();
		compressed_length_ = 0;
		uncompressed_length_ = 0;
	}

	constexpr auto reserve(size_t const size) { buffer_.reserve(size);}

	[[nodiscard]] constexpr auto size() const noexcept { return buffer_.size(); }
	[[nodiscard]] constexpr auto size_bytes() const noexcept -> size_t { return buffer_.size() * sizeof(LZ77_Token); }
	[[nodiscard]] constexpr auto compressed_length() const noexcept {return compressed_length_;}
	[[nodiscard]] constexpr auto uncompressed_length() const noexcept {return uncompressed_length_;}
	[[nodiscard]] constexpr auto const& operator[](size_t const index) const noexcept {return buffer_[index];}
	[[nodiscard]] constexpr auto begin() const noexcept {return buffer_.begin();}
	[[nodiscard]] constexpr auto end() const noexcept {return buffer_.end();}

	template <typename Self>
	auto&& buffer(this Self& self) noexcept
	{
		return std::forward<Self>(self).buffer_;
	}


	//setter
	[[nodiscard]] constexpr auto& operator[](size_t const index) noexcept
	{
		assert(index < buffer_.size());
		return buffer_[index];
	}



};


template<typename Alloc = std::allocator<LZ77Block<>::value_type>, typename DataAlloc = std::allocator<Sym>>
class LZ77Model
{
public:
	using block_type = LZ77Block<Alloc>;
	using data_type = std::vector<Sym, DataAlloc>;

	LZ77Model(std::span<Sym const> const data, CompPreset const preset)
		: raw_data_({reinterpret_cast<std::byte const *>(data.data()), data.size_bytes()}),
		  window(Window{data, preset}),
		  pattern_matcher(alg::Rabin{window.value(), 0, preset})
	{
	}

	explicit LZ77Model(std::span<Sym const> const data)//Can be simplified as CompPreset::NO_COMP exists
		: raw_data_({reinterpret_cast<std::byte const *>(data.data()), data.size_bytes()}) {}

	/**
	 * @brief Compress the data received in the constructor
	 * @param out_data Where the output will go
	 *
	 * @note Will clear out_data block
	 */
	void compress(LZ77Block<Alloc>& out_data)
	{
		out_data.clear();
		out_data.reserve(raw_data_.size() / sizeof(LZ77_Token));

		size_t consumed_tokens{};
		//loop over the stream
		for (auto i = raw_data_.begin(); i != raw_data_.end();)
		{
			auto const token = this->pattern_matcher.value().find_pattern();

			auto const num_of_rolls = token.length + 1;
			this->pattern_matcher.value().roll_hash(num_of_rolls);//roll, updates the hash and position
			this->window.value().slide(num_of_rolls);
			i += num_of_rolls;
			consumed_tokens += num_of_rolls;

			out_data.push_back(token);
		}
	}

	/**
	 * @brief Decompress the data received in the constructor
	 * @param out_data Where the output will go
	 *
	 * @note Will clear out_data buffer
	 */
	void decompress(std::vector<Sym, DataAlloc>& out_data) const
	{
		block_type in_data{raw_data_};
		out_data.reserve(in_data.size() / sizeof(Sym));
		for (auto const token : in_data)
		{
			if (token.offset != 0)
			{
				auto size = out_data.size();
				for (auto _ : std::views::iota(0U, token.length))
					out_data.push_back(out_data[size - token.offset]);
			}
			out_data.emplace_back(token.symbol);
		}
		assert(out_data.capacity() == out_data.size());
	}

private:
	std::span<std::byte const> const raw_data_;
	std::optional<Window> window;
	std::optional<alg::Rabin> pattern_matcher;
};


