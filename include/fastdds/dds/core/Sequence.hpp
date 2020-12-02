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

template<typename T>
class GenericSequence
{
public:
    using size_type = uint32_t;
    using element_type = T*;

    GenericSequence() = default;

    GenericSequence(
            size_type maximum)
    {
        if (!maximum)
        {
            return;
        }

        resize(maximum);
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
            return;
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
        bool other_ownership = other.has_ownership();

        if (has_ownership_ != other_ownership)
        {
            release();
        }

        if (!other_ownership)
        {
            replace(other.maximum(), other.length(), other.get_buffer(), false);
        }
        else
        {
            length(other.length());
            element_type* other_buf = other.get_buffer();
            for (size_type n = 0; n < length_; ++n)
            {
                *(elements_[n]) = *(other_buf[n]);
            }
        }

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
            if (!has_ownership_)
            {
                return false;
            }

            resize(new_length);
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

        return elements_[index];
    }

    const element_type& operator[](
            size_type index) const
    {
        if (index >= length_)
        {
            throw std::out_of_range();
        }

        return elements_[index];
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

    void resize(
            size_type maximum)
    {
        assert(has_ownership_);

        // Resize collection and get new pointer
        data_.reserve(maximum);
        data_.resize(maximum);
        elements_ = data_.data();

        // Allocate individual elements
        while (maximum_ < maximum)
        {
            elements_[maximum_++] = new T();
        }
    }

    void release()
    {
        if (has_ownership_ && elements_)
        {
            for (size_t n = 0; n < maximum_; ++n)
            {
                element_type elem = elements_[n];
                delete elem;
            }
            std::vector<element_type>().swap(data_);
        }

        maximum_ = 0u;
        length_ = 0u;
        elements_ = nullptr;
        has_ownership_ = true;
    }

    size_type maximum_ = 0u;
    size_type length_ = 0u;
    element_type* elements_ = nullptr;
    bool has_ownership_ = true;
    std::vector<element_type> data_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_DDS_CORE_SEQUENCE_HPP_
