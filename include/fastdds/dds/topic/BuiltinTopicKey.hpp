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

namespace eprosima {
namespace fastdds {
namespace dds {

using BUILTIN_TOPIC_KEY_TYPE_NATIVE = int32_t;

class BuiltinTopicKey
{
public:
    BuiltinTopicKey(){}

    BuiltinTopicKey(
            int32_t key[])
    {
        key_[0] = key[0];
        key_[1] = key[1];
        key_[2] = key[2];
    }

    ~BuiltinTopicKey() {}

    const int32_t* key() const
    {
        return key_;
    }

    void key(
            int32_t k[])
    {
        key_[0] = k[0];
        key_[1] = k[1];
        key_[2] = k[2];
    }

    void key(
            const BuiltinTopicKey& key)
    {
        key_[0] = key.key()[0];
        key_[1] = key.key()[1];
        key_[2] = key.key()[2];
    }

    bool operator ==(
            const BuiltinTopicKey& other) const
    {
        return (key_[0] == other.key_[0] &&
                key_[1] == other.key_[1] &&
                key_[2] == other.key_[2]);
    }

private:
    BUILTIN_TOPIC_KEY_TYPE_NATIVE key_[3];
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima


#endif // _FASTDDS_BUILTIN_TOPIC_KEY_HPP_
