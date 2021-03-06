#pragma once
#include <cstddef>
#include <cstdint>
#include <FEL/optional.hpp>
#include <FEL/iterator.hpp>
#include <FEL/function.hpp>

namespace fel{

	template<typename T>
	class buffer final{
	public:
		struct buffer_iterator final
		{
			T* data;
			typedef T value_type;
			typedef std::size_t difference_type;
			static constexpr iterator_type_t iterator_type = iterator_type_t::contiguous_iterator;

			constexpr buffer_iterator(const buffer_iterator& oth)
			: data{oth.data}
			{}

			constexpr buffer_iterator(T* ptr)
			: data{ptr}
			{}

			constexpr T& operator*(){
				return *data;
			}

			constexpr buffer_iterator operator++()
			{
				return buffer_iterator{++data};
			}

			constexpr buffer_iterator operator++(int)
			{
				return buffer_iterator{data++};
			}

			constexpr buffer_iterator operator--()
			{
				return buffer_iterator{--data};
			}

			constexpr buffer_iterator operator--(int)
			{
				return buffer_iterator{data--};
			}

			constexpr buffer_iterator operator+(const std::size_t offset)
			{
				return buffer_iterator{data+offset};
			}

			constexpr buffer_iterator operator-(const std::size_t offset)
			{
				return buffer_iterator{data-offset};
			}

			constexpr difference_type operator-(const buffer_iterator& oth) const
			{
				return (T*)data-(T*)oth.data;
			}

			constexpr bool operator==(const buffer_iterator& oth)
			{
				return data==oth.data;
			}

			constexpr bool operator!=(buffer_iterator& oth)
			{
				return data!=oth.data;
			}

			constexpr bool before_or_equal(const buffer_iterator& oth)
			{
				return reinterpret_cast<std::intptr_t>(data) <= reinterpret_cast<std::intptr_t>(oth.data);
			}

			constexpr bool operator<=(const buffer_iterator& oth)
			{
				return before_or_equal(oth);
			}
		};
	private:
		buffer_iterator begin_elem;
		buffer_iterator end_elem;
	public:
		using associated_iterator = buffer_iterator;

		constexpr buffer(T* beg_ptr, T* end_ptr)
		: begin_elem{beg_ptr}
		, end_elem{end_ptr}
		{}

		constexpr buffer(T* beg_ptr, std::size_t sz)
		: begin_elem{beg_ptr}
		, end_elem{beg_ptr+sz}
		{}

		constexpr typename buffer_iterator::difference_type size() const
		{
			return end_elem - begin_elem;
		}

		constexpr associated_iterator begin() const
		{
			return begin_elem;
		}

		constexpr associated_iterator end() const
		{
			return end_elem;
		}

		constexpr T& operator[](std::size_t offset)
		{
			return *(begin_elem+offset);
		}

		optional<buffer_iterator> at(std::size_t offset)
		{
			auto elem = begin()+offset;
			if(!contains(elem))
			{
				return nullopt;
			}
			return elem;
		}

		constexpr bool contains(buffer_iterator ptr)
		{
			return 
				ptr.data < end_elem.data
				&& ptr.data >= begin_elem.data;
		}
	};

	template<typename T>
	class unique_buffer final{
		buffer<T> data;
		fel::function<void(void*)> deallocator;
	public:
		using associated_iterator = typename buffer<T>::associated_iterator;

		unique_buffer(T* beg_ptr, T* end_ptr, fel::function<void(void*)> deallocator_impl)
		: data{beg_ptr,end_ptr}
		, deallocator{deallocator_impl}
		{}

		unique_buffer(unique_buffer&) = delete;
		unique_buffer& operator=(unique_buffer&) = delete;

		unique_buffer(unique_buffer&& oth)
		: data{oth.data}
		, deallocator{oth.deallocator}
		{
			oth.data=buffer<T>{nullptr,nullptr};
		}

		~unique_buffer()
		{
			if(!data.contains(nullptr))
			{
				for(auto& elem : data)
				{
					elem.~T();
				}
				deallocator(&*data.begin());
			}
		}

		constexpr auto begin()
		{
			return data.begin();
		}

		constexpr auto end()
		{
			return data.end();
		}

		constexpr T& operator[](std::size_t offset)
		{
			return data[offset];
		}

		constexpr auto at(std::size_t offset)
		{
			return data.at(offset);
		}

		constexpr bool contains(typename buffer<T>::buffer_iterator ptr)
		{
			return data.contains(ptr);
		}
	};
}