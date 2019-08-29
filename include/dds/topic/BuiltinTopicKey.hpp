/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
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
 */

#ifndef OMG_DDS_TOPIC_BUILTIN_TOPIC_KEY_HPP_
#define OMG_DDS_TOPIC_BUILTIN_TOPIC_KEY_HPP_

#include <dds/topic/detail/BuiltinTopicKey.hpp>
#include <dds/core/Value.hpp>
#include <dds/core/types.hpp>

#include <dds/core/Value.hpp>
#include <dds/core/detail/inttypes.hpp>

namespace dds {
namespace topic {

/**
 * @brief
 * Global unique identifier of the Topic.
 */
template<typename D>
class TBuiltinTopicKey : public dds::core::Value<D>
{
public:
    /**
     * Gets the BuiltinTopicKey.
     *
     * @return the BuiltinTopicKey
     */
    const int32_t* value() const;

    /**
     * Sets the BuiltinTopicKey.
     *
     * @param v the value to set
     */
    void value(
            const int32_t v[]);
};

typedef detail::BuiltinTopicKey BuiltinTopicKey;

} //namespace topic
} //namespace dds

#endif //OMG_DDS_TOPIC_BUILTIN_TOPIC_KEY_HPP_
