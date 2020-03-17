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
        const fastrtps::SubscriberAttributes& att,
        SubscriberListener* listen)
    : participant_(p)
    , qos_(&qos == &SUBSCRIBER_QOS_DEFAULT ? participant_->get_default_subscriber_qos() : qos)
    , att_(att)
    , listener_(listen)
    , subscriber_listener_(this)
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

const SubscriberListener* SubscriberImpl::get_listener() const
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
        const fastrtps::TopicAttributes& topic_att,
        const fastrtps::ReaderQos& reader_qos,
        DataReaderListener* listener)
{
    logInfo(SUBSCRIBER, "CREATING SUBSCRIBER IN TOPIC: " << topic_att.getTopicName())
    //Look for the correct type registration
    TypeSupport type_support = participant_->find_type(topic_att.getTopicDataType().to_string());

    /// Preconditions
    // Check the type was registered.
    if (type_support.empty())
    {
        logError(SUBSCRIBER, "Type : " << topic_att.getTopicDataType() << " Not Registered");
        return nullptr;
    }
    if (topic_att.topicKind == WITH_KEY && !type_support->m_isGetKeyDefined)
    {
        logError(SUBSCRIBER, "Keyed Topic needs getKey function");
        return nullptr;
    }

    if (!reader_qos.checkQos() || !topic_att.checkQos())
    {
        return nullptr;
    }

    ReaderAttributes ratt;
    ratt.endpoint.durabilityKind = reader_qos.m_durability.durabilityKind();
    ratt.endpoint.endpointKind = READER;
    ratt.endpoint.multicastLocatorList = att_.multicastLocatorList;
    ratt.endpoint.reliabilityKind = reader_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS ? RELIABLE : BEST_EFFORT;
    ratt.endpoint.topicKind = topic_att.topicKind;
    ratt.endpoint.unicastLocatorList = att_.unicastLocatorList;
    ratt.endpoint.remoteLocatorList = att_.remoteLocatorList;
    ratt.expectsInlineQos = att_.expectsInlineQos;
    ratt.endpoint.properties = att_.properties;

    if (att_.getEntityID() > 0)
    {
        ratt.endpoint.setEntityID(static_cast<uint8_t>(att_.getEntityID()));
    }

    if (att_.getUserDefinedID() > 0)
    {
        ratt.endpoint.setUserDefinedID(static_cast<uint8_t>(att_.getUserDefinedID()));
    }

    ratt.times = att_.times;

    // TODO(Ricardo) Remove in future
    // Insert topic_name and partitions
    Property property;
    property.name("topic_name");
    property.value(topic_att.getTopicName().c_str());
    ratt.endpoint.properties.properties().push_back(std::move(property));
    if (reader_qos.m_partition.names().size() > 0)
    {
        property.name("partitions");
        std::string partitions;
        for (auto partition : reader_qos.m_partition.names())
        {
            partitions += partition + ";";
        }
        property.value(std::move(partitions));
        ratt.endpoint.properties.properties().push_back(std::move(property));
    }
    if (reader_qos.m_disablePositiveACKs.enabled)
    {
        ratt.disable_positive_acks = true;
    }

    DataReaderImpl* impl = new DataReaderImpl(
        this,
        type_support,
        topic_att,
        ratt,
        reader_qos,
        att_.historyMemoryPolicy,
        listener);

    if (impl->reader_ == nullptr)
    {
        logError(SUBSCRIBER, "Problem creating associated Reader");
        delete impl;
        return nullptr;
    }

    DataReader* reader = new DataReader(impl);
    impl->user_datareader_ = reader;

    rtps_participant_->registerReader(impl->reader_, topic_att, reader_qos);

    {
        std::lock_guard<std::mutex> lock(mtx_readers_);
        readers_[topic_att.getTopicName().to_string()].push_back(impl);
    }

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
    auto it = readers_.find(reader->get_topic().getTopicName().to_string());
    if (it != readers_.end())
    {
        auto dr_it = std::find(it->second.begin(), it->second.end(), reader->impl_);
        if (dr_it != it->second.end())
        {
            (*dr_it)->set_listener(nullptr);
            it->second.erase(dr_it);
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
    for (auto it : readers_)
    {
        for (DataReaderImpl* dr : it.second)
        {
            dr->listener_->on_data_available(dr->user_datareader_);
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
        const fastrtps::ReaderQos& qos)
{
    if (&qos == &DATAREADER_QOS_DEFAULT)
    {
        default_datareader_qos_.setQos(DATAREADER_QOS_DEFAULT, true);
        return ReturnCode_t::RETCODE_OK;
    }
    else if (qos.checkQos())
    {
        default_datareader_qos_.setQos(qos, true);
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
}

const fastrtps::ReaderQos& SubscriberImpl::get_default_datareader_qos() const
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

/* TODO
   bool SubscriberImpl::copy_from_topic_qos(
        fastrtps::ReaderQos&,
        const fastrtps::TopicAttributes&) const
   {
    logError(PUBLISHER, "Operation not implemented");
    return false;
   }
 */

bool SubscriberImpl::set_attributes(
        const fastrtps::SubscriberAttributes& att)
{
    bool updated = true;
    bool missing = false;
    if (att.unicastLocatorList.size() != att_.unicastLocatorList.size() ||
            att.multicastLocatorList.size() != att_.multicastLocatorList.size())
    {
        logWarning(RTPS_READER, "Locator Lists cannot be changed or updated in this version");
        updated &= false;
    }
    else
    {
        for (LocatorListConstIterator lit1 = att_.unicastLocatorList.begin();
                lit1 != att_.unicastLocatorList.end(); ++lit1)
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
        for (LocatorListConstIterator lit1 = att_.multicastLocatorList.begin();
                lit1 != att_.multicastLocatorList.end(); ++lit1)
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
        att_ = att;
    }

    return updated;
}

const DomainParticipant* SubscriberImpl::get_participant() const
{
    return participant_->get_participant();
}

void SubscriberImpl::SubscriberReaderListener::on_data_available(
        DataReader* /*reader*/)
{
    if (subscriber_->listener_ != nullptr)
    {
        subscriber_->listener_->on_new_data_message(subscriber_->user_subscriber_);
    }
}

void SubscriberImpl::SubscriberReaderListener::on_subscription_matched(
        DataReader* /*reader*/,
        const fastdds::dds::SubscriptionMatchedStatus& info)
{
    if (subscriber_->listener_ != nullptr)
    {
        subscriber_->listener_->on_subscription_matched(subscriber_->user_subscriber_, info);
    }
}

void SubscriberImpl::SubscriberReaderListener::on_requested_deadline_missed(
        DataReader* /*reader*/,
        const fastrtps::RequestedDeadlineMissedStatus& status)
{
    if (subscriber_->listener_ != nullptr)
    {
        subscriber_->listener_->on_requested_deadline_missed(subscriber_->user_subscriber_, status);
    }
}

void SubscriberImpl::SubscriberReaderListener::on_liveliness_changed(
        DataReader* /*reader*/,
        const fastrtps::LivelinessChangedStatus& status)
{
    if (subscriber_->listener_ != nullptr)
    {
        subscriber_->listener_->on_liveliness_changed(subscriber_->user_subscriber_, status);
    }
}

void SubscriberImpl::SubscriberReaderListener::on_sample_rejected(
        DataReader* /*reader*/,
        const fastrtps::SampleRejectedStatus& /*status*/)
{
    /* TODO
       if (subscriber_->listener_ != nullptr)
       {
        subscriber_->listener_->on_sample_rejected(subscriber_->user_subscriber_, status);
       }
     */
}

void SubscriberImpl::SubscriberReaderListener::on_requested_incompatible_qos(
        DataReader* /*reader*/,
        const RequestedIncompatibleQosStatus& /*status*/)
{
    /* TODO
       if (subscriber_->listener_ != nullptr)
       {
        subscriber_->listener_->on_requested_incompatible_qos(subscriber_->user_subscriber_, status);
       }
     */
}

void SubscriberImpl::SubscriberReaderListener::on_sample_lost(
        DataReader* /*reader*/,
        const SampleLostStatus& /*status*/)
{
    /* TODO
       if (subscriber_->listener_ != nullptr)
       {
        subscriber_->listener_->on_sample_rejected(subscriber_->user_subscriber_, status);
       }
     */
}

const InstanceHandle_t& SubscriberImpl::get_instance_handle() const
{
    return handle_;
}

bool SubscriberImpl::type_in_use(
        const std::string& type_name) const
{
    for (auto it : readers_)
    {
        for (DataReaderImpl* reader : it.second)
        {
            if (reader->get_topic().getTopicDataType() == type_name)
            {
                return true; // Is in use
            }
        }
    }
    return false;
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
