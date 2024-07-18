// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICPUBSUBTYPE_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICPUBSUBTYPE_HPP

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastdds/utils/md5.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicType;
class DynamicData;

class DynamicPubSubType : public virtual eprosima::fastdds::dds::TopicDataType
{
    traits<DynamicType>::ref_type dynamic_type_;

    unsigned char* key_buffer_ {nullptr};

    MD5 md5_;

public:

    //{{{ Public functions

    FASTDDS_EXPORTED_API DynamicPubSubType() = default;

    /*
     * Constructs a @ref DynamicPubSubType from a @ref DynamicType
     * @param type @ref DynamicType object associated to the data
     * @remark Ownership is not transferred.
     */
    FASTDDS_EXPORTED_API DynamicPubSubType(
            traits<DynamicType>::ref_type type);

    FASTDDS_EXPORTED_API virtual ~DynamicPubSubType();

    /*
     * Create a new data object of the specified type
     * @return pointer to the new object
     * @remark Ownership is transferred. This object must be removed using @ref deleteData
     */
    FASTDDS_EXPORTED_API void* create_data() override;

    /*
     * Deletes an object previously allocated via @ref createData
     * @param data pointer to the object to be deleted
     * @remark Ownership is transferred. This object must be allocated using @ref createData
     */
    FASTDDS_EXPORTED_API void delete_data (
            void* data) override;

    /*
     * Deserialize an object from the given payload
     * @param payload @ref eprosima::fastdds::rtps::SerializedPayload_t to parse
     * @param data object to fill in with payload data
     * @return bool specifying success
     */
    FASTDDS_EXPORTED_API bool deserialize (
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            void* data) override;

    /*
     * Returns a copy of the internal  @ref DynamicType object
     * @return pointer to the new object
     */
    FASTDDS_EXPORTED_API traits<DynamicType>::ref_type get_dynamic_type() const noexcept;

    /*
     * Calculate the key associated to a given object
     * @param data payload containing the serialized object whose key is calculated
     * @param ihandle @ref eprosima::fastdds::rtps::InstanceHandle_t to fill in
     * @param force_md5 use always md5 even if key payload footprint is smaller than the hash
     * @return bool specifying success
     */
    FASTDDS_EXPORTED_API bool compute_key(
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            eprosima::fastdds::rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override;

    /*
     * Calculate the key associated to a given object
     * @param data object whose key is calculated
     * @param ihandle @ref eprosima::fastdds::rtps::InstanceHandle_t to fill in
     * @param force_md5 use always md5 even if key payload footprint is smaller than the hash
     * @return bool specifying success
     */
    FASTDDS_EXPORTED_API bool compute_key(
            const void* const data,
            eprosima::fastdds::rtps::InstanceHandle_t& ihandle,
            bool force_md5 = false) override;

    /*
     * Provide a functor that calculates a specified object serialized size
     * @param[in] data object whose payload footprint to calculate
     * @param [in] data_representation Representation that should be used for calculating the serialized size.
     * @return functor that calculates the size
     */
    FASTDDS_EXPORTED_API uint32_t calculate_serialized_size(
            const void* const data,
            DataRepresentationId_t data_representation) override;

    /*
     * Serialize an object into a given payload
     * @param[in] data object to serialize
     * @param[out] payload @ref eprosima::fastdds::rtps::SerializedPayload_t to fill in
     * @param [in] data_representation Representation that should be used to encode the data into the payload.
     * @return bool specifying success
     */
    FASTDDS_EXPORTED_API bool serialize(
            const void* const data,
            eprosima::fastdds::rtps::SerializedPayload_t& payload,
            fastdds::dds::DataRepresentationId_t data_representation) override;

    /*
     * Sets up the internal @ref DynamicType object
     * @param @ref DynamicType to copy
     * @return @ref ReturnCode_t with operation status
     * @remark Ownership is not transferred.
     */
    FASTDDS_EXPORTED_API ReturnCode_t set_dynamic_type(
            traits<DynamicType>::ref_type type);

    //Register TypeObject representation in Fast DDS TypeObjectRegistry
    FASTDDS_EXPORTED_API void register_type_object_representation() override;

    //}}}

private:

    void update_dynamic_type();
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES__DYNAMICPUBSUBTYPE_HPP
