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

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/domain/DomainParticipantImpl.hpp>
#include <fastdds/rtps/common/Property.hpp>
#include <fastdds/rtps/participant/RTPSParticipant.hpp>
#include <fastdds/subscriber/DataReaderImpl.hpp>
#include <fastdds/topic/TopicDescriptionImpl.hpp>
#include <fastdds/utils/QosConverters.hpp>

#include <rtps/network/utils/netmask_filter.hpp>
#ifdef FASTDDS_STATISTICS
#include <statistics/types/monitorservice_types.hpp>
#endif //FASTDDS_STATISTICS
#include <xmlparser/attributes/SubscriberAttributes.hpp>
#include <xmlparser/XMLProfileManager.h>

namespace eprosima {
namespace fastdds {
namespace dds {

using xmlparser::XMLProfileManager;
using xmlparser::XMLP_ret;
using rtps::InstanceHandle_t;
using rtps::Property;

SubscriberImpl::SubscriberImpl(
        DomainParticipantImpl* p,
        const SubscriberQos& qos,
        SubscriberListener* listen)
    : participant_(p)
    , qos_(&qos == &SUBSCRIBER_QOS_DEFAULT ? participant_->get_default_subscriber_qos() : qos)
    , listener_(listen)
    , subscriber_listener_(this)
    , user_subscriber_(nullptr)
    , rtps_participant_(p->get_rtps_participant())
    , default_datareader_qos_(DATAREADER_QOS_DEFAULT)
{
    xmlparser::SubscriberAttributes sub_attr;
    XMLProfileManager::getDefaultSubscriberAttributes(sub_attr);
    utils::set_qos_from_attributes(default_datareader_qos_, sub_attr);
}

ReturnCode_t SubscriberImpl::enable()
{
    if (qos_.entity_factory().autoenable_created_entities)
    {
        std::lock_guard<std::mutex> lock(mtx_readers_);
        for (auto topic_readers : readers_)
        {
            for (DataReaderImpl* dr : topic_readers.second)
            {
                dr->user_datareader_->enable();
            }
        }
    }

    return RETCODE_OK;
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
    bool enabled = user_subscriber_->is_enabled();
    const SubscriberQos& qos_to_set = (&qos == &SUBSCRIBER_QOS_DEFAULT) ?
            participant_->get_default_subscriber_qos() : qos;

    if (&qos != &SUBSCRIBER_QOS_DEFAULT)
    {
        ReturnCode_t check_result = check_qos(qos_to_set);
        if (RETCODE_OK != check_result)
        {
            return check_result;
        }
    }

    if (enabled && !can_qos_be_updated(qos_, qos_to_set))
    {
        return RETCODE_IMMUTABLE_POLICY;
    }
    set_qos(qos_, qos_to_set, !enabled);

    if (enabled)
    {
        std::lock_guard<std::mutex> lock(mtx_readers_);
        for (auto topic_readers : readers_)
        {
            for (auto reader : topic_readers.second)
            {
                reader->subscriber_qos_updated();
            }
        }
    }

    return RETCODE_OK;
}

const SubscriberListener* SubscriberImpl::get_listener() const
{
    return listener_;
}

ReturnCode_t SubscriberImpl::set_listener(
        SubscriberListener* listener)
{
    listener_ = listener;
    return RETCODE_OK;
}

DataReaderImpl* SubscriberImpl::create_datareader_impl(
        const TypeSupport& type,
        TopicDescription* topic,
        const DataReaderQos& qos,
        DataReaderListener* listener,
        std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool)
{
    return new DataReaderImpl(this, type, topic, qos, listener, payload_pool);
}

DataReader* SubscriberImpl::create_datareader(
        TopicDescription* topic,
        const DataReaderQos& qos,
        DataReaderListener* listener,
        const StatusMask& mask,
        std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool)
{
    EPROSIMA_LOG_INFO(SUBSCRIBER, "CREATING SUBSCRIBER IN TOPIC: " << topic->get_name());
    //Look for the correct type registration
    TypeSupport type_support = participant_->find_type(topic->get_type_name());

    /// Preconditions
    // Check the type was registered.
    if (type_support.empty())
    {
        EPROSIMA_LOG_ERROR(SUBSCRIBER, "Type : " << topic->get_type_name() << " Not Registered");
        return nullptr;
    }

    if (RETCODE_OK != DataReaderImpl::check_qos_including_resource_limits(qos, type_support))
    {
        return nullptr;
    }

    // Check netmask filtering preconditions
    if (nullptr != rtps_participant_)
    {
        std::vector<fastdds::rtps::TransportNetmaskFilterInfo> netmask_filter_info =
                rtps_participant_->get_netmask_filter_info();
        std::string error_msg;
        if (!fastdds::rtps::network::netmask_filter::check_preconditions(netmask_filter_info,
                qos.endpoint().ignore_non_matching_locators,
                error_msg) ||
                !fastdds::rtps::network::netmask_filter::check_preconditions(netmask_filter_info,
                qos.endpoint().external_unicast_locators, error_msg))
        {
            EPROSIMA_LOG_ERROR(SUBSCRIBER,
                    "Failed to create reader -> " << error_msg);
            return nullptr;
        }
    }

    topic->get_impl()->reference();

    DataReaderImpl* impl = create_datareader_impl(type_support, topic, qos, listener, payload_pool);
    DataReader* reader = new DataReader(impl, mask);
    impl->user_datareader_ = reader;

    {
        std::lock_guard<std::mutex> lock(mtx_readers_);
        readers_[topic->get_name()].push_back(impl);
    }

    if (user_subscriber_->is_enabled() && qos_.entity_factory().autoenable_created_entities)
    {
        if (RETCODE_OK != reader->enable())
        {
            delete_datareader(reader);
            return nullptr;
        }
    }

    return reader;
}

DataReader* SubscriberImpl::create_datareader_with_profile(
        TopicDescription* topic,
        const std::string& profile_name,
        DataReaderListener* listener,
        const StatusMask& mask,
        std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool)
{
    // TODO (ILG): Change when we have full XML support for DDS QoS profiles
    xmlparser::SubscriberAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillSubscriberAttributes(profile_name, attr))
    {
        DataReaderQos qos = default_datareader_qos_;
        utils::set_qos_from_attributes(qos, attr);
        return create_datareader(topic, qos, listener, mask, payload_pool);
    }

