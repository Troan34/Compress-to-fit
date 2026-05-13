export module util:concurrent_file_buffer;

import std.compat;

/**
 * @brief Circular array where if size > capacity, size - minimum_size is written to stream
 * @tparam T 
 */
export template <typename T>
class ConcurrentFileBuffer
{
public:

	ConcurrentFileBuffer(std::ofstream out_stream, size_t minimum_size)
		:out_stream_(out_stream), minimum_size_(minimum_size)
	{
		reserve(minimum_size_);
	}


	constexpr void push_back(T const& value);
	constexpr void push_back(T&& value);


	template <std::ranges::input_range R>
		requires std::convertible_to<std::ranges::range_reference_t<R>, T>
	constexpr void append_range(R&& rg);

	constexpr void reserve(size_t new_capacity)
	{
		if (new_capacity <= capacity_) return;

		T* new_data_ = allocator.allocate(new_capacity);
		std::unique_lock<std::mutex> lock(mut);
		
		if constexpr (std::is_trivially_copyable<T>::value)
		{
			std::memcpy(new_data_, data_, size_ * sizeof(T));
		}
		else
		{
			for (size_t i{}; i < size_; i++)
			{
				new (&new_data_[i]) T(std::move(data_[i]));

				data_[i].~T();
			}
		}
		
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
	constexpr size_t size() const noexcept { return size_; }
	constexpr size_t minimum_size() const noexcept { return minimum_size_; }


private:
	std::ofstream out_stream_;
	T* data_;
	std::allocator<T> allocator;
	std::mutex mut;
	size_t size_ = 0;
	size_t index_ = 0;
	size_t capacity_ = 0;
	size_t minimum_size_ = 0;


};