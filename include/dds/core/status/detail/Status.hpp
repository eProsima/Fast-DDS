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
#include <fastdds/dds/core/status/LivelinessChangedStatus.hpp>
#include <fastdds/dds/core/status/DeadlineMissedStatus.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/dds/core/status/SampleRejectedStatus.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
namespace core {
namespace status {

using StatusMask = eprosima::fastdds::dds::StatusMask;

namespace detail {

using InconsistentTopicStatus = eprosima::fastdds::dds::InconsistentTopicStatus;

using LivelinessChangedStatus = eprosima::fastdds::dds::LivelinessChangedStatus;

using LivelinessLostStatus = eprosima::fastdds::dds::LivelinessLostStatus;

using OfferedDeadlineMissedStatus = eprosima::fastdds::dds::OfferedDeadlineMissedStatus;

using OfferedIncompatibleQosStatus = eprosima::fastdds::dds::OfferedIncompatibleQosStatus;

using PublicationMatchedStatus = eprosima::fastdds::dds::PublicationMatchedStatus;

using SampleRejectedStatus = eprosima::fastdds::dds::SampleRejectedStatus;

using RequestedDeadlineMissedStatus = eprosima::fastdds::dds::RequestedDeadlineMissedStatus;

using RequestedIncompatibleQosStatus = eprosima::fastdds::dds::RequestedIncompatibleQosStatus;

using SampleLostStatus = eprosima::fastdds::dds::SampleLostStatus;

using SubscriptionMatchedStatus = eprosima::fastdds::dds::SubscriptionMatchedStatus;

} //namespace detail
} //namespace status
} //namespace core
} //namespace dds

/** @endcond */

#endif //EPROSIMA_DDS_CORE_STATUS_DETAIL_STATUS_HPP_
