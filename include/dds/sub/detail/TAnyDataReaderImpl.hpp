/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */

#include <dds/sub/AnyDataReader.hpp>
#include <dds/topic/TopicDescription.hpp>

namespace dds {
namespace sub {

template<typename DELEGATE>
TAnyDataReader<DELEGATE>::~TAnyDataReader()
{
}

template<typename DELEGATE>
const dds::sub::Subscriber& TAnyDataReader<DELEGATE>::subscriber() const
{
    return this->delegate()->get_subscriber();
}

template<typename DELEGATE>
const dds::topic::TopicDescription& TAnyDataReader<DELEGATE>::topic_description() const
{
    return this->delegate()->get_topic_description();
}

template<typename DELEGATE>
void TAnyDataReader<DELEGATE>::wait_for_historical_data(
        const dds::core::Duration& /*timeout*/)
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    this->delegate()->wait_for_historical_data(timeout);
}


template<typename DELEGATE>
dds::sub::qos::DataReaderQos TAnyDataReader<DELEGATE>::qos() const
{
    return this->delegate()->get_qos();
}

template<typename DELEGATE>
void TAnyDataReader<DELEGATE>::qos(
        const dds::sub::qos::DataReaderQos& qos)
{
    this->delegate()->set_qos(qos);
}

template<typename DELEGATE>
TAnyDataReader<DELEGATE>& TAnyDataReader<DELEGATE>::operator <<(
        const dds::sub::qos::DataReaderQos& qos)
{
    this->qos(qos);
    return *this;
}

template<typename DELEGATE>
const TAnyDataReader<DELEGATE>& TAnyDataReader<DELEGATE>::operator >>(
        dds::sub::qos::DataReaderQos& qos) const
{
    qos = this->qos();
    return *this;
}


template<typename DELEGATE>
dds::core::status::LivelinessChangedStatus TAnyDataReader<DELEGATE>::liveliness_changed_status()
{
    return this->delegate()->get_liveliness_changed_status();
}

template<typename DELEGATE>
dds::core::status::SampleRejectedStatus TAnyDataReader<DELEGATE>::sample_rejected_status()
{
    return this->delegate()->get_sample_rejected_status();
}

template<typename DELEGATE>
dds::core::status::SampleLostStatus TAnyDataReader<DELEGATE>::sample_lost_status()
{
    //To implement
    //    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    //    return this->delegate()->sample_lost_status();
    *this = dds::core::null;
    return *this;
}

template<typename DELEGATE>
dds::core::status::RequestedDeadlineMissedStatus TAnyDataReader<DELEGATE>::requested_deadline_missed_status()
{
    return this->delegate()->get_requested_deadline_missed_status();
}

template<typename DELEGATE>
dds::core::status::RequestedIncompatibleQosStatus TAnyDataReader<DELEGATE>::requested_incompatible_qos_status()
{
    return this->delegate()->get_requested_incompatible_qos_status();
}

template<typename DELEGATE>
dds::core::status::SubscriptionMatchedStatus TAnyDataReader<DELEGATE>::subscription_matched_status()
{
    return this->delegate()->get_subscription_matched_status();
}

} //namespace sub
} //namespace dds

