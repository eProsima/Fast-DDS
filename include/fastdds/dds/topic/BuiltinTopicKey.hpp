/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/**
 * @file BuiltinTopicKey.hpp
 */

#ifndef _FASTDDS_BUILTIN_TOPIC_KEY_HPP_
#define _FASTDDS_BUILTIN_TOPIC_KEY_HPP_

#include <fastrtps/fastrtps_dll.h>
#include <fastdds/rtps/common/Guid.h>

#include <sstream>

namespace eprosima {
namespace fastdds {
namespace dds {

using BUILTIN_TOPIC_KEY_TYPE_NATIVE = int32_t;

class BuiltinTopicKey
{
public:

    BuiltinTopicKey(){}

    BuiltinTopicKey(
            const fastrtps::rtps::GUID_t& guid)
    {
        memcpy(&value_[0], &guid.guidPrefix.value[0], sizeof(guid.guidPrefix));
        memcpy(&value_[3], &guid.entityId.value[0], sizeof(guid.entityId));
    }

    BuiltinTopicKey(
            int32_t val[])
    {
        value_[0] = val[0];
        value_[1] = val[1];
        value_[2] = val[2];
        value_[3] = val[3];
    }

    ~BuiltinTopicKey() {}

    const int32_t* value() const
    {
        return value_;
    }

    void value(
            int32_t v[])
    {
        value_[0] = v[0];
        value_[1] = v[1];
        value_[2] = v[2];
        value_[3] = v[3];
    }

    void value(
            const BuiltinTopicKey& key)
    {
        value_[0] = key.value()[0];
        value_[1] = key.value()[1];
        value_[2] = key.value()[2];
        value_[3] = key.value()[3];
    }

    bool operator ==(
            const BuiltinTopicKey& other) const
    {
        return (value_[0] == other.value_[0] &&
               value_[1] == other.value_[1] &&
               value_[2] == other.value_[2] &&
               value_[3] == other.value_[3]);
    }

private:

    BUILTIN_TOPIC_KEY_TYPE_NATIVE value_[4];
};

inline std::ostream& operator <<(
        std::ostream& output,
        const BuiltinTopicKey& key)
{
    output << std::hex;
    uint8_t* ptr = (uint8_t*) key.value();
    for (uint32_t i = 0; i < 15; ++i)
    {
        output << (int) *ptr << ".";
        ptr++;
    }
    output << (int) *ptr;
    return output << std::dec;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima


#endif // _FASTDDS_BUILTIN_TOPIC_KEY_HPP_
