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

#ifndef EPROSIMA_DDS_CORE_DELEGATE_BUILTIN_TOPIC_TYPES_HPP_
#define EPROSIMA_DDS_CORE_DELEGATE_BUILTIN_TOPIC_TYPES_HPP_

//#include <dds/topic/Topic.hpp>
//TODO: Fix when BuiltinTopicDelegate and BuiltinTopic are implemented
//#include <dds/topic/detail/TTopicImpl.hpp>
//#include <dds/topic/detail/TBuiltinTopicImpl.hpp>

#if defined (OMG_DDS_X_TYPES_BUILTIN_TOPIC_TYPES_SUPPORT)

namespace dds {
namespace core {
namespace detail {

//TODO: Fix when BuiltinTopicDelegate and BuiltinTopic are implemented
//typedef dds::core::TBytesTopicType<org::opensplice::topic::BytesTopicTypeDelegate> BytesTopicType;
class BytesTopicType
{
};

//TODO: Fix when BuiltinTopicDelegate and BuiltinTopic are implemented
//typedef dds::core::TStringTopicType<org::opensplice::topic::StringTopicTypeDelegate> StringTopicType;
class StringTopicType
{
};

//TODO: Fix when BuiltinTopicDelegate and BuiltinTopic are implemented
//typedef dds::core::TKeyedBytesTopicType<org::opensplice::topic::KeyedBytesTopicTypeDelegate> KeyedBytesTopicType;
class KeyedBytesTopicType
{
};

//TODO: Fix when BuiltinTopicDelegate and BuiltinTopic are implemented
//typedef dds::core::TKeyedStringTopicType<org::opensplice::topic::KeyedStringTopicTypeDelegate> KeyedStringTopicType;
class KeyedStringTopicType
{
};

} //namespace detail


//==============================================================================
//            Bytes Template
//==============================================================================

template<typename DELEGATE>
TBytesTopicType<DELEGATE>::TBytesTopicType()
    : Value<DELEGATE>(
        std::vector<uint8_t>())
{
}

template<typename DELEGATE>
TBytesTopicType<DELEGATE>::TBytesTopicType(
        const std::vector<uint8_t>& data)
    : dds::core::Value<DELEGATE>(
        data)
{
}

template<typename DELEGATE>
TBytesTopicType<DELEGATE>::operator std::vector<uint8_t>&() const
{
    return this->delegate().value();
}

template<typename DELEGATE>
const std::vector<uint8_t>& TBytesTopicType<DELEGATE>::data() const
{
    return this->delegate().value();
}

template<typename DELEGATE>
void TBytesTopicType<DELEGATE>::data(
        const std::vector<uint8_t>& data)
{
    this->delegate().value(data);
}

//==============================================================================
//            String Template
//==============================================================================

template<typename DELEGATE>
TStringTopicType<DELEGATE>::TStringTopicType()
    : Value<DELEGATE>(
        std::string())
{
}

template<typename DELEGATE>
TStringTopicType<DELEGATE>::TStringTopicType(
        const std::string& data)
    : dds::core::Value<DELEGATE>(
        data)
{
}

template<typename DELEGATE>
TStringTopicType<DELEGATE>::operator std::string& () const
{
    return this->delegate().value();
}

template<typename DELEGATE>
const std::string& TStringTopicType<DELEGATE>::data() const
{
    return this->delegate().value();
}

template<typename DELEGATE>
void TStringTopicType<DELEGATE>::data(
        const std::string& data)
{
    this->delegate().value(data);
}

//==============================================================================
//            KeyedBytes Template
//==============================================================================

template<typename DELEGATE>
TKeyedBytesTopicType<DELEGATE>::TKeyedBytesTopicType()
    : Value<DELEGATE>(
        std::string(),
        std::vector<uint8_t>())
{
}

template<typename DELEGATE>
TKeyedBytesTopicType<DELEGATE>::TKeyedBytesTopicType(
        const std::string& key,
        const std::vector<uint8_t>& value)
    : Value<DELEGATE>(
        key,
        value)
{
}

template<typename DELEGATE>
const std::string& TKeyedBytesTopicType<DELEGATE>::key() const
{
    return this->delegate().key();
}

template<typename DELEGATE>
void TKeyedBytesTopicType<DELEGATE>::key(
        const std::string& key)
{
    this->delegate().key(key);
}

template<typename DELEGATE>
const std::vector<uint8_t>& TKeyedBytesTopicType<DELEGATE>::value() const
{
    return this->delegate().value();
}

template<typename DELEGATE>
void TKeyedBytesTopicType<DELEGATE>::value(
        const std::vector<uint8_t>& value)
{
    this->delegate().value(value);
}

//==============================================================================
//            KeyedString Template
//==============================================================================

template<typename DELEGATE>
TKeyedStringTopicType<DELEGATE>::TKeyedStringTopicType()
    : Value<DELEGATE>(
        std::string(),
        std::string())
{
}

template<typename DELEGATE>
TKeyedStringTopicType<DELEGATE>::TKeyedStringTopicType(
        const std::string& key,
        const std::string& value)
    : Value<DELEGATE>(
        key,
        value)
{
}

template<typename DELEGATE>
const std::string& TKeyedStringTopicType<DELEGATE>::key() const
{
    return this->delegate().key();
}

template<typename DELEGATE>
void TKeyedStringTopicType<DELEGATE>::key(
        const std::string& key)
{
    this->delegate().key(key);
}

template<typename DELEGATE>
const std::string& TKeyedStringTopicType<DELEGATE>::value() const
{
    return this->delegate().value();
}

template<typename DELEGATE>
void TKeyedStringTopicType<DELEGATE>::value(
        const std::string& value)
{
    this->delegate().value(value);
}

} //namespace core
} //namespace dds

#endif //OMG_DDS_X_TYPES_BUILTIN_TOPIC_TYPES_SUPPORT

#endif //EPROSIMA_DDS_CORE_DELEGATE_BUILTIN_TOPIC_TYPES_HPP_
