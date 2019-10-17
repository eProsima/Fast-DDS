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
 * @file ParticipantBuiltinTopicData.hpp
*/

#ifndef _FASTDDS_PARTICIPANT_BUILTIN_TOPIC_DATA_HPP_
#define _FASTDDS_PARTICIPANT_BUILTIN_TOPIC_DATA_HPP_

#include <fastdds/dds/topic/BuiltinTopicKey.hpp>
#include <fastdds/dds/core/policy/QosPolicies.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class ParticipantBuiltinTopicData
{
public:
    ParticipantBuiltinTopicData() {}

    ~ParticipantBuiltinTopicData() {}

    const BuiltinTopicKey& key() const
    {
        return key_;
    }

    void key(
            const BuiltinTopicKey& key)
    {
        key_ = key;
    }

    const UserDataQosPolicy& user_data() const
    {
        return user_data_;
    }

    void user_data(
            const UserDataQosPolicy& user_data)
    {
        user_data_ = user_data;
    }

    bool operator ==(
            const ParticipantBuiltinTopicData& other)
    {
        return (key_ == other.key() &&
                user_data_ == other.user_data());
    }

private:

    BuiltinTopicKey key_;

    UserDataQosPolicy user_data_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // _FASTDDS_PARTICIPANT_BUILTIN_TOPIC_DATA_HPP_
