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

#ifndef FASTDDS_DDS_XTYPES_DYNAMIC_TYPES_DYNAMIC_PUB_SUB_TYPE_HPP
#define FASTDDS_DDS_XTYPES_DYNAMIC_TYPES_DYNAMIC_PUB_SUB_TYPE_HPP

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/xtypes/Types.hpp>
#include <fastrtps/utils/md5.h>

namespace eprosima {
namespace fastdds {
namespace dds {

class DynamicType;
class DynamicData;

class DynamicPubSubType : public virtual eprosima::fastdds::dds::TopicDataType
{
protected:

    void UpdateDynamicTypeInfo();

    traits<DynamicType>::ref_type dynamic_type_;

    MD5 m_md5;

    unsigned char* m_keyBuffer = nullptr;

public:

    RTPS_DllAPI DynamicPubSubType() = default;

    /*
     * Constructs a @ref DynamicPubSubType from a @ref DynamicType
     * @param type @ref DynamicType object associated to the data
     * @remark Ownership is not transferred.
     */
    RTPS_DllAPI DynamicPubSubType(
            traits<DynamicType>::ref_type type);

    RTPS_DllAPI virtual ~DynamicPubSubType();

    /*
     * Create a new data object of the specified type
     * @return pointer to the new object
     * @remark Ownership is transferred. This object must be removed using @ref deleteData
     */
    RTPS_DllAPI void* createData() override;

    /*
     * Deletes an object previously allocated via @ref createData
     * @param data pointer to the object been deleted
     * @remark Ownership is transferred. This object must be allocated using @ref createData
     */
    RTPS_DllAPI void deleteData (
            void* data) override;

    /*
     * Deserialize an object from the given payload
     * @param payload @ref eprosima::fastrtps::rtps::SerializedPayload_t to parse
     * @param data object to fill in with payload data
     * @return bool specifying success
     * @remark Ownership is transferred. This object must be removed using @ref deleteData
     */
    RTPS_DllAPI bool deserialize (
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    /*
     * Calculate the key associated to a given object
     * @param data object whose key to calculate
     * @param ihandle @ref eprosima::fastrtps::rtps::InstanceHandle_t to fill in
     * @param force_md5 use always md5 even if key payload footprint is smaller than the hash
     * @return bool specifying success
     */
    RTPS_DllAPI bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) override;

    /*
     * Provide a functor that calculates a specified object serialized size
     * @param data object whose payload footprint to calculate
     * @return functor that calculates the size
     */
    RTPS_DllAPI std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;

    /*
     * Serialize an object into a given payload
     * @param data object to serialize
     * @param payload @ref eprosima::fastrtps::rtps::SerializedPayload_t to fill in
     * @return bool specifying success
     */
    RTPS_DllAPI bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override
    {
        return serialize(data, payload, fastdds::dds::DEFAULT_DATA_REPRESENTATION);
    }

    /*
     * Serialize an object into a given payload
     * @param data object to serialize
     * @param payload @ref eprosima::fastrtps::rtps::SerializedPayload_t to fill in
     * @return bool specifying success
     */
    RTPS_DllAPI bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            fastdds::dds::DataRepresentationId_t data_representation) override;

    /*
     * Returns a copy of the internal  @ref DynamicType object
     * @return pointer to the new object
     * @remark Ownership is transferred. This object must be removed using @ref deleteData
     */
    RTPS_DllAPI traits<DynamicType>::ref_type GetDynamicType() const;

    /*
     * Sets up the internal @ref DynamicType object
     * @param @ref DynamicData object whose type to copy
     * @return @ref ReturnCode_t with operation status
     * @remark Ownership is not transferred.
     */
    RTPS_DllAPI ReturnCode_t SetDynamicType(
            traits<DynamicData>::ref_type data);

    /*
     * Sets up the internal @ref DynamicType object
     * @param @ref DynamicType to copy
     * @return @ref ReturnCode_t with operation status
     * @remark Ownership is not transferred.
     */
    RTPS_DllAPI ReturnCode_t SetDynamicType(
            traits<DynamicType>::ref_type type);
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_DYNAMIC_TYPES_DYNAMIC_PUB_SUB_TYPE_HPP
