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

#ifndef TYPES_1_3_DYNAMIC_PUB_SUB_TYPE_H
#define TYPES_1_3_DYNAMIC_PUB_SUB_TYPE_H

#include <fastrtps/types/TypesBase.h>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastrtps/utils/md5.h>

namespace eprosima {
namespace fastrtps {
namespace types {
namespace v1_3 {

class DynamicType;
class DynamicData;

class DynamicPubSubType : public virtual eprosima::fastdds::dds::TopicDataType
{
protected:

    void UpdateDynamicTypeInfo();

    const v1_3::DynamicType* dynamic_type_ = nullptr;
    MD5 m_md5;
    unsigned char* m_keyBuffer = nullptr;

public:

    RTPS_DllAPI DynamicPubSubType() = default;

    /*
     * Constructs a @ref DynamicPubSubType from a @ref v1_3::DynamicType
     * @param type @ref v1_3::DynamicType object associated to the data
     * @remark Ownership is not transferred.
     */
    RTPS_DllAPI DynamicPubSubType(
            const v1_3::DynamicType& type);

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
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

    //! Deletes the internal @ref v1_3::DynamicType object
    RTPS_DllAPI void CleanDynamicType();

    /*
     * Returns a copy of the internal  @ref v1_3::DynamicType object
     * @return pointer to the new object
     * @remark Ownership is transferred. This object must be removed using @ref deleteData
     */
    RTPS_DllAPI const v1_3::DynamicType* GetDynamicType() const;

    /*
     * Sets up the internal @ref v1_3::DynamicType object
     * @param @ref v1_3::DynamicData object whose type to copy
     * @return @ref ReturnCode_t with operation status
     * @remark Ownership is not transferred.
     */
    RTPS_DllAPI ReturnCode_t SetDynamicType(
            const v1_3::DynamicData& data);

    /*
     * Sets up the internal @ref v1_3::DynamicType object
     * @param @ref v1_3::DynamicType to copy
     * @return @ref ReturnCode_t with operation status
     * @remark Ownership is not transferred.
     */
    RTPS_DllAPI ReturnCode_t SetDynamicType(
            const v1_3::DynamicType& type);
};

} // namespace v1_3
} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif // TYPES_1_3_DYNAMIC_PUB_SUB_TYPE_H