    return nullptr;
}

ReturnCode_t SubscriberImpl::delete_datareader(
        const DataReader* reader)
{
    if (user_subscriber_ != reader->get_subscriber())
    {
        return RETCODE_PRECONDITION_NOT_MET;
    }
    std::unique_lock<std::mutex> lock(mtx_readers_);
    auto it = readers_.find(reader->impl_->get_topicdescription()->get_name());
    if (it != readers_.end())
    {
        auto dr_it = std::find(it->second.begin(), it->second.end(), reader->impl_);
        if (dr_it != it->second.end())
        {
            //First extract the reader from the maps to free the mutex
            DataReaderImpl* reader_impl = *dr_it;
            if (!reader_impl->can_be_deleted(false))
            {
                return RETCODE_PRECONDITION_NOT_MET;
            }

            it->second.erase(dr_it);
            if (it->second.empty())
            {
                readers_.erase(it);
            }
            lock.unlock();

            //Now we can delete it
            reader_impl->get_topicdescription()->get_impl()->dereference();
            delete (reader_impl);
            return RETCODE_OK;
        }
    }
    return RETCODE_ERROR;
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
    return RETCODE_OK;
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
    EPROSIMA_LOG_ERROR(PUBLISHER, "Operation not implemented");
    return false;
   }
 */

/* TODO
   bool SubscriberImpl::end_access()
   {
    EPROSIMA_LOG_ERROR(PUBLISHER, "Operation not implemented");
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
    return RETCODE_OK;
}

ReturnCode_t SubscriberImpl::set_default_datareader_qos(
        const DataReaderQos& qos)
{
    if (&qos == &DATAREADER_QOS_DEFAULT)
    {
        reset_default_datareader_qos();
        return RETCODE_OK;
    }

    ReturnCode_t check_result = DataReaderImpl::check_qos(qos);
    if (RETCODE_OK != check_result)
    {
        return check_result;
    }

    DataReaderImpl::set_qos(default_datareader_qos_, qos, true);
    return RETCODE_OK;
}

void SubscriberImpl::reset_default_datareader_qos()
{
    // TODO (ILG): Change when we have full XML support for DDS QoS profiles
    DataReaderImpl::set_qos(default_datareader_qos_, DATAREADER_QOS_DEFAULT, true);
    xmlparser::SubscriberAttributes attr;
    XMLProfileManager::getDefaultSubscriberAttributes(attr);
    utils::set_qos_from_attributes(default_datareader_qos_, attr);
}

const DataReaderQos& SubscriberImpl::get_default_datareader_qos() const
{
    return default_datareader_qos_;
}

DataReaderQos& SubscriberImpl::get_default_datareader_qos()
{
    return default_datareader_qos_;
}

bool SubscriberImpl::contains_entity(
        const fastdds::rtps::InstanceHandle_t& handle) const
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

ReturnCode_t SubscriberImpl::get_datareader_qos_from_profile(
        const std::string& profile_name,
        DataReaderQos& qos) const
{
    std::string _topic_name;
    return get_datareader_qos_from_profile(profile_name, qos, _topic_name);
}

ReturnCode_t SubscriberImpl::get_datareader_qos_from_profile(
        const std::string& profile_name,
        DataReaderQos& qos,
        std::string& topic_name) const
{
    xmlparser::SubscriberAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fillSubscriberAttributes(profile_name, attr, false))
    {
        qos = default_datareader_qos_;
        utils::set_qos_from_attributes(qos, attr);
        topic_name = attr.topic.getTopicName();
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t SubscriberImpl::get_datareader_qos_from_xml(
        const std::string& xml,
        DataReaderQos& qos) const
{
    std::string _topic_name;
    return get_datareader_qos_from_xml(xml, qos, _topic_name);
}

ReturnCode_t SubscriberImpl::get_datareader_qos_from_xml(
        const std::string& xml,
        DataReaderQos& qos,
        std::string& topic_name) const
{
    xmlparser::SubscriberAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_subscriber_attributes_from_xml(xml, attr, false))
    {
        qos = default_datareader_qos_;
        utils::set_qos_from_attributes(qos, attr);
        topic_name = attr.topic.getTopicName();
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t SubscriberImpl::get_datareader_qos_from_xml(
        const std::string& xml,
        DataReaderQos& qos,
        const std::string& profile_name) const
{
    std::string _topic_name;
    return get_datareader_qos_from_xml(xml, qos, _topic_name, profile_name);
}

ReturnCode_t SubscriberImpl::get_datareader_qos_from_xml(
        const std::string& xml,
        DataReaderQos& qos,
        std::string& topic_name,
        const std::string& profile_name) const
{
    if (profile_name.empty())
    {
        EPROSIMA_LOG_ERROR(SUBSCRIBER, "Provided profile name must be non-empty");
        return RETCODE_BAD_PARAMETER;
    }

    xmlparser::SubscriberAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_subscriber_attributes_from_xml(xml, attr, true, profile_name))
    {
        qos = default_datareader_qos_;
        utils::set_qos_from_attributes(qos, attr);
        topic_name = attr.topic.getTopicName();
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t SubscriberImpl::get_default_datareader_qos_from_xml(
        const std::string& xml,
        DataReaderQos& qos) const
{
    std::string _topic_name;
    return get_default_datareader_qos_from_xml(xml, qos, _topic_name);
}

ReturnCode_t SubscriberImpl::get_default_datareader_qos_from_xml(
        const std::string& xml,
        DataReaderQos& qos,
        std::string& topic_name) const
{
    xmlparser::SubscriberAttributes attr;
    if (XMLP_ret::XML_OK == XMLProfileManager::fill_default_subscriber_attributes_from_xml(xml, attr, true))
    {
        qos = default_datareader_qos_;
        utils::set_qos_from_attributes(qos, attr);
        topic_name = attr.topic.getTopicName();
        return RETCODE_OK;
    }

    return RETCODE_BAD_PARAMETER;
}

ReturnCode_t SubscriberImpl::copy_from_topic_qos(
        DataReaderQos& reader_qos,
        const TopicQos& topic_qos)
{
    reader_qos.durability(topic_qos.durability());
    reader_qos.durability_service(topic_qos.durability_service());
    reader_qos.deadline(topic_qos.deadline());
    reader_qos.latency_budget(topic_qos.latency_budget());
    reader_qos.liveliness(topic_qos.liveliness());
    reader_qos.reliability(topic_qos.reliability());
    reader_qos.destination_order(topic_qos.destination_order());
    reader_qos.history(topic_qos.history());
    reader_qos.resource_limits(topic_qos.resource_limits());
    reader_qos.ownership(topic_qos.ownership());
    reader_qos.representation() = topic_qos.representation();
    return RETCODE_OK;
}

const DomainParticipant* SubscriberImpl::get_participant() const
{
    return const_cast<const DomainParticipantImpl*>(participant_)->get_participant();
}

void SubscriberImpl::SubscriberReaderListener::on_data_available(
        DataReader* reader)
{
    if (subscriber_->listener_ != nullptr)
    {
        subscriber_->listener_->on_data_available(reader);
        subscriber_->listener_->on_data_on_readers(subscriber_->user_subscriber_);
    }
}

void SubscriberImpl::SubscriberReaderListener::on_subscription_matched(
        DataReader* reader,
        const fastdds::dds::SubscriptionMatchedStatus& info)
{
    if (subscriber_->listener_ != nullptr)
    {
        subscriber_->listener_->on_subscription_matched(reader, info);
    }
}

void SubscriberImpl::SubscriberReaderListener::on_requested_deadline_missed(
        DataReader* reader,
        const RequestedDeadlineMissedStatus& status)
{
    if (subscriber_->listener_ != nullptr)
    {
        subscriber_->listener_->on_requested_deadline_missed(reader, status);
    }
}

void SubscriberImpl::SubscriberReaderListener::on_liveliness_changed(
        DataReader* reader,
        const LivelinessChangedStatus& status)
{
    (void)status;

    SubscriberListener* listener = subscriber_->listener_;
    if (listener == nullptr ||
            !subscriber_->user_subscriber_->get_status_mask().is_active(StatusMask::liveliness_changed()))
    {
        auto participant = subscriber_->get_participant();
        auto part_listener = const_cast<DomainParticipantListener*>(participant->get_listener());
        listener = static_cast<SubscriberListener*>(part_listener);
        if (!participant->get_status_mask().is_active(StatusMask::liveliness_changed()))
        {
            listener = nullptr;
        }
    }

    if (listener != nullptr)
    {
        LivelinessChangedStatus callback_status;
        reader->get_liveliness_changed_status(callback_status);
        listener->on_liveliness_changed(reader, callback_status);
    }
}

void SubscriberImpl::SubscriberReaderListener::on_sample_rejected(
        DataReader* /*reader*/,
        const SampleRejectedStatus& /*status*/)
{
    /* TODO
       if (subscriber_->listener_ != nullptr)
       {
        subscriber_->listener_->on_sample_rejected(reader, status);
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
        subscriber_->listener_->on_requested_incompatible_qos(reader, status);
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
        subscriber_->listener_->on_sample_rejected(reader, status);
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
            if (reader->get_topicdescription()->get_type_name() == type_name)
            {
                return true; // Is in use
            }
        }
    }
    return false;
}

void SubscriberImpl::set_qos(
        SubscriberQos& to,
        const SubscriberQos& from,
        bool first_time)
{
    if (first_time || !(to.presentation() == from.presentation()))
    {
        to.presentation() = from.presentation();
        to.presentation().hasChanged = true;
    }
    if (!(to.partition() == from.partition()))
    {
        to.partition() = from.partition();
        to.partition().hasChanged = true;
    }
    if (to.group_data().getValue() != from.group_data().getValue())
    {
        to.group_data() = from.group_data();
        to.group_data().hasChanged = true;
    }
    if (to.entity_factory().autoenable_created_entities != from.entity_factory().autoenable_created_entities)
    {
        to.entity_factory() = from.entity_factory();
    }
}

ReturnCode_t SubscriberImpl::check_qos(
        const SubscriberQos& qos)
{
    (void) qos;
    return RETCODE_OK;
}

bool SubscriberImpl::can_qos_be_updated(
        const SubscriberQos& to,
        const SubscriberQos& from)
{
    (void) to;
    (void) from;
    return true;
}

SubscriberListener* SubscriberImpl::get_listener_for(
        const StatusMask& status)
{
    if (listener_ != nullptr &&
            user_subscriber_->get_status_mask().is_active(status))
    {
        return listener_;
    }
    return participant_->get_listener_for(status);
}

ReturnCode_t SubscriberImpl::delete_contained_entities()
{
    // Let's be optimistic
    ReturnCode_t result = RETCODE_OK;

    std::lock_guard<std::mutex> lock(mtx_readers_);
    for (auto reader: readers_)
    {
        for (DataReaderImpl* dr : reader.second)
        {
            if (!dr->can_be_deleted())
            {
                return RETCODE_PRECONDITION_NOT_MET;
            }
        }
    }

    // We traverse the map trying to delete all readers;
    auto reader_iterator = readers_.begin();
    while (reader_iterator != readers_.end())
    {
        //First extract the reader from the maps to free the mutex
        auto it = reader_iterator->second.begin();
        DataReaderImpl* reader_impl = *it;
        bool ret_code = reader_impl->can_be_deleted();
        if (!ret_code)
        {
            return RETCODE_ERROR;
        }
        reader_impl->set_listener(nullptr);
        it = reader_iterator->second.erase(it);
        if (reader_iterator->second.empty())
        {
            reader_iterator = readers_.erase(reader_iterator);
        }

        reader_impl->get_topicdescription()->get_impl()->dereference();
        delete (reader_impl);
    }
    return result;
}

bool SubscriberImpl::can_be_deleted() const
{
    bool return_status = true;

    std::lock_guard<std::mutex> lock(mtx_readers_);
    for (auto topic_readers : readers_)
    {
        for (DataReaderImpl* dr : topic_readers.second)
        {
            return_status = dr->can_be_deleted();
            if (!return_status)
            {
                return false;
            }
        }
    }
    return true;
}

bool SubscriberImpl::can_be_deleted(
        DataReader* reader) const
{
    if (!reader)
    {
        EPROSIMA_LOG_ERROR(SUBSCRIBER, "DataReader is nullptr.");
        return false;
    }

    if (user_subscriber_ != reader->get_subscriber())
    {
        EPROSIMA_LOG_ERROR(SUBSCRIBER, "DataReader does not belong to this Subscriber.");
        return false;
    }

    std::lock_guard<std::mutex> lock(mtx_readers_);
    auto it = readers_.find(reader->impl_->get_topicdescription()->get_name());

    if (it != readers_.end())
    {
        auto dr_it = std::find(it->second.begin(), it->second.end(), reader->impl_);

        if (dr_it == it->second.end())
        {
            EPROSIMA_LOG_ERROR(SUBSCRIBER, "DataReader implementation not found.");
            return false;
        }

        return (*dr_it)->can_be_deleted();
    }

    EPROSIMA_LOG_ERROR(SUBSCRIBER, "DataReader not found.");
    return false;
}

#ifdef FASTDDS_STATISTICS
bool SubscriberImpl::get_monitoring_status(
        statistics::MonitorServiceData& status,
        const fastdds::rtps::GUID_t& entity_guid)
{
    bool ret = false;
    std::vector<DataReader*> readers;
    if (get_datareaders(readers) == RETCODE_OK)
    {
        for (auto& reader : readers)
        {
            if (reader->guid() == entity_guid)
            {
                switch (status._d())
                {
                    case statistics::StatusKind::INCOMPATIBLE_QOS:
                    {
                        RequestedIncompatibleQosStatus incompatible_qos_status;
                        reader->get_requested_incompatible_qos_status(incompatible_qos_status);
                        status.incompatible_qos_status().total_count(incompatible_qos_status.total_count);
                        status.incompatible_qos_status().last_policy_id(incompatible_qos_status.last_policy_id);
                        for (auto& qos : incompatible_qos_status.policies)
                        {
                            statistics::QosPolicyCount_s count;
                            count.count(qos.count);
                            count.policy_id(qos.policy_id);
                            status.incompatible_qos_status().policies().push_back(count);
                        }
                        ret = true;
                        break;
                    }
                    //! TODO
                    /*case statistics::StatusKind::INCONSISTENT_TOPIC:
                       {
                        reader->get_inconsistent_topic_status();
                        ret = true;
                        break;
                       }*/
                    case statistics::StatusKind::LIVELINESS_CHANGED:
                    {
                        LivelinessChangedStatus liveliness_changed_status;
                        reader->get_liveliness_changed_status(liveliness_changed_status);
                        status.liveliness_changed_status().alive_count(liveliness_changed_status.alive_count);
                        status.liveliness_changed_status().not_alive_count(liveliness_changed_status.not_alive_count);
                        std::memcpy(
                            status.liveliness_changed_status().last_publication_handle().data(),
                            liveliness_changed_status.last_publication_handle.value,
                            16);
                        ret = true;
                        break;
                    }
                    case statistics::StatusKind::DEADLINE_MISSED:
                    {
                        DeadlineMissedStatus deadline_missed_status;
                        reader->get_requested_deadline_missed_status(deadline_missed_status);
                        status.deadline_missed_status().total_count(deadline_missed_status.total_count);
                        std::memcpy(
                            status.deadline_missed_status().last_instance_handle().data(),
                            deadline_missed_status.last_instance_handle.value,
                            16);
                        ret = true;
                        break;
                    }
                    case statistics::StatusKind::SAMPLE_LOST:
                    {
                        SampleLostStatus sample_lost_status;
                        reader->get_sample_lost_status(sample_lost_status);
                        status.sample_lost_status().total_count(sample_lost_status.total_count);
                        ret = true;
                        break;
                    }
                    default:
                    {
                        EPROSIMA_LOG_ERROR(SUBSCRIBER, "Queried status not available for this entity " << status._d());
                        break;
                    }
                }
            }
        }
    }
    else
    {
        EPROSIMA_LOG_ERROR(SUBSCRIBER, "Could not retrieve datareaders");
    }

    return ret;
}

#endif //FASTDDS_STATISTICS

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
