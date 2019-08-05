/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#ifndef OMG_DDS_CORE_DELEGATE_BUILTIN_TOPIC_TYPES_HPP_
#define OMG_DDS_CORE_DELEGATE_BUILTIN_TOPIC_TYPES_HPP_

#include <dds/topic/Topic.hpp>
#include <dds/topic/detail/TTopicImpl.hpp>
#include <dds/topic/detail/TBuiltinTopicImpl.hpp>
#include <org/opensplice/topic/BuiltinTopicDelegate.hpp>
#include <org/opensplice/topic/BuiltinTopic.hpp>

#if defined (OMG_DDS_X_TYPES_BUILTIN_TOPIC_TYPES_SUPPORT)

namespace dds { namespace core { namespace detail {

    typedef dds::core::TBytesTopicType<org::opensplice::topic::BytesTopicTypeDelegate>
    BytesTopicType;

    typedef dds::core::TStringTopicType<org::opensplice::topic::StringTopicTypeDelegate>
    StringTopicType;

    typedef dds::core::TKeyedBytesTopicType<org::opensplice::topic::KeyedBytesTopicTypeDelegate>
    KeyedBytesTopicType;

    typedef dds::core::TKeyedStringTopicType<org::opensplice::topic::KeyedStringTopicTypeDelegate>
    KeyedStringTopicType;
}}}





//==============================================================================
//            Bytes Template
//==============================================================================

template <typename DELEGATE>
dds::core::TBytesTopicType<DELEGATE>::TBytesTopicType() :
    dds::core::Value<DELEGATE>(std::vector<uint8_t>())
{
}

template <typename DELEGATE>
dds::core::TBytesTopicType<DELEGATE>::TBytesTopicType(const std::vector<uint8_t>& data) :
    dds::core::Value<DELEGATE>(data)
{
}

template <typename DELEGATE>
dds::core::TBytesTopicType<DELEGATE>::operator std::vector<uint8_t>& () const
{
    return this->delegate().value();
}

template <typename DELEGATE>
const std::vector<uint8_t>& dds::core::TBytesTopicType<DELEGATE>::data() const
{
    return this->delegate().value();
}

template <typename DELEGATE>
void dds::core::TBytesTopicType<DELEGATE>::data(const std::vector<uint8_t>& data)
{
    this->delegate().value(data);
}



//==============================================================================
//            String Template
//==============================================================================

template <typename DELEGATE>
dds::core::TStringTopicType<DELEGATE>::TStringTopicType() :
    dds::core::Value<DELEGATE>(std::string())
{
}

template <typename DELEGATE>
dds::core::TStringTopicType<DELEGATE>::TStringTopicType(const std::string& data) :
    dds::core::Value<DELEGATE>(data)
{
}

template <typename DELEGATE>
dds::core::TStringTopicType<DELEGATE>::operator std::string& () const
{
    return this->delegate().value();
}

template <typename DELEGATE>
const std::string& dds::core::TStringTopicType<DELEGATE>::data() const
{
    return this->delegate().value();
}

template <typename DELEGATE>
void dds::core::TStringTopicType<DELEGATE>::data(const std::string& data)
{
    this->delegate().value(data);
}



//==============================================================================
//            KeyedBytes Template
//==============================================================================

template <typename DELEGATE>
dds::core::TKeyedBytesTopicType<DELEGATE>::TKeyedBytesTopicType() :
    dds::core::Value<DELEGATE>(std::string(), std::vector<uint8_t>())
{
}

template <typename DELEGATE>
dds::core::TKeyedBytesTopicType<DELEGATE>::TKeyedBytesTopicType(const std::string& key, const std::vector<uint8_t>& value) :
    dds::core::Value<DELEGATE>(key, value)
{
}

template <typename DELEGATE>
const std::string& dds::core::TKeyedBytesTopicType<DELEGATE>::key() const
{
    return this->delegate().key();
}

template <typename DELEGATE>
void dds::core::TKeyedBytesTopicType<DELEGATE>::key(const std::string& key)
{
    this->delegate().key(key);
}

template <typename DELEGATE>
const std::vector<uint8_t>& dds::core::TKeyedBytesTopicType<DELEGATE>::value() const
{
    return this->delegate().value();
}

template <typename DELEGATE>
void dds::core::TKeyedBytesTopicType<DELEGATE>::value(const std::vector<uint8_t>& value)
{
    this->delegate().value(value);
}



//==============================================================================
//            KeyedString Template
//==============================================================================

template <typename DELEGATE>
dds::core::TKeyedStringTopicType<DELEGATE>::TKeyedStringTopicType() :
    dds::core::Value<DELEGATE>(std::string(), std::string())
{
}

template <typename DELEGATE>
dds::core::TKeyedStringTopicType<DELEGATE>::TKeyedStringTopicType(const std::string& key, const std::string& value) :
    dds::core::Value<DELEGATE>(key, value)
{
}

template <typename DELEGATE>
const std::string& dds::core::TKeyedStringTopicType<DELEGATE>::key() const
{
    return this->delegate().key();
}

template <typename DELEGATE>
void dds::core::TKeyedStringTopicType<DELEGATE>::key(const std::string& key)
{
    this->delegate().key(key);
}

template <typename DELEGATE>
const std::string& dds::core::TKeyedStringTopicType<DELEGATE>::value() const
{
    return this->delegate().value();
}

template <typename DELEGATE>
void dds::core::TKeyedStringTopicType<DELEGATE>::value(const std::string& value)
{
    this->delegate().value(value);
}


#endif /* OMG_DDS_X_TYPES_BUILTIN_TOPIC_TYPES_SUPPORT */

#endif /* OMG_DDS_CORE_DELEGATE_BUILTIN_TOPIC_TYPES_HPP_ */
