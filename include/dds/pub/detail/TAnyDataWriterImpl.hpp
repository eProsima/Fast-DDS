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
#ifndef OMG_DDS_PUB_DETAIL_TANYDATAWRITER_HPP_
#define OMG_DDS_PUB_DETAIL_TANYDATAWRITER_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <dds/pub/TAnyDataWriter.hpp>

// Implementation

namespace dds
{
namespace pub
{

template <typename DELEGATE>
TAnyDataWriter<DELEGATE>::~TAnyDataWriter()
{
}

template <typename DELEGATE>
const dds::pub::Publisher&
TAnyDataWriter<DELEGATE>::publisher() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->publisher();
}

template <typename DELEGATE>
const dds::topic::TopicDescription&
TAnyDataWriter<DELEGATE>::topic_description() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);

    return this->delegate()->topic_description();
}

template <typename DELEGATE>
dds::pub::qos::DataWriterQos
TAnyDataWriter<DELEGATE>::qos() const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->qos();
}

template <typename DELEGATE>
void
TAnyDataWriter<DELEGATE>::qos(const dds::pub::qos::DataWriterQos& qos)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->qos(qos);
}

template <typename DELEGATE>
TAnyDataWriter<DELEGATE>&
TAnyDataWriter<DELEGATE>::operator << (const dds::pub::qos::DataWriterQos& qos)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->qos(qos);
    return *this;
}

template <typename DELEGATE>
const TAnyDataWriter<DELEGATE>&
TAnyDataWriter<DELEGATE>::operator >> (dds::pub::qos::DataWriterQos& qos) const
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    qos = this->delegate()->qos();
    return *this;
}

template <typename DELEGATE>
void
TAnyDataWriter<DELEGATE>::wait_for_acknowledgments(const dds::core::Duration& timeout)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->wait_for_acknowledgments(timeout);
}

template <typename DELEGATE>
const dds::core::status::LivelinessLostStatus
TAnyDataWriter<DELEGATE>::liveliness_lost_status()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->liveliness_lost_status();
}

template <typename DELEGATE>
const dds::core::status::OfferedDeadlineMissedStatus
TAnyDataWriter<DELEGATE>::offered_deadline_missed_status()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->offered_deadline_missed_status();
}

template <typename DELEGATE>
const dds::core::status::OfferedIncompatibleQosStatus
TAnyDataWriter<DELEGATE>::offered_incompatible_qos_status()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->offered_incompatible_qos_status();
}

template <typename DELEGATE>
const dds::core::status::PublicationMatchedStatus
TAnyDataWriter<DELEGATE>::publication_matched_status()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    return this->delegate()->publication_matched_status();
}

template <typename DELEGATE>
void
TAnyDataWriter<DELEGATE>::assert_liveliness()
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(*this);
    this->delegate()->assert_liveliness();
}

}
}
// End of implementation

#endif /* OMG_DDS_PUB_DETAIL_TANYDATAWRITER_HPP_ */
