/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 * All rights reserved.
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

#ifndef OMG_DDS_CORE_TBUILTIN_TOPIC_TYPES_HPP_
#define OMG_DDS_CORE_TBUILTIN_TOPIC_TYPES_HPP_


#if defined (OMG_DDS_X_TYPES_BUILTIN_TOPIC_TYPES_SUPPORT)
namespace dds
{
namespace core
{

template<typename DELEGATE>
class TBytesTopicType;

template<typename DELEGATE>
class TStringTopicType;

template<typename DELEGATE>
class TKeyedBytesTopicType;

template<typename DELEGATE>
class TKeyedStringTopicType;
}
}


/**
 * @brief
 * Class that is a built-in topic type that can be used to readily create Topics,
 * DataReaders and DataWriters for this type without the need for code generation.
 *
 * This built-in type allows for easy transfer of vectors of bytes.
 */
template<typename DELEGATE>
class dds::core::TBytesTopicType : public ::dds::core::Value<DELEGATE>
{
public:
    /**
     * Creates topic type with an empty byte vector.
     */
    TBytesTopicType();

    /**
     * Creates topic type with the given byte vector.
     */
    TBytesTopicType(const std::vector<uint8_t>& data);

    /**
     * Conversion operator to a vector of bytes.
     */
    operator std::vector<uint8_t>& () const;

    /**
     * Getter function for the internal vector of bytes.
     */
    const std::vector<uint8_t>& data() const;

    /**
     * Setter function for the internal vector of bytes.
     */
    void data(const std::vector<uint8_t>& data);
};


/**
 * @brief
 * Class that is a built-in topic type that can be used to readily create Topics,
 * DataReaders and DataWriters for this type without the need for code generation.
 *
 * This built-in type allows for easy transfer of strings.
 */
template<typename DELEGATE>
class dds::core::TStringTopicType : public ::dds::core::Value<DELEGATE>
{
public:
    /**
     * Creates topic type with an empty data string.
     */
    TStringTopicType();

    /**
     * Creates topic type with the given string.
     */
    TStringTopicType(const std::string& data);

    /**
     * Conversion operator to a string.
     */
    operator std::string& () const;

    /**
     * Getter function for the internal data string.
     */
    const std::string& data() const;

    /**
     * Setter function for the internal data string.
     */
    void data(const std::string& data);
};


/**
 * @brief
 * Class that is a built-in topic type that can be used to readily create Topics,
 * DataReaders and DataWriters for this type without the need for code generation.
 *
 * This built-in type allows for easy transfer of keyed strings.
 */
template<typename DELEGATE>
class dds::core::TKeyedStringTopicType : public ::dds::core::Value<DELEGATE>
{
public:
    /**
     * Creates topic type with an empty key and data strings.
     */
    TKeyedStringTopicType();

    /**
     * Creates topic type with the given key and data strings.
     */
    TKeyedStringTopicType(const std::string& key, const std::string& value);

    /**
     * Getter function for the key string.
     */
    const std::string& key() const;

    /**
     * Setter function for the key string.
     */
    void key(const std::string& key);

    /**
     * Getter function for the internal data string.
     */
    const std::string& value() const;

    /**
     * Setter function for the internal data string.
     */
    void value(const std::string& value);
};


/**
 * @brief
 * Class that is a built-in topic type that can be used to readily create Topics,
 * DataReaders and DataWriters for this type without the need for code generation.
 *
 * This built-in type allows for easy transfer of keyed vectors of bytes.
 */
template<typename DELEGATE>
class dds::core::TKeyedBytesTopicType : public ::dds::core::Value<DELEGATE>
{
public:
    /**
     * Creates topic type with an empty key string and data vector.
     */
    TKeyedBytesTopicType();

    /**
     * Creates topic type with given key string and data vector.
     */
    TKeyedBytesTopicType(const std::string& key, const std::vector<uint8_t>& value);

    /**
     * Getter function for the key string.
     */
    const std::string& key() const;

    /**
     * Setter function for the key string.
     */
    void key(const std::string& key);

    /**
     * Getter function for the internal vector of bytes.
     */
    const std::vector<uint8_t>& value() const;

    /**
     * Setter function for the internal vector of bytes.
     */
    void value(const std::vector<uint8_t>& value);
};


#endif /* OMG_DDS_X_TYPES_BUILTIN_TOPIC_TYPES_SUPPORT */

#endif /* OMG_DDS_CORE_TBUILTIN_TOPIC_TYPES_HPP_ */
