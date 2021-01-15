// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _TEST_UNITTEST_DDS_SUBSCRIBER_FOOBOUNDEDTYPE_HPP_
#define _TEST_UNITTEST_DDS_SUBSCRIBER_FOOBOUNDEDTYPE_HPP_

#include <cstdint>
#include <vector>

#include <fastcdr/Cdr.h>

namespace eprosima {
namespace fastdds {
namespace dds {

class FooBoundedType
{
public:

    inline uint32_t index() const
    {
        return index_;
    }

    inline uint32_t& index()
    {
        return index_;
    }

    inline void index(
            uint32_t value)
    {
        index_ = value;
    }

    inline const std::vector<char>& message() const
    {
        return message_;
    }

    inline std::vector<char>& message()
    {
        return message_;
    }

    inline void message(
            const std::vector<char>& value)
    {
        message_ = value;
    }

    inline static size_t getCdrSerializedSize(
            const FooBoundedType& data)
    {
        return 4u + 4u + data.message().size();
    }

    inline void serialize(
            eprosima::fastcdr::Cdr& scdr) const
    {
        scdr << index_;
        scdr << message_;
    }

    inline void deserialize(
            eprosima::fastcdr::Cdr& dcdr)
    {
        dcdr >> index_;
        dcdr >> message_;
    }

    inline bool isKeyDefined()
    {
        return false;
    }

    inline void serializeKey(
            eprosima::fastcdr::Cdr& /*scdr*/) const
    {
    }

private:

    uint32_t index_ = 0;
    std::vector<char> message_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif  // _TEST_UNITTEST_DDS_SUBSCRIBER_FOOBOUNDEDTYPE_HPP_
