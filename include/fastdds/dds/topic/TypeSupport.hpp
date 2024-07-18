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

#ifndef FASTDDS_DDS_TOPIC__TYPESUPPORT_HPP
#define FASTDDS_DDS_TOPIC__TYPESUPPORT_HPP

#include <string>
#include <functional>
#include <memory>

#include <fastdds/dds/common/InstanceHandle.hpp>
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipant;

/**
 * @note This class inherits from std::shared_ptr<TopicDataType>.
 * @brief Class TypeSupport used to provide the DomainRTPSParticipant with the methods to serialize,
 * deserialize and get the key of a specific data type.
 * The user should created a class that inherits from this one,
 * where Serialize and deserialize methods MUST be implemented.
 * @ingroup FASTDDS_MODULE
 */
class TypeSupport : public std::shared_ptr<TopicDataType>
{
public:

    using Base = std::shared_ptr<TopicDataType>;

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API TypeSupport() noexcept = default;

    /**
     * @brief Copy Constructor
     *
     * @param type Another instance of TypeSupport
     */
    FASTDDS_EXPORTED_API TypeSupport(
            const TypeSupport& type) noexcept = default;

    /**
     * @brief Move Constructor
     *
     * @param type Another instance of TypeSupport
     */
    FASTDDS_EXPORTED_API TypeSupport(
            TypeSupport&& type) noexcept = default;

    /**
     * @brief Copy Assignment
     *
     * @param type Another instance of TypeSupport
     */
    FASTDDS_EXPORTED_API TypeSupport& operator = (
            const TypeSupport& type) noexcept = default;

    /**
     * @brief Move Assignment
     *
     * @param type Another instance of TypeSupport
     */
    FASTDDS_EXPORTED_API TypeSupport& operator = (
            TypeSupport&& type) noexcept = default;

    /*!
     * @brief TypeSupport constructor that receives a TopicDataType pointer.
     * The passed pointer will be managed by the TypeSupport object, so creating two TypeSupport
     * from the same pointer or deleting the passed pointer will produce a runtime error.
     *
     * @param ptr
     */
    FASTDDS_EXPORTED_API explicit TypeSupport(
            TopicDataType* ptr)
        : std::shared_ptr<TopicDataType>(ptr)
    {
    }

    /**
     * @brief Registers the type on a participant
     *
     * @param participant DomainParticipant where the type is going to be registered
     * @return RETCODE_BAD_PARAMETER if the type name is empty, RETCODE_PRECONDITION_NOT_MET if there is another type with
     * the same name registered on the DomainParticipant and RETCODE_OK if it is registered correctly
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t register_type(
            DomainParticipant* participant) const;

    /**
     * @brief Registers the type on a participant
     *
     * @param participant DomainParticipant where the type is going to be registered
     * @param type_name Name of the type to register
     * @return RETCODE_BAD_PARAMETER if the type name is empty, RETCODE_PRECONDITION_NOT_MET if there is another type with
     * the same name registered on the DomainParticipant and RETCODE_OK if it is registered correctly
     */
    FASTDDS_EXPORTED_API virtual ReturnCode_t register_type(
            DomainParticipant* participant,
            std::string type_name) const;

    /**
     * @brief Getter for the type name
     *
     * @return name of the data type
     */
    FASTDDS_EXPORTED_API virtual const std::string& get_type_name() const
    {
        return get()->get_name();
    }

    /**
     * @brief Serializes the data
     *
     * @param data Pointer to data
     * @param payload Pointer to payload
     * @param [in] data_representation Representation that should be used to encode the data into the payload.
     * @return true if it is serialized correctly, false if not
     */
    FASTDDS_EXPORTED_API virtual bool serialize(
            const void* const data,
            fastdds::rtps::SerializedPayload_t& payload,
            DataRepresentationId_t data_representation);

    /**
     * @brief Deserializes the data
     *
     * @param payload Pointer to payload
     * @param data Pointer to data
     * @return true if it is deserialized correctly, false if not
     */
    FASTDDS_EXPORTED_API virtual bool deserialize(
            fastdds::rtps::SerializedPayload_t& payload,
            void* data);

    /*!
     * @brief Returns a function which can be used to calculate the serialized size of the provided data.
     *
     * @param [in] data Pointer to data.
     * @param [in] data_representation Representation that should be used for calculating the serialized size.
     * @return Functor which calculates the serialized size of the data.
     */
    FASTDDS_EXPORTED_API virtual uint32_t calculate_serialized_size(
            const void* const data,
            DataRepresentationId_t data_representation)
    {
        return get()->calculate_serialized_size(data, data_representation);
    }

    /**
     * @brief Creates new data
     *
     * @return Pointer to the data
     */
    FASTDDS_EXPORTED_API virtual void* create_data()
    {
        return get()->create_data();
    }

    /**
     * @brief Deletes data
     *
     * @param data Pointer to the data to delete
     */
    FASTDDS_EXPORTED_API virtual void delete_data(
            void* data)
    {
        return get()->delete_data(data);
    }

    /**
     * @brief Getter for the data key
     *
     * @param data Pointer to serialized payload containing the data.
     * @param i_handle InstanceHandle pointer to store the key
     * @param force_md5 boolean to force md5 (default: false)
     * @return true if the key is returned, false if not
     */
    FASTDDS_EXPORTED_API virtual bool compute_key(
            const void* const data,
            InstanceHandle_t& i_handle,
            bool force_md5 = false)
    {
        return get()->compute_key(data, i_handle, force_md5);
    }

    /**
     * @brief Getter for the data key
     *
     * @param payload Pointer to data
     * @param i_handle InstanceHandle pointer to store the key
     * @param force_md5 boolean to force md5 (default: false)
     * @return true if the key is returned, false if not
     */
    FASTDDS_EXPORTED_API virtual bool compute_key(
            fastdds::rtps::SerializedPayload_t& payload,
            InstanceHandle_t& i_handle,
            bool force_md5 = false)
    {
        return get()->compute_key(payload, i_handle, force_md5);
    }

    FASTDDS_EXPORTED_API virtual bool operator ==(
            const TypeSupport& type_support)
    {
        return get()->max_serialized_type_size == type_support->max_serialized_type_size
               && get()->is_compute_key_provided == type_support->is_compute_key_provided
               && get()->get_name() == type_support->get_name()
               && get()->type_identifiers() == type_support->type_identifiers();
    }

    /**
     * @brief Check if the TypeSupport is empty
     *
     * @return true if empty, false if not
     */
    FASTDDS_EXPORTED_API bool empty() const
    {
        return get() == nullptr;
    }

    /**
     * Checks if the type is bounded.
     */
    FASTDDS_EXPORTED_API virtual inline bool is_bounded() const
    {
        return get()->is_bounded();
    }

    /**
     * Checks if the type is plain when using a specific encoding.
     */
    FASTDDS_EXPORTED_API virtual inline bool is_plain(
            DataRepresentationId_t data_representation) const
    {
        return get()->is_plain(data_representation);
    }

    FASTDDS_EXPORTED_API bool operator !=(
            std::nullptr_t) const
    {
        return bool(*this);
    }

    FASTDDS_EXPORTED_API bool operator ==(
            std::nullptr_t) const
    {
        return !*this;
    }

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_TOPIC__TYPESUPPORT_HPP
