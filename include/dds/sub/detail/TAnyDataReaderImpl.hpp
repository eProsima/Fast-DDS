/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef OMG_DDS_SUB_DETAIL_TANYDATAREADER_HPP_
#define OMG_DDS_SUB_DETAIL_TANYDATAREADER_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */

#include <dds/sub/AnyDataReader.hpp>
#include <dds/topic/TopicDescription.hpp>

// Implementation

namespace dds
{
namespace sub
{

template<typename DELEGATE>
TAnyDataReader<DELEGATE>::~TAnyDataReader() {}

template<typename DELEGATE>
const dds::sub::Subscriber&
TAnyDataReader<DELEGATE>::subscriber() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->subscriber();
}

template<typename DELEGATE>
const dds::topic::TopicDescription&
TAnyDataReader<DELEGATE>::topic_description() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    return this->delegate()->topic_description();
}

template<typename DELEGATE>
void
TAnyDataReader<DELEGATE>::wait_for_historical_data(const dds::core::Duration& timeout)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->wait_for_historical_data(timeout);
}


template<typename DELEGATE>
dds::sub::qos::DataReaderQos
TAnyDataReader<DELEGATE>::qos() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->qos();
}

template<typename DELEGATE>
void
TAnyDataReader<DELEGATE>::qos(const dds::sub::qos::DataReaderQos& qos)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->qos(qos);
}

template<typename DELEGATE>
TAnyDataReader<DELEGATE>&
TAnyDataReader<DELEGATE>::operator << (const dds::sub::qos::DataReaderQos& qos)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->qos(qos);
    return *this;
}

template<typename DELEGATE>
const TAnyDataReader<DELEGATE>&
TAnyDataReader<DELEGATE>::operator >> (dds::sub::qos::DataReaderQos& qos) const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    qos = this->delegate()->qos();
    return *this;
}


template<typename DELEGATE>
dds::core::status::LivelinessChangedStatus
TAnyDataReader<DELEGATE>::liveliness_changed_status()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->liveliness_changed_status();
}

template<typename DELEGATE>
dds::core::status::SampleRejectedStatus
TAnyDataReader<DELEGATE>::sample_rejected_status()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->sample_rejected_status();
}

template<typename DELEGATE>
dds::core::status::SampleLostStatus
TAnyDataReader<DELEGATE>::sample_lost_status()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->sample_lost_status();
}

template<typename DELEGATE>
dds::core::status::RequestedDeadlineMissedStatus
TAnyDataReader<DELEGATE>::requested_deadline_missed_status()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->requested_deadline_missed_status();
}

template<typename DELEGATE>
dds::core::status::RequestedIncompatibleQosStatus
TAnyDataReader<DELEGATE>::requested_incompatible_qos_status()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->requested_incompatible_qos_status();
}

template<typename DELEGATE>
dds::core::status::SubscriptionMatchedStatus
TAnyDataReader<DELEGATE>::subscription_matched_status()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->subscription_matched_status();
}

}
}
// End of implementation

#endif /* OMG_DDS_SUB_DETAIL_TANYDATAREADER_HPP_ */
