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
 * @file TopicDataType.hpp
 */

#ifndef FASTDDS_DDS_TOPIC__TOPICDATATYPE_HPP
#define FASTDDS_DDS_TOPIC__TOPICDATATYPE_HPP

#include <functional>
#include <memory>
#include <string>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobject.hpp>
#include <fastdds/fastdds_dll.hpp>
#include <fastdds/rtps/common/CdrSerialization.hpp>
#include <fastdds/rtps/common/InstanceHandle.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/utils/md5.hpp>

// This version of TypeSupport has `is_bounded()`
#define TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

// This version of TypeSupport has `is_plain()`
#define TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

// This version of TypeSupport has `construct_sample()`
#define TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

namespace eprosima {
namespace fastdds {

namespace rtps {
struct SerializedPayload_t;
struct InstanceHandle_t;
} // namespace rtps

namespace dds {

class TypeSupport;

/**
 * Class TopicDataType used to provide the DomainRTPSParticipant with the methods to serialize, deserialize and get the key of a specific data type.
 * The user should created a class that inherits from this one, where Serialize and deserialize methods MUST be implemented.
 * @ingroup FASTDDS_MODULE
 */
class TopicDataType
{
public:

    /**
     * @brief Constructor
     */
    FASTDDS_EXPORTED_API TopicDataType() = default;

    /**
     * @brief Destructor
     */
    FASTDDS_EXPORTED_API virtual ~TopicDataType() = default;

    /**
     * Serialize method, it should be implemented by the user, since it is abstract. If not implemented, this method
     * will call the serialize method in which the topic data representation is not considered.
     * It is VERY IMPORTANT that the user sets the SerializedPayload length correctly.
     *
     * @param [in] data Pointer to the data
     * @param [out] payload Pointer to the payload
     * @param [in] data_representation Representation that should be used to encode the data into the payload.
     * @return True if correct.
     */
    FASTDDS_EXPORTED_API virtual bool serialize(
            const void* const data,
            rtps::SerializedPayload_t& payload,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) = 0;

    /**
     * Deserialize method, it should be implemented by the user, since it is abstract.
     *
     * @param [in] payload Pointer to the payload
     * @param [out] data Pointer to the data
     * @return True if correct.
     */
    FASTDDS_EXPORTED_API virtual bool deserialize(
            rtps::SerializedPayload_t& payload,
            void* data) = 0;

    /*!
     * @brief Calculates the serialized size of the provided data.
     *
     * @param [in] data Pointer to data.
     * @param [in] data_representation Representation that should be used for calculating the serialized size.
     * @return Serialized size of the data.
     */
    FASTDDS_EXPORTED_API virtual uint32_t calculate_serialized_size(
            const void* const data,
            eprosima::fastdds::dds::DataRepresentationId_t data_representation) = 0;

    /**
     * Create a Data Type.
     *
     * @return Void pointer to the created object.
     */
    FASTDDS_EXPORTED_API virtual void* create_data() = 0;

    /**
     * Remove a previously created object.
     *
     * @param data Pointer to the created Data.
     */
    FASTDDS_EXPORTED_API virtual void delete_data(
            void* data) = 0;

    /**
     * Get the key associated with the data.
     *
     * @param [in] payload Pointer to the payload containing the data.
     * @param [out] ihandle Pointer to the Handle.
     * @param [in] force_md5 Force MD5 checking.
     * @return True if correct.
     */
    FASTDDS_EXPORTED_API virtual bool compute_key(
            rtps::SerializedPayload_t& payload,
            rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) = 0;

    /**
     * Get the key associated with the data.
     *
     * @param [in] data Pointer to the data.
     * @param [out] ihandle Pointer to the Handle.
     * @param [in] force_md5 Force MD5 checking.
     * @return True if correct.
     */
    FASTDDS_EXPORTED_API virtual bool compute_key(
            const void* const data,
            rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) = 0;

    /**
     * Set topic data type name
     *
     * @param nam Topic data type name
     */
    FASTDDS_EXPORTED_API inline void set_name(
            const std::string& nam)
    {
        topic_data_typename_ = nam;
    }

    /**
     * Set topic data type name
     *
     * @param nam Topic data type name
     */
    FASTDDS_EXPORTED_API inline void set_name(
            std::string&& nam)
    {
        topic_data_typename_ = std::move(nam);
    }

    /**
     * Get topic data type name
     *
     * @return Topic data type name
     */
    FASTDDS_EXPORTED_API inline const std::string& get_name() const
    {
        return topic_data_typename_;
    }

    /**
     * Get the type identifiers
     *
     * @return @ref xtypes::TypeIdentifierPair
     */
    FASTDDS_EXPORTED_API inline const xtypes::TypeIdentifierPair& type_identifiers() const
    {
        return type_identifiers_;
    }

    /**
     * Checks if the type is bounded.
     */
    FASTDDS_EXPORTED_API virtual inline bool is_bounded() const
    {
        return false;
    }

    /**
     * Checks if the type is plain when using a specific encoding.
     */
    FASTDDS_EXPORTED_API virtual inline bool is_plain(
            DataRepresentationId_t) const
    {
        return false;
    }

    /**
     * Construct a sample on a memory location.
     *
     * @param memory Pointer to the memory location where the sample should be constructed.
     *
     * @return whether this type supports in-place construction or not.
     */
    FASTDDS_EXPORTED_API virtual inline bool construct_sample(
            void* memory) const
    {
        static_cast<void>(memory);
        return false;
    }

    /**
     * @brief Register TypeObject type representation
     */
    FASTDDS_EXPORTED_API virtual inline void register_type_object_representation()
    {
    }

    //! Maximum serialized size of the type in bytes.
    //! If the type has unbounded fields, and therefore cannot have a maximum size, use 0.
    uint32_t max_serialized_type_size {0};

    //! Indicates whether the method to obtain the key has been implemented.
    bool is_compute_key_provided {false};

protected:

    xtypes::TypeIdentifierPair type_identifiers_;

private:

    //! Data Type Name.
    std::string topic_data_typename_;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_TOPIC__TOPICDATATYPE_HPP
