module;
#if defined(__INTELLISENSE__)
#include "../../for_intellisense/everything.hpp"
#endif
export module util:concurrent_file_buffer;

import std.compat;

/**
 * @brief Ring queue-like where if size > capacity, the oldest elements are written to stream.
 *		  At least the most recent #minimum_size_ number of elements shall be available for reading.
 * @tparam T type stored, must be trivially copyable
 */
export template <typename T>
	requires std::is_trivially_copyable_v<T>
class ConcurrentFileBuffer
{
public:

	ConcurrentFileBuffer(std::ofstream out_stream, size_t const minimum_size)
		:out_stream_(std::move(out_stream)), minimum_size_(minimum_size)
	{
		reserve(1.5f * minimum_size_);
	}

	~ConcurrentFileBuffer()
	{
		auto elements_to_write_n = index_ - written_index_;
		out_stream_.write(reinterpret_cast<char const*>(&data_[written_index_]), elements_to_write_n * sizeof(T));
		allocator.deallocate(data_, capacity_);
	}

	/**
	 * @brief 
	 * @param value
	 * @pre #index_ > #minimum_size_
	 */
	constexpr void push(T const& value)
	{
		lock.lock();

		write_buffer();

		data_[true_index()] = value;
		index_++;
		
		lock.unlock();
	}

	/**
	 * @brief
	 * @param value
	 * @pre #index_ > #minimum_size_
	 */
	constexpr void push(T&& value)
	{
		lock.lock();

		write_buffer();

		data_[true_index()] = value;
		index_++;

		lock.unlock();
	}

	/**
	 * @brief Appends a range to the container
	 * @param rg range to append
	 * @pre #index_ > #minimum_size_
	 */
	template <std::ranges::range R>
		requires std::convertible_to<std::ranges::range_reference_t<R>, T>
	constexpr void append_range(R&& rg)
	{
		auto size = rg.end() - rg.start();
		lock.lock();

		if (index_ + size - written_index_ > capacity_)//if we need to write elements to disk
		{
			auto const elements_to_write_n = index_ - minimum_size_ - written_index_;

			if constexpr (std::is_trivially_copyable_v<T>)
			{
				//TODO: this blocks a thread, but if there are 12 threads AND they aren't writing frequently, it shouldn't be THAT bad. Benchmark this.
				//We could try coroutines here
				//Maybe we can lock the stream, or stuff like that
				out_stream_.write(reinterpret_cast<char const*>(&data_[written_index_]), elements_to_write_n * sizeof(T));
			}

			written_index_ = index_ - minimum_size_;
		}

		//might be worthwhile to optimize this
		std::for_each(rg.begin(), rg.end(), 
					  [&](T const& value)
					  {
						  data_[index_] = value;
						  index_++;
					  });

		lock.unlock();
	}

	/**
	 * @brief Increase capacity to at least #new_capacity
	 * @param new_capacity New capacity of the buffer
	 */
	constexpr void reserve(size_t new_capacity) noexcept(false)
	{
		if (new_capacity <= capacity_) return;

		//allocate size for new data
		T* new_data_ = allocator.allocate(new_capacity);
		auto const size = index_ - written_index_;
		std::unique_lock<std::shared_mutex> lock(mut);
		
		if constexpr (std::is_trivially_copyable_v<T>)//if trivially copyable use std::memcpy
		{
			std::memcpy(new_data_, data_, size * sizeof(T));
		}

		/* To be used if we ever want to use non-trivial types
		else//if not, call T's ctor
		{
			for (size_t i{}; i < size_; i++)
			{
				new (&new_data_[i]) T(std::move(data_[i]));

				data_[i].~T();
			}
		}*/
		
		data_ = new_data_;
		auto old_cap = capacity_;
		lock.unlock();

		allocator.deallocate(data_, old_cap);
	}

	constexpr auto operator[](size_t index) const -> T
	{
		//if in debug mode
#ifndef NDEBUG
		auto size = index_ - written_index_;
		//holy yap
		if ((index % capacity_) > (index_ % capacity_) or (index_ % capacity_) < ((index_ % capacity_) - size))
			throw std::out_of_range("Accessed an element outside of buffer.");
#endif
		std::shared_lock<std::shared_mutex> lock(mut);
		return data_[index];
	}

	//getters
	constexpr size_t capacity() const noexcept { return capacity_; }
	constexpr size_t minimum_size() const noexcept { return minimum_size_; }

	//setters
	/**
	 * @brief Set a new minimum size. A value less than minimum_size will be ignored
	 *
	 * @param new_minimum_size New minimum size, shall be at least #minimum_size big
	 */
	void set_minimum_size(size_t const new_minimum_size) noexcept
	{
		lock.lock();
		if (new_minimum_size > minimum_size_)
			minimum_size_ = new_minimum_size;
		lock.unlock();
	}

private:
	std::ofstream out_stream_;
	T* data_;
	std::allocator<T> allocator;
	mutable std::shared_mutex mut;
	std::unique_lock<std::shared_mutex> lock{ mut, std::defer_lock  }; //only one thread at a time may write to data_, so we keep this class-wide
	size_t index_ = 0;
	size_t written_index_ = 0; //oldest index that has not yet been written to file
	size_t capacity_ = 0;
	size_t minimum_size_ = 0;

	/**
	 * @brief Writes elements outside the minimum range
	 * @pre lock MUST be locked with #lock
	 */
	auto inline write_buffer()
	{
		if (index_ + 1 - written_index_ > capacity_)//if we need to write elements to disk
		{
			auto const elements_to_write_n = index_ - minimum_size_ - written_index_;

			//TODO: this blocks a thread, but if there are 12 threads AND they aren't writing frequently, it shouldn't be THAT bad. Benchmark this.
			//We could try coroutines here
			//Maybe we can lock the stream, or stuff like that
			out_stream_.write(reinterpret_cast<char const*>(&data_[written_index_]), elements_to_write_n * sizeof(T));

			written_index_ = index_ - minimum_size_;
		}
	}


	inline constexpr size_t true_index() const noexcept { return index_ % capacity_; }
};