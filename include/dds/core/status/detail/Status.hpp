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

#ifndef EPROSIMA_DDS_CORE_STATUS_DETAIL_STATUS_HPP_
#define EPROSIMA_DDS_CORE_STATUS_DETAIL_STATUS_HPP_

#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastrtps/qos/LivelinessChangedStatus.h>
#include <fastrtps/qos/LivelinessLostStatus.h>
#include <fastrtps/qos/DeadlineMissedStatus.h>
#include <fastrtps/qos/IncompatibleQosStatus.hpp>
#include <fastrtps/qos/SampleRejectedStatus.hpp>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
namespace core {
namespace status {
namespace detail {

using InconsistentTopicStatus =
    dds::core::status::TInconsistentTopicStatus<eprosima::fastdds::dds::InconsistentTopicStatus>;

using LivelinessChangedStatus =
    dds::core::status::TLivelinessChangedStatus <eprosima::fastrtps::LivelinessChangedStatus>;

using LivelinessLostStatus =
    dds::core::status::TLivelinessLostStatus<eprosima::fastrtps::LivelinessLostStatus>;

using OfferedDeadlineMissedStatus =
    dds::core::status::TOfferedDeadlineMissedStatus<eprosima::fastrtps::OfferedDeadlineMissedStatus>;

using OfferedIncompatibleQosStatus =
    dds::core::status::TOfferedIncompatibleQosStatus<eprosima::fastrtps::OfferedIncompatibleQosStatus>;

using PublicationMatchedStatus =
    dds::core::status::TPublicationMatchedStatus<eprosima::fastdds::dds::PublicationMatchedStatus>;

using SampleRejectedStatus =
    dds::core::status::TSampleRejectedStatus<eprosima::fastrtps::SampleRejectedStatus>;

using RequestedDeadlineMissedStatus =
    dds::core::status::TRequestedDeadlineMissedStatus<eprosima::fastrtps::RequestedDeadlineMissedStatus>;

using RequestedIncompatibleQosStatus =
    dds::core::status::TRequestedIncompatibleQosStatus<eprosima::fastrtps::RequestedIncompatibleQosStatus>;

using SampleLostStatus =
    dds::core::status::TSampleLostStatus<eprosima::fastdds::dds::SampleLostStatus>;

using SubscriptionMatchedStatus =
    dds::core::status::TSubscriptionMatchedStatus<eprosima::fastdds::dds::SubscriptionMatchedStatus>;

} //namespace detail
} //namespace status
} //namespace core
} //namespace dds

/** @endcond */

#include <dds/core/status/detail/TStatusImpl.hpp>

#endif //EPROSIMA_DDS_CORE_STATUS_DETAIL_STATUS_HPP_
