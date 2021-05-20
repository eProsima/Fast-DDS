// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TypeSupport.hpp
 */

#ifndef _FASTDDS_TYPE_SUPPORT_HPP_
#define _FASTDDS_TYPE_SUPPORT_HPP_

#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastrtps/types/DynamicPubSubType.h>
#include <fastrtps/types/TypesBase.h>

#include <string>
#include <functional>
#include <memory>

namespace eprosima {
namespace fastdds {
namespace dds {

//! Handle to identiy different instances of the same Topic of a certain type.
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;

//! The NIL instance handle.
extern RTPS_DllAPI const InstanceHandle_t HANDLE_NIL;

class DomainParticipant;

/**
 * @note This class inherits from std::shared_ptr<TopicDataType>.
 * @brief Class TypeSupport used to provide the DomainRTPSParticipant with the methods to serialize,
 * deserialize and get the key of a specific data type.
 * The user should created a class that inherits from this one,
 * where Serialize and deserialize methods MUST be implemented.
 * @ingroup FASTDDS_MODULE
 */
class TypeSupport : public std::shared_ptr<fastdds::dds::TopicDataType>
{
public:

    using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

    using Base = std::shared_ptr<fastdds::dds::TopicDataType>;
    using Base::operator ->;
    using Base::operator *;
    using Base::operator bool;

    /**
     * @brief Constructor
     */
    RTPS_DllAPI TypeSupport() noexcept = default;

    /**
     * @brief Copy Constructor
     * @param type Another instance of TypeSupport
     */
    RTPS_DllAPI TypeSupport(
            const TypeSupport& type) noexcept = default;

    /**
     * @brief Move Constructor
     * @param type Another instance of TypeSupport
     */
    RTPS_DllAPI TypeSupport(
            TypeSupport&& type) noexcept = default;

    /**
     * @brief Copy Assignment
     * @param type Another instance of TypeSupport
     */
    RTPS_DllAPI TypeSupport& operator = (
            const TypeSupport& type) noexcept = default;

    /**
     * @brief Move Assignment
     * @param type Another instance of TypeSupport
     */
    RTPS_DllAPI TypeSupport& operator = (
            TypeSupport&& type) noexcept = default;

    /*!
     * \brief TypeSupport constructor that receives a TopicDataType pointer.
     * The passed pointer will be managed by the TypeSupport object, so creating two TypeSupport
     * from the same pointer or deleting the passed pointer will produce a runtime error.
     * \param ptr
     */
    RTPS_DllAPI explicit TypeSupport(
            fastdds::dds::TopicDataType* ptr)
        : std::shared_ptr<fastdds::dds::TopicDataType>(ptr)
    {
    }

    /*!
     * \brief TypeSupport constructor that receives a DynamicPubSubType.
     * It will copy the instance so the user will keep the ownership of his object.
     * \param ptr
     */
    RTPS_DllAPI TypeSupport(
            fastrtps::types::DynamicPubSubType ptr)
        : std::shared_ptr<fastdds::dds::TopicDataType>(std::make_shared<fastrtps::types::DynamicPubSubType>(std::move(
                    ptr)))
    {
    }

    /**
     * @brief Registers the type on a participant
     * @param participant DomainParticipant where the type is going to be registered
     * @return RETCODE_BAD_PARAMETER if the type name is empty, RETCODE_PRECONDITION_NOT_MET if there is another type with
     * the same name registered on the DomainParticipant and RETCODE_OK if it is registered correctly
     */
    RTPS_DllAPI virtual ReturnCode_t register_type(
            DomainParticipant* participant) const;

    /**
     * @brief Registers the type on a participant
     * @param participant DomainParticipant where the type is going to be registered
     * @param type_name Name of the type to register
     * @return RETCODE_BAD_PARAMETER if the type name is empty, RETCODE_PRECONDITION_NOT_MET if there is another type with
     * the same name registered on the DomainParticipant and RETCODE_OK if it is registered correctly
     */
    RTPS_DllAPI virtual ReturnCode_t register_type(
            DomainParticipant* participant,
            std::string type_name) const;

    /**
     * @brief Getter for the type name
     * @return name of the data type
     */
    RTPS_DllAPI virtual const std::string& get_type_name() const
    {
        return get()->m_topicDataTypeName;
    }

    /**
     * @brief Serializes the data
     * @param data Pointer to data
     * @param payload Pointer to payload
     * @return true if it is serialized correctly, false if not
     */
    RTPS_DllAPI virtual bool serialize(
            void* data,
            fastrtps::rtps::SerializedPayload_t* payload);

    /**
     * @brief Deserializes the data
     * @param payload Pointer to payload
     * @param data Pointer to data
     * @return true if it is deserialized correctly, false if not
     */
    RTPS_DllAPI virtual bool deserialize(
            fastrtps::rtps::SerializedPayload_t* payload,
            void* data);

    /**
     * @brief Getter for the SerializedSizeProvider
     * @param data Pointer to data
     * @return function
     */
    RTPS_DllAPI virtual std::function<uint32_t()> get_serialized_size_provider(
            void* data)
    {
        return get()->getSerializedSizeProvider(data);
    }

    /**
     * @brief Creates new data
     * @return Pointer to the data
     */
    RTPS_DllAPI virtual void* create_data()
    {
        return get()->createData();
    }

    /**
     * @brief Deletes data
     * @param data Pointer to the data to delete
     */
    RTPS_DllAPI virtual void delete_data(
            void* data)
    {
        return get()->deleteData(data);
    }

    /**
     * @brief Getter for the data key
     * @param data Pointer to data
     * @param i_handle InstanceHandle pointer to store the key
     * @param force_md5 boolean to force md5 (default: false)
     * @return true if the key is returned, false if not
     */
    RTPS_DllAPI virtual bool get_key(
            void* data,
            InstanceHandle_t* i_handle,
            bool force_md5 = false)
    {
        return get()->getKey(data, i_handle, force_md5);
    }

    RTPS_DllAPI virtual bool operator ==(
            const TypeSupport& type_support)
    {
        return get()->m_typeSize == type_support->m_typeSize
               && get()->m_isGetKeyDefined == type_support->m_isGetKeyDefined
               && get()->m_topicDataTypeName == type_support->m_topicDataTypeName
               && get()->type_identifier() == type_support->type_identifier()
               && get()->type_information() == type_support->type_information()
               && get()->type_object() == type_support->type_object();
    }

    /**
     * @brief Check if the TypeSupport is empty
     * @return true if empty, false if not
     */
    RTPS_DllAPI bool empty() const
    {
        return get() == nullptr;
    }

    /**
     * Checks if the type is bounded.
     */
    RTPS_DllAPI virtual inline bool is_bounded() const
    {
        return get()->is_bounded();
    }

    /**
     * Checks if the type is plain.
     */
    RTPS_DllAPI virtual inline bool is_plain() const
    {
        return get()->is_plain();
    }

    RTPS_DllAPI bool operator !=(
            std::nullptr_t) const
    {
        return bool(*this);
    }

    RTPS_DllAPI bool operator ==(
            std::nullptr_t) const
    {
        return !*this;
    }

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_TYPE_SUPPORT_HPP_ */
