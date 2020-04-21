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

class DomainParticipant;

/**
 * NOTE: This class inherits from std::shared_ptr<TopicDataType>.
 * Class TypeSupport used to provide the DomainRTPSParticipant with the methods to serialize,
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
    using Base::operator =;

    RTPS_DllAPI TypeSupport()
        : std::shared_ptr<fastdds::dds::TopicDataType>(nullptr)
    {
    }

    RTPS_DllAPI TypeSupport(
            const TypeSupport& type)
        : std::shared_ptr<fastdds::dds::TopicDataType>(type)
    {
    }

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

    RTPS_DllAPI virtual ReturnCode_t register_type(
            DomainParticipant* participant) const;

    RTPS_DllAPI virtual ReturnCode_t register_type(
            DomainParticipant* participant,
            std::string type_name) const;

    RTPS_DllAPI virtual const std::string& get_type_name() const
    {
        return get()->m_topicDataTypeName;
    }

    RTPS_DllAPI virtual bool serialize(
            void* data,
            fastrtps::rtps::SerializedPayload_t* payload)
    {
        return get()->serialize(data, payload);
    }

    RTPS_DllAPI virtual bool deserialize(
            fastrtps::rtps::SerializedPayload_t* payload,
            void* data)
    {
        return get()->deserialize(payload, data);
    }

    RTPS_DllAPI virtual std::function<uint32_t()> get_serialized_size_provider(
            void* data)
    {
        return get()->getSerializedSizeProvider(data);
    }

    RTPS_DllAPI virtual void* create_data()
    {
        return get()->createData();
    }

    RTPS_DllAPI virtual void delete_data(
            void* data)
    {
        return get()->deleteData(data);
    }

    RTPS_DllAPI virtual bool get_key(
            void* data,
            fastrtps::rtps::InstanceHandle_t* i_handle,
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

    RTPS_DllAPI bool empty() const
    {
        return get() == nullptr;
    }

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_TYPE_SUPPORT_HPP_ */
