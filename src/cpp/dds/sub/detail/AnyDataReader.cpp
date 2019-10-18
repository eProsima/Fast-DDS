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

AnyDataReader::~AnyDataReader()
{
}

const dds::sub::Subscriber& AnyDataReader::subscriber() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->subscriber();
}

const dds::topic::TopicDescription& AnyDataReader::topic_description() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->topic_description();
}

void AnyDataReader::wait_for_historical_data(
        const dds::core::Duration& timeout)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    this->delegate()->wait_for_historical_data(timeout);
}


dds::sub::qos::DataReaderQos AnyDataReader::qos() const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->qos();
}

void AnyDataReader::qos(
        const dds::sub::qos::DataReaderQos& qos)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    this->delegate()->qos(qos);
}

AnyDataReader& AnyDataReader::operator <<(
        const dds::sub::qos::DataReaderQos& qos)
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    this->delegate()->qos(qos);
//    return *this;
}

const AnyDataReader& AnyDataReader::operator >>(
        dds::sub::qos::DataReaderQos& qos) const
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    qos = this->delegate()->qos();
//    return *this;
}


dds::core::status::LivelinessChangedStatus AnyDataReader::liveliness_changed_status()
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->liveliness_changed_status();
}

dds::core::status::SampleRejectedStatus AnyDataReader::sample_rejected_status()
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->sample_rejected_status();
}

dds::core::status::SampleLostStatus AnyDataReader::sample_lost_status()
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->sample_lost_status();
}

dds::core::status::RequestedDeadlineMissedStatus AnyDataReader::requested_deadline_missed_status()
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->requested_deadline_missed_status();
}

dds::core::status::RequestedIncompatibleQosStatus AnyDataReader::requested_incompatible_qos_status()
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->requested_incompatible_qos_status();
}

dds::core::status::SubscriptionMatchedStatus AnyDataReader::subscription_matched_status()
{
    //To implement
//    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
//    return this->delegate()->subscription_matched_status();
}

} //namespace sub
} //namespace dds

