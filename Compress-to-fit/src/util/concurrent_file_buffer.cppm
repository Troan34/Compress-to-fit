export module util:concurrent_file_buffer;

import std.compat;

/**
 * @brief Circular array where if size > capacity, size - minimum_size is written to stream
 * @tparam T 
 */
export template <typename T>
	requires std::is_trivially_copyable_v<T>
class ConcurrentFileBuffer
{
public:

	ConcurrentFileBuffer(std::ofstream out_stream, size_t minimum_size)
		:out_stream_(out_stream), minimum_size_(minimum_size)
	{
		reserve(minimum_size_ + (1.5f * minimum_size_));
	}


	constexpr void push_back(T const& value)
	{
		lock.lock();

		auto true_index = index_ % capacity_;

		if (index_ + 1 - written_index_ > capacity_)
			write_to_disk();


		data_[true_index] = value;
		index_++;
		
		lock.unlock();

	}

	constexpr void push_back(T&& value);


	template <std::ranges::input_range R>
		requires std::convertible_to<std::ranges::range_reference_t<R>, T>
	constexpr void append_range(R&& rg);

	constexpr void reserve(size_t new_capacity)
	{
		if (new_capacity <= capacity_) return;

		//allocate size for new data
		T* new_data_ = allocator.allocate(new_capacity);
		std::unique_lock<std::mutex> lock(mut);
		
		if constexpr (std::is_trivially_copyable_v<T>)//if trivially copyable use std::memcpy
		{
			std::memcpy(new_data_, data_, size_ * sizeof(T));
		}

		/* To be used if we ever want to use non trivial types
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

	constexpr T operator[](size_t index) const
	{
		//if in debug mode
#ifndef NDEBUG
		//holy yap
		if ((index % capacity_) > (index_ % capacity_) or (index_ % capacity_) < ((index_ % capacity_) - size_))
			throw std::out_of_range("Accessed an element outside of buffer, make sure to increase your index.");
#endif
		std::shared_lock<std::mutex> lock(mut);
		return data_[index];
	}

	//getters
	constexpr size_t capacity() const noexcept { return capacity_; }
	constexpr size_t minimum_size() const noexcept { return minimum_size_; }


private:
	std::ofstream out_stream_;
	T* data_;
	std::allocator<T> allocator;
	std::mutex mut;
	std::unique_lock<std::mutex> lock{ mut, std::defer_lock  }; //only one thread at a time may write to data_, so we keep this class-wide
	size_t index_ = 0;
	size_t written_index_ = 0; //oldest index that has not yet been written to file
	size_t capacity_ = 0;
	size_t minimum_size_ = 0;


	/**
	 * @brief Write data outside of [start_unbuffered_, index_] to disk
	 * 
	 * @pre #mut must be locked through #lock
	 */
	constexpr void write_to_disk()
	{
#ifndef NDEBUG
		if (!lock.owns_lock()) std::terminate();
#endif

		if (index_ < minimum_size_)//if buffer is not full
			return;

		auto elements_to_write_n = index - minimum_size_ - written_index_;

		if constexpr (std::is_trivially_copyable_v<T>)
		{
			//TODO: this technically blocks a thread, but if there are 12 threads AND they aren't writing frequently, it shouldn't be THAT bad. Test this.
			out_stream_.write(reinterpret_cast<char const*>(&data_[written_index_]), elements_to_write_n * sizeof(T));
		}

		written_index_ = index_ - minimum_size_;

	}

};