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
 * Class TypeSupport used to provide the DomainRTPSParticipant with the methods to serialize,
 * deserialize and get the key of a specific data type.
 * The user should created a class that inherits from this one,
 * where Serialize and deserialize methods MUST be implemented.
 * @ingroup FASTDDS_MODULE
 */
class TypeSupport
{
private:
    //! Data Type Name
    std::string data_type_name_;

    //! Type Identifier
    const fastrtps::types::TypeIdentifier* type_id_;

    // TopicDataType for retro-compatibility
    std::shared_ptr<fastdds::dds::TopicDataType> topic_data_type_;

public:
    RTPS_DllAPI TypeSupport()
        : type_id_(nullptr)
        , topic_data_type_(nullptr)
    {}

    RTPS_DllAPI TypeSupport(
            const TypeSupport& type)
        : data_type_name_(type.data_type_name_)
        , type_id_(type.type_id_)
        , topic_data_type_(type.topic_data_type_)
    {}

    /*!
     * \brief TypeSupport constructor that receives a TopicDataType pointer.
     * The passed pointer will be managed by the TypeSupport object, so creating two TypeSupport
     * from the same pointer or deleting the passed pointer will produce a runtime error.
     * \param ptr
     */
    RTPS_DllAPI explicit TypeSupport(
            fastdds::dds::TopicDataType* ptr)
        : data_type_name_(ptr != nullptr ? ptr->m_topicDataTypeName : "")
        , type_id_(ptr != nullptr ? ptr->type_id_ : nullptr)
        , topic_data_type_(std::shared_ptr<fastdds::dds::TopicDataType>(ptr))
    {}

    /*!
     * \brief TypeSupport constructor that receives a DynamicPubSubType.
     * It will copy the instance so the user will keep the ownership of his object.
     * \param ptr
     */
    RTPS_DllAPI TypeSupport(
            fastrtps::types::DynamicPubSubType ptr)
        : data_type_name_(ptr.m_topicDataTypeName)
        , type_id_(ptr.type_id_)
        , topic_data_type_(std::shared_ptr<fastdds::dds::TopicDataType>(
                  std::make_shared<fastrtps::types::DynamicPubSubType>(std::move(ptr))))
    {}

    RTPS_DllAPI virtual eprosima::fastrtps::types::ReturnCode_t register_type(
            DomainParticipant* participant,
            std::string type_name) const;

    RTPS_DllAPI virtual const std::string& get_type_name() const
    {
        return data_type_name_;
    }

    RTPS_DllAPI virtual bool serialize(
            void* data,
            fastrtps::rtps::SerializedPayload_t* payload) const
    {
        if (topic_data_type_ != nullptr)
        {
            return topic_data_type_->serialize(data, payload);
        }
        return false;
    }

    RTPS_DllAPI virtual bool deserialize(
            fastrtps::rtps::SerializedPayload_t* payload,
            void* data)
    {
        if (topic_data_type_ != nullptr)
        {
            return topic_data_type_->deserialize(payload, data);
        }
        return false;
    }

    RTPS_DllAPI virtual std::function<uint32_t()> get_serialized_size_provider(
            void* data)
    {
        if (topic_data_type_ != nullptr)
        {
            topic_data_type_->getSerializedSizeProvider(data);
        }
        return [](){ return 0; };
    }

    RTPS_DllAPI virtual void* create_data()
    {
        if (topic_data_type_ != nullptr)
        {
            return topic_data_type_->createData();
        }
        return nullptr;
    }

    RTPS_DllAPI virtual void delete_data(
            void* data)
    {
        if (topic_data_type_ != nullptr)
        {
            topic_data_type_->deleteData(data);
        }
    }

    RTPS_DllAPI virtual bool get_key(
            void* data,
            fastrtps::rtps::InstanceHandle_t* i_handle,
            bool force_md5 = false)
    {
        if (topic_data_type_ != nullptr)
        {
            return topic_data_type_->getKey(data, i_handle, force_md5);
        }
        return false;
    }

    RTPS_DllAPI virtual bool operator==(
            const TypeSupport& type_support)
    {
        if (topic_data_type_ != nullptr && type_support.topic_data_type_ != nullptr)
        {
            if (topic_data_type_ == type_support.topic_data_type_)
            {
                return true;
            }
            else
            {
                return topic_data_type_->m_typeSize == type_support.topic_data_type_->m_typeSize
                       && topic_data_type_->m_isGetKeyDefined == type_support.topic_data_type_->m_isGetKeyDefined
                       && topic_data_type_->m_topicDataTypeName == type_support.topic_data_type_->m_topicDataTypeName;
            }
        }
        return data_type_name_ == type_support.data_type_name_;
    }

    RTPS_DllAPI bool empty() const
    {
        if (topic_data_type_ != nullptr)
        {
            return topic_data_type_ == nullptr;
        }
        return data_type_name_.empty();
    }

    RTPS_DllAPI TopicDataType* get() const
    {
        return topic_data_type_.get();
    }
};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_TYPE_SUPPORT_HPP_ */
