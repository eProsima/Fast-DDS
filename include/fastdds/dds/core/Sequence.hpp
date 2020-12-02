// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file Sequence.hpp
 */

#ifndef _FASTDDS_DDS_CORE_SEQUENCE_HPP_
#define _FASTDDS_DDS_CORE_SEQUENCE_HPP_

#include <cstdint>
#include <type_traits>
#include <vector>

namespace eprosima {
namespace fastdds {
namespace dds {

template<typename T, typename T_ptr = T*, T_ptr default_value = T_ptr()>
class GenericSequence
{
public:
	using size_type = uint32_t;
	using element_type = T_ptr;

	GenericSequence() = default;

	GenericSequence(
			size_type maximum)
	{
		elements_ = new element_type[maximum];
		maximum_ = maximum;
		for (size_type n = 0; n < maximum_; ++n)
		{
			elements_[n] = default_value;
		}
	}

	GenericSequence(
			size_type maximum,
			size_type length,
			element_type* data,
			bool take_ownership = false)
	{
		replace(maximum, length, data, take_ownership);
	}

	~GenericSequence()
	{
		if (elements_ && !has_ownership_)
		{
			// Warning
		}

		release();
	}

	GenericSequence(
			const GenericSequence& other)
	{
		*this = other;
	}

	GenericSequence& operator=(
			const GenericSequence& other)
	{
		// shallow copy
		release();
		replace(other.maximum(), other.length(), other.get_buffer(), false);
		return *this;
	}

	size_type maximum() const
	{
		return maximum_;
	}

	size_type length() const
	{
		return length_;
	}

	bool length(
			size_type new_length)
	{
		if (new_length > maximum_)
		{
			return false;
		}

		length_ = new_length;
		return true;
	}

	element_type& operator[](
			size_type index)
	{
		if (index >= length_)
		{
			throw std::out_of_range();
		}

		return allocate(elements_[index]);
	}

	const element_type& operator[](
			size_type index) const
	{
		if (index >= length_)
		{
			throw std::out_of_range();
		}

		return allocate(elements_[index]);
	}

	bool has_ownership() const
	{
		return has_ownership_;
	}

	bool replace(
			size_type maximum,
		    size_type length,
			element_type* data,
			bool take_ownership = false)
	{
		if (!has_ownership_ || maximum_ > 0)
		{
			return false;
		}

		maximum_ = maximum;
		length_ = length;
		elements_ = data;
		has_ownership_ = take_ownership;
		return true;
	}

	element_type* get_buffer(
			bool orphan = false)
	{
		if (!orphan)
		{
			return elements_;
		}

		if (!has_ownership_)
		{
			return nullptr;
		}

		element_type* ret_val = elements_;

		maximum_ = 0u;
		length_ = 0u;
		elements_ = nullptr;

		return ret_val;
	}

	const element_type* get_buffer() const
	{
		return elements_;
	}

private:

	void release()
	{
		if (has_ownership_ && elements_)
		{
			for (size_t n = 0; n < maximum_; ++n)
			{
				deallocate(elements_[n]);
			}
			delete[] elements_;
			elements_ = nullptr;
		}
	}

	template<bool condition = std::is_same<typename T, typename T_ptr>::value>
	element_type& allocate(
			element_type& value)
	{
		return value;
	}

	template<>
	element_type& allocate<false>(
			element_type& value)
	{
		if (value == default_value)
		{
			value = new T();
		}
		return value;
	}

	template<bool condition = std::is_same<typename T, typename T_ptr>::value>
	void deallocate(
			element_type& value)
	{
	}

	template<>
	void deallocate<false>(
			element_type& value)
	{
		delete value;
		value = default_value;
	}

	size_type maximum_ = 0u;
	size_type length_ = 0u;
	element_type* elements_ = nullptr;
	bool has_ownership_ = true;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DDS_CORE_SEQUENCE_HPP_
