// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file rpc_typesPubSubTypes.h
 * This header file contains the declaration of the serialization functions.
 *
 * This file was generated by the tool fastddsgen.
 */


#ifndef _FAST_DDS_GENERATED_EPROSIMA_FASTDDS_DDS_RPC_RPC_TYPES_PUBSUBTYPES_H_
#define _FAST_DDS_GENERATED_EPROSIMA_FASTDDS_DDS_RPC_RPC_TYPES_PUBSUBTYPES_H_

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/rtps/common/InstanceHandle.h>
#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastdds/utils/md5.h>

#include "rpc_types.hpp"


#if !defined(GEN_API_VER) || (GEN_API_VER != 2)
#error \
    Generated rpc_types is not compatible with current installed Fast DDS. Please, regenerate it with fastddsgen.
#endif  // GEN_API_VER

namespace eprosima {

namespace fastdds {

namespace dds {


typedef std::array<uint8_t, 12> GuidPrefix_t;

/*!
 * @brief This class represents the TopicDataType of the type EntityId_t defined by the user in the IDL file.
 * @ingroup rpc_types
 */
class EntityId_tPubSubType : public eprosima::fastdds::dds::TopicDataType
{
public:

    typedef EntityId_t type;

    eProsima_user_DllExport EntityId_tPubSubType();

    eProsima_user_DllExport ~EntityId_tPubSubType() override;

