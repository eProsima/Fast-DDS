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
 * @file SubscriberImpl.cpp
 *
 */

#include <fastdds/subscriber/SubscriberImpl.hpp>
#include <fastdds/topic/DataReaderImpl.hpp>
#include <fastdds/domain/DomainParticipantImpl.hpp>

#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/dds/topic/DataReader.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/dds/log/Log.hpp>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastdds {
namespace dds {

SubscriberImpl::SubscriberImpl(
        DomainParticipantImpl* p,
        const SubscriberQos& qos,
        SubscriberListener* listen)
    : participant_(p)
    , qos_(&qos == &SUBSCRIBER_QOS_DEFAULT ? participant_->get_default_subscriber_qos() : qos)
    , listener_(listen)
    , user_subscriber_(nullptr)
    , rtps_participant_(p->rtps_participant())
{
}

void SubscriberImpl::disable()
{
    set_listener(nullptr);
    user_subscriber_->set_listener(nullptr);
    {
        std::lock_guard<std::mutex> lock(mtx_readers_);
        for (auto it = readers_.begin(); it != readers_.end(); ++it)
        {
            for (DataReaderImpl* dr : it->second)
            {
                dr->disable();
            }
        }
    }
}

SubscriberImpl::~SubscriberImpl()
{
    {
        std::lock_guard<std::mutex> lock(mtx_readers_);
        for (auto it = readers_.begin(); it != readers_.end(); ++it)
        {
            for (DataReaderImpl* dr : it->second)
            {
                delete dr;
            }
        }
        readers_.clear();
    }

    delete user_subscriber_;
}

const SubscriberQos& SubscriberImpl::get_qos() const
{
    return qos_;
}

ReturnCode_t SubscriberImpl::set_qos(
        const SubscriberQos& qos)
{
    if (!qos.check_qos())
    {
        if (!qos_.can_qos_be_updated(qos))
        {
            return ReturnCode_t::RETCODE_IMMUTABLE_POLICY;
        }
        qos_.set_qos(qos, false);
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
}

SubscriberListener* SubscriberImpl::get_listener() const
{
    return listener_;
}

ReturnCode_t SubscriberImpl::set_listener(
        SubscriberListener* listener)
{
    listener_ = listener;
    return ReturnCode_t::RETCODE_OK;
}

DataReader* SubscriberImpl::create_datareader(
        Topic* topic,
        const DataReaderQos& reader_qos,
        DataReaderListener* listener,
        const ::dds::core::status::StatusMask& mask)
{
    logInfo(SUBSCRIBER, "CREATING SUBSCRIBER IN TOPIC: " << topic->get_name())
    //Look for the correct type registration
    TypeSupport type_support = participant_->find_type(topic->get_type_name());

    /// Preconditions
    // Check the type was registered.
    if (type_support.empty())
    {
        logError(SUBSCRIBER, "Type : " << topic->get_type_name() << " Not Registered");
        return nullptr;
    }
    if (topic->get_topic_attributes().topicKind == WITH_KEY && !type_support.get()->m_isGetKeyDefined)
    {
        logError(SUBSCRIBER, "Keyed Topic needs getKey function");
        return nullptr;
    }

    if (!reader_qos.checkQos() || !topic->get_topic_attributes().checkQos())
    {
        return nullptr;
    }

    ReaderAttributes ratt;
    ratt.endpoint.durabilityKind = reader_qos.durability.durabilityKind();
    ratt.endpoint.endpointKind = READER;
    ratt.endpoint.multicastLocatorList = qos_.sub_attr.multicastLocatorList;
    ratt.endpoint.reliabilityKind = reader_qos.reliability.kind == RELIABLE_RELIABILITY_QOS ? RELIABLE : BEST_EFFORT;
    ratt.endpoint.topicKind = topic->get_topic_attributes().topicKind;
    ratt.endpoint.unicastLocatorList = qos_.sub_attr.unicastLocatorList;
    ratt.endpoint.remoteLocatorList = qos_.sub_attr.remoteLocatorList;
    ratt.expectsInlineQos = qos_.sub_attr.expectsInlineQos;
    ratt.endpoint.properties = qos_.sub_attr.properties;

    if (qos_.sub_attr.getEntityID() > 0)
    {
        ratt.endpoint.setEntityID(static_cast<uint8_t>(qos_.sub_attr.getEntityID()));
    }

    if (qos_.sub_attr.getUserDefinedID() > 0)
    {
        ratt.endpoint.setUserDefinedID(static_cast<uint8_t>(qos_.sub_attr.getUserDefinedID()));
    }

    ratt.times = qos_.sub_attr.times;

    // TODO(Ricardo) Remove in future
    // Insert topic_name and partitions
    Property property;
    property.name("topic_name");
    property.value(topic->get_name());
    ratt.endpoint.properties.properties().push_back(std::move(property));
    if (qos_.partition.names().size() > 0)
    {
        property.name("partitions");
        std::string partitions;
        for (auto partition : qos_.partition.names())
        {
            partitions += partition + ";";
        }
        property.value(std::move(partitions));
        ratt.endpoint.properties.properties().push_back(std::move(property));
    }
    if (reader_qos.disable_positive_ACKs.enabled)
    {
        ratt.disable_positive_acks = true;
    }

    if (listener == nullptr)
    {
        listener = listener_;
    }

    DataReaderImpl* impl = new DataReaderImpl(
        this,
        type_support,
        topic,
        ratt,
        reader_qos,
        qos_.sub_attr.historyMemoryPolicy,
        listener);

    if (impl->reader_ == nullptr)
    {
        logError(SUBSCRIBER, "Problem creating associated Reader");
        delete impl;
        return nullptr;
    }

    DataReader* reader = new DataReader(impl, mask);
    impl->user_datareader_ = reader;

    if (qos_.entity_factory.autoenable_created_entities == true && user_subscriber_->is_enabled())
    {
        reader->enable();
    }

    ReaderQos rqos = reader_qos.changeToReaderQos();
    rtps_participant_->registerReader(impl->reader_, topic->get_topic_attributes(), rqos);
    reader->set_instance_handle(impl->reader_->getGuid());
    {
        std::lock_guard<std::mutex> lock(mtx_readers_);
        readers_[topic->get_name()].push_back(impl);
    }

    topic->get_readers()->push_back(reader);

    return reader;
}

ReturnCode_t SubscriberImpl::delete_datareader(
        DataReader* reader)
{
    if (user_subscriber_ != reader->get_subscriber())
    {
        return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
    }
    std::lock_guard<std::mutex> lock(mtx_readers_);
    auto it = readers_.find(reader->get_topic()->get_name());
    if (it != readers_.end())
    {
        auto dr_it = std::find(it->second.begin(), it->second.end(), reader->impl_);
        if (dr_it != it->second.end())
        {
            std::vector<DataReader*>* topic_readers = (*dr_it)->get_topic()->get_readers();
            auto t_it = std::find(topic_readers->begin(), topic_readers->end(), reader);
            if (t_it != topic_readers->end())
            {
                topic_readers->erase(t_it);
            }
            BuiltinSubscriber::get_instance()->delete_subscription_data(reader->get_instance_handle());
            (*dr_it)->set_listener(nullptr);
            it->second.erase(dr_it);
            delete *dr_it;
            if (it->second.empty())
            {
                readers_.erase(it);
            }
            return ReturnCode_t::RETCODE_OK;
        }
    }
    return ReturnCode_t::RETCODE_ERROR;
}

DataReader* SubscriberImpl::lookup_datareader(
        const std::string& topic_name) const
{
    std::lock_guard<std::mutex> lock(mtx_readers_);
    auto it = readers_.find(topic_name);
    if (it != readers_.end() && it->second.size() > 0)
    {
        return it->second.front()->user_datareader_;
    }
    return nullptr;
}

ReturnCode_t SubscriberImpl::get_datareaders(
        std::vector<DataReader*>& readers) const
{
    std::lock_guard<std::mutex> lock(mtx_readers_);
    for (auto it : readers_)
    {
        for (DataReaderImpl* dr : it.second)
        {
            readers.push_back(dr->user_datareader_);
        }
    }
    return ReturnCode_t::RETCODE_OK;
}

bool SubscriberImpl::has_datareaders() const
{
    if (readers_.empty())
    {
        return false;
    }
    return true;
}

/* TODO
   bool SubscriberImpl::begin_access()
   {
    logError(PUBLISHER, "Operation not implemented");
    return false;
   }
 */

/* TODO
   bool SubscriberImpl::end_access()
   {
    logError(PUBLISHER, "Operation not implemented");
    return false;
   }
 */

ReturnCode_t SubscriberImpl::notify_datareaders() const
{
    if (!user_subscriber_->is_enabled())
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }
    for (auto it : readers_)
    {
        for (DataReaderImpl* dr : it.second)
        {
            if (dr->user_datareader_->get_status_mask().is_compatible(::dds::core::status::StatusMask::data_available())
                    && dr->user_datareader_->is_enabled() && dr->listener_ != nullptr)
            {
                dr->listener_->on_data_available(dr->user_datareader_);
            }
            else if (dr->get_subscriber()->get_participant().get_listener() != nullptr &&
                    dr->get_subscriber()->get_participant().is_enabled() &&
                    dr->get_subscriber()->get_participant().get_status_mask().is_compatible(::dds::core::status::
                    StatusMask::data_available()))
            {
                dr->get_subscriber()->get_participant().get_listener()->on_data_available(dr->user_datareader_);
            }
        }
    }

    return ReturnCode_t::RETCODE_OK;
}

/* TODO
   bool SubscriberImpl::delete_contained_entities()
   {
    logError(PUBLISHER, "Operation not implemented");
    return false;
   }
 */

ReturnCode_t SubscriberImpl::set_default_datareader_qos(
        const DataReaderQos& qos)
{
    if (!user_subscriber_->is_enabled())
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }
    if (&qos == &DDS_DATAREADER_QOS_DEFAULT)
    {
        default_datareader_qos_.setQos(DDS_DATAREADER_QOS_DEFAULT, true);
        return ReturnCode_t::RETCODE_OK;
    }
    else if (qos.checkQos())
    {
        default_datareader_qos_.setQos(qos, true);
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
}

const DataReaderQos& SubscriberImpl::get_default_datareader_qos() const
{
    return default_datareader_qos_;
}

bool SubscriberImpl::contains_entity(
        const fastrtps::rtps::InstanceHandle_t& handle) const
{
    std::lock_guard<std::mutex> lock(mtx_readers_);
    for (auto vit : readers_)
    {
        for (DataReaderImpl* dw : vit.second)
        {
            InstanceHandle_t h(dw->guid());
            if (h == handle)
            {
                return true;
            }
        }
    }
    return false;
}

ReturnCode_t SubscriberImpl::copy_from_topic_qos(
        DataReaderQos& reader_qos,
        const TopicQos& topic_qos) const
{
    if (!user_subscriber_->is_enabled())
    {
        return ReturnCode_t::RETCODE_NOT_ENABLED;
    }
    reader_qos.copyFromTopicQos(topic_qos);
    return ReturnCode_t::RETCODE_OK;
}

bool SubscriberImpl::set_attributes(
        const fastrtps::SubscriberAttributes& att)
{
    bool updated = true;
    bool missing = false;
    if (att.unicastLocatorList.size() != qos_.sub_attr.unicastLocatorList.size() ||
            att.multicastLocatorList.size() != qos_.sub_attr.multicastLocatorList.size())
    {
        logWarning(RTPS_READER, "Locator Lists cannot be changed or updated in this version");
        updated &= false;
    }
    else
    {
        for (LocatorListConstIterator lit1 = qos_.sub_attr.unicastLocatorList.begin();
                lit1 != qos_.sub_attr.unicastLocatorList.end(); ++lit1)
        {
            missing = true;
            for (LocatorListConstIterator lit2 = att.unicastLocatorList.begin();
                    lit2 != att.unicastLocatorList.end(); ++lit2)
            {
                if (*lit1 == *lit2)
                {
                    missing = false;
                    break;
                }
            }
            if (missing)
            {
                logWarning(RTPS_READER, "Locator: " << *lit1 << " not present in new list");
                logWarning(RTPS_READER, "Locator Lists cannot be changed or updated in this version");
            }
        }
        for (LocatorListConstIterator lit1 = qos_.sub_attr.multicastLocatorList.begin();
                lit1 != qos_.sub_attr.multicastLocatorList.end(); ++lit1)
        {
            missing = true;
            for (LocatorListConstIterator lit2 = att.multicastLocatorList.begin();
                    lit2 != att.multicastLocatorList.end(); ++lit2)
            {
                if (*lit1 == *lit2)
                {
                    missing = false;
                    break;
                }
            }
            if (missing)
            {
                logWarning(RTPS_READER, "Locator: " << *lit1 << " not present in new list");
                logWarning(RTPS_READER, "Locator Lists cannot be changed or updated in this version");
            }
        }
    }

    if (updated)
    {
        qos_.sub_attr = att;
    }

    return updated;
}

DomainParticipant& SubscriberImpl::get_participant() const
{
    return *participant_->get_participant();
}

bool SubscriberImpl::type_in_use(
        const std::string& type_name) const
{
    for (auto it : readers_)
    {
        for (DataReaderImpl* reader : it.second)
        {
            if (reader->get_topic()->get_type_name() == type_name)
            {
                return true; // Is in use
            }
        }
    }
    return false;
}

ReturnCode_t SubscriberImpl::autoenable_entities()
{
    if (qos_.entity_factory.autoenable_created_entities)
    {
        std::lock_guard<std::mutex> lock(mtx_readers_);
        for (auto topic : readers_)
        {
            for (auto reader: topic.second)
            {
                if (!reader->user_datareader_->is_enabled())
                {
                    reader->user_datareader_->enable();
                }
            }
        }
    }
    return ReturnCode_t::RETCODE_OK;
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