    eProsima_user_DllExport bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override
    {
        return serialize(data, payload, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
    }

    eProsima_user_DllExport bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    eProsima_user_DllExport bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    eProsima_user_DllExport std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override
    {
        return getSerializedSizeProvider(data, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
    }

    eProsima_user_DllExport std::function<uint32_t()> getSerializedSizeProvider(
            void* data,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    eProsima_user_DllExport bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) override;

    eProsima_user_DllExport void* createData() override;

    eProsima_user_DllExport void deleteData(
            void* data) override;

    //Register TypeObject representation in Fast DDS TypeObjectRegistry
    eProsima_user_DllExport void register_type_object_representation() const override;

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
    eProsima_user_DllExport inline bool is_bounded() const override
    {
        return true;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
    eProsima_user_DllExport inline bool is_plain() const override
    {
        return false;
    }

    eProsima_user_DllExport inline bool is_plain(
        eprosima::fastdds::dds::DataRepresentationId_t data_representation) const override
    {
        static_cast<void>(data_representation);
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

#ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    eProsima_user_DllExport inline bool construct_sample(
            void* memory) const override
    {
        static_cast<void>(memory);
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

    MD5 m_md5;
    unsigned char* m_keyBuffer;

};

/*!
 * @brief This class represents the TopicDataType of the type GUID_t defined by the user in the IDL file.
 * @ingroup rpc_types
 */
class GUID_tPubSubType : public eprosima::fastdds::dds::TopicDataType
{
public:

    typedef GUID_t type;

    eProsima_user_DllExport GUID_tPubSubType();

    eProsima_user_DllExport ~GUID_tPubSubType() override;

    eProsima_user_DllExport bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override
    {
        return serialize(data, payload, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
    }

    eProsima_user_DllExport bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    eProsima_user_DllExport bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    eProsima_user_DllExport std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override
    {
        return getSerializedSizeProvider(data, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
    }

    eProsima_user_DllExport std::function<uint32_t()> getSerializedSizeProvider(
            void* data,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    eProsima_user_DllExport bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) override;

    eProsima_user_DllExport void* createData() override;

    eProsima_user_DllExport void deleteData(
            void* data) override;

    //Register TypeObject representation in Fast DDS TypeObjectRegistry
    eProsima_user_DllExport void register_type_object_representation() const override;

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
    eProsima_user_DllExport inline bool is_bounded() const override
    {
        return true;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
    eProsima_user_DllExport inline bool is_plain() const override
    {
        return false;
    }

    eProsima_user_DllExport inline bool is_plain(
        eprosima::fastdds::dds::DataRepresentationId_t data_representation) const override
    {
        static_cast<void>(data_representation);
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

#ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    eProsima_user_DllExport inline bool construct_sample(
            void* memory) const override
    {
        static_cast<void>(memory);
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

    MD5 m_md5;
    unsigned char* m_keyBuffer;

};

/*!
 * @brief This class represents the TopicDataType of the type SequenceNumber_t defined by the user in the IDL file.
 * @ingroup rpc_types
 */
class SequenceNumber_tPubSubType : public eprosima::fastdds::dds::TopicDataType
{
public:

    typedef SequenceNumber_t type;

    eProsima_user_DllExport SequenceNumber_tPubSubType();

    eProsima_user_DllExport ~SequenceNumber_tPubSubType() override;

    eProsima_user_DllExport bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override
    {
        return serialize(data, payload, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
    }

    eProsima_user_DllExport bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    eProsima_user_DllExport bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    eProsima_user_DllExport std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override
    {
        return getSerializedSizeProvider(data, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
    }

    eProsima_user_DllExport std::function<uint32_t()> getSerializedSizeProvider(
            void* data,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    eProsima_user_DllExport bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) override;

    eProsima_user_DllExport void* createData() override;

    eProsima_user_DllExport void deleteData(
            void* data) override;

    //Register TypeObject representation in Fast DDS TypeObjectRegistry
    eProsima_user_DllExport void register_type_object_representation() const override;

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
    eProsima_user_DllExport inline bool is_bounded() const override
    {
        return true;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
    eProsima_user_DllExport inline bool is_plain() const override
    {
        return false;
    }

    eProsima_user_DllExport inline bool is_plain(
        eprosima::fastdds::dds::DataRepresentationId_t data_representation) const override
    {
        static_cast<void>(data_representation);
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

#ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    eProsima_user_DllExport inline bool construct_sample(
            void* memory) const override
    {
        static_cast<void>(memory);
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

    MD5 m_md5;
    unsigned char* m_keyBuffer;

};

/*!
 * @brief This class represents the TopicDataType of the type SampleIdentity defined by the user in the IDL file.
 * @ingroup rpc_types
 */
class SampleIdentityPubSubType : public eprosima::fastdds::dds::TopicDataType
{
public:

    typedef SampleIdentity type;

    eProsima_user_DllExport SampleIdentityPubSubType();

    eProsima_user_DllExport ~SampleIdentityPubSubType() override;

    eProsima_user_DllExport bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override
    {
        return serialize(data, payload, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
    }

    eProsima_user_DllExport bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    eProsima_user_DllExport bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    eProsima_user_DllExport std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override
    {
        return getSerializedSizeProvider(data, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
    }

    eProsima_user_DllExport std::function<uint32_t()> getSerializedSizeProvider(
            void* data,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

    eProsima_user_DllExport bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) override;

    eProsima_user_DllExport void* createData() override;

    eProsima_user_DllExport void deleteData(
            void* data) override;

    //Register TypeObject representation in Fast DDS TypeObjectRegistry
    eProsima_user_DllExport void register_type_object_representation() const override;

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
    eProsima_user_DllExport inline bool is_bounded() const override
    {
        return true;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
    eProsima_user_DllExport inline bool is_plain() const override
    {
        return false;
    }

    eProsima_user_DllExport inline bool is_plain(
        eprosima::fastdds::dds::DataRepresentationId_t data_representation) const override
    {
        static_cast<void>(data_representation);
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

#ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    eProsima_user_DllExport inline bool construct_sample(
            void* memory) const override
    {
        static_cast<void>(memory);
        return false;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

    MD5 m_md5;
    unsigned char* m_keyBuffer;

};
namespace rpc
{
    typedef uint8_t UnknownOperation;
    typedef uint8_t UnknownException;
    typedef uint8_t UnusedMember;

    typedef eprosima::fastcdr::fixed_string<255> InstanceName;

    /*!
     * @brief This class represents the TopicDataType of the type RequestHeader defined by the user in the IDL file.
     * @ingroup rpc_types
     */
    class RequestHeaderPubSubType : public eprosima::fastdds::dds::TopicDataType
    {
    public:

        typedef RequestHeader type;

        eProsima_user_DllExport RequestHeaderPubSubType();

        eProsima_user_DllExport ~RequestHeaderPubSubType() override;

        eProsima_user_DllExport bool serialize(
                void* data,
                eprosima::fastrtps::rtps::SerializedPayload_t* payload) override
        {
            return serialize(data, payload, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
        }

        eProsima_user_DllExport bool serialize(
                void* data,
                eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

        eProsima_user_DllExport bool deserialize(
                eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                void* data) override;

        eProsima_user_DllExport std::function<uint32_t()> getSerializedSizeProvider(
                void* data) override
        {
            return getSerializedSizeProvider(data, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
        }

        eProsima_user_DllExport std::function<uint32_t()> getSerializedSizeProvider(
                void* data,
                eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

        eProsima_user_DllExport bool getKey(
                void* data,
                eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
                bool force_md5 = false) override;

        eProsima_user_DllExport void* createData() override;

        eProsima_user_DllExport void deleteData(
                void* data) override;

        //Register TypeObject representation in Fast DDS TypeObjectRegistry
        eProsima_user_DllExport void register_type_object_representation() const override;

    #ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
        eProsima_user_DllExport inline bool is_bounded() const override
        {
            return true;
        }

    #endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

    #ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
        eProsima_user_DllExport inline bool is_plain() const override
        {
            return false;
        }

        eProsima_user_DllExport inline bool is_plain(
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) const override
        {
            static_cast<void>(data_representation);
            return false;
        }

    #endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

    #ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
        eProsima_user_DllExport inline bool construct_sample(
                void* memory) const override
        {
            static_cast<void>(memory);
            return false;
        }

    #endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

        MD5 m_md5;
        unsigned char* m_keyBuffer;

    };

    /*!
     * @brief This class represents the TopicDataType of the type ReplyHeader defined by the user in the IDL file.
     * @ingroup rpc_types
     */
    class ReplyHeaderPubSubType : public eprosima::fastdds::dds::TopicDataType
    {
    public:

        typedef ReplyHeader type;

        eProsima_user_DllExport ReplyHeaderPubSubType();

        eProsima_user_DllExport ~ReplyHeaderPubSubType() override;

        eProsima_user_DllExport bool serialize(
                void* data,
                eprosima::fastrtps::rtps::SerializedPayload_t* payload) override
        {
            return serialize(data, payload, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
        }

        eProsima_user_DllExport bool serialize(
                void* data,
                eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

        eProsima_user_DllExport bool deserialize(
                eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                void* data) override;

        eProsima_user_DllExport std::function<uint32_t()> getSerializedSizeProvider(
                void* data) override
        {
            return getSerializedSizeProvider(data, eprosima::fastdds::dds::DEFAULT_DATA_REPRESENTATION);
        }

        eProsima_user_DllExport std::function<uint32_t()> getSerializedSizeProvider(
                void* data,
                eprosima::fastdds::dds::DataRepresentationId_t data_representation) override;

        eProsima_user_DllExport bool getKey(
                void* data,
                eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
                bool force_md5 = false) override;

        eProsima_user_DllExport void* createData() override;

        eProsima_user_DllExport void deleteData(
                void* data) override;

        //Register TypeObject representation in Fast DDS TypeObjectRegistry
        eProsima_user_DllExport void register_type_object_representation() const override;

    #ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
        eProsima_user_DllExport inline bool is_bounded() const override
        {
            return false;
        }

    #endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

    #ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
        eProsima_user_DllExport inline bool is_plain() const override
        {
            return false;
        }

        eProsima_user_DllExport inline bool is_plain(
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) const override
        {
            static_cast<void>(data_representation);
            return false;
        }

    #endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

    #ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
        eProsima_user_DllExport inline bool construct_sample(
                void* memory) const override
        {
            static_cast<void>(memory);
            return false;
        }

    #endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

        MD5 m_md5;
        unsigned char* m_keyBuffer;

    };
} // namespace rpc

} // namespace dds

} // namespace fastdds

} // namespace eprosima


#endif // _FAST_DDS_GENERATED_EPROSIMA_FASTDDS_DDS_RPC_RPC_TYPES_PUBSUBTYPES_H_
