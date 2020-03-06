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

//TODO: Fix when StatusDelegate is implemented
//#include <dds/core/status/detail/TStatusImpl.hpp>
//#include <org/opensplice/core/status/StatusDelegate.hpp>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds {
namespace core {
namespace status {
namespace detail {

//TODO: Fix when StatusDelegate is implemented
//typedef dds::core::status::TInconsistentTopicStatus< org::opensplice::core::InconsistentTopicStatusDelegate > InconsistentTopicStatus;
class InconsistentTopicStatus
{
};

//TODO: Fix when StatusDelegate is implemented
//typedef dds::core::status::TLivelinessChangedStatus <org::opensplice::core::LivelinessChangedStatusDelegate> LivelinessChangedStatus;
class LivelinessChangedStatus
{
};

//TODO: Fix when StatusDelegate is implemented
//typedef dds::core::status::TLivelinessLostStatus<org::opensplice::core::LivelinessLostStatusDelegate> LivelinessLostStatus;
class LivelinessLostStatus
{
};

//TODO: Fix when StatusDelegate is implemented
//typedef dds::core::status::TOfferedDeadlineMissedStatus<org::opensplice::core::OfferedDeadlineMissedStatusDelegate> OfferedDeadlineMissedStatus;
class OfferedDeadlineMissedStatus
{
};

//TODO: Fix when StatusDelegate is implemented
//typedef dds::core::status::TOfferedIncompatibleQosStatus<org::opensplice::core::OfferedIncompatibleQosStatusDelegate> OfferedIncompatibleQosStatus;
class OfferedIncompatibleQosStatus
{
};

//TODO: Fix when StatusDelegate is implemented
//typedef dds::core::status::TPublicationMatchedStatus<org::opensplice::core::PublicationMatchedStatusDelegate> PublicationMatchedStatus;
class PublicationMatchedStatus
{
};

//TODO: Fix when StatusDelegate is implemented
//typedef dds::core::status::TSampleRejectedStatus< org::opensplice::core::SampleRejectedStatusDelegate > SampleRejectedStatus;
class SampleRejectedStatus
{
};

//TODO: Fix when StatusDelegate is implemented
//typedef dds::core::status::TRequestedDeadlineMissedStatus<org::opensplice::core::RequestedDeadlineMissedStatusDelegate> RequestedDeadlineMissedStatus;
class RequestedDeadlineMissedStatus
{
};

//TODO: Fix when StatusDelegate is implemented
//typedef dds::core::status::TRequestedIncompatibleQosStatus<org::opensplice::core::RequestedIncompatibleQosStatusDelegate> RequestedIncompatibleQosStatus;
class RequestedIncompatibleQosStatus
{
};

//TODO: Fix when StatusDelegate is implemented
//typedef dds::core::status::TSampleLostStatus<org::opensplice::core::SampleLostStatusDelegate> SampleLostStatus;
class SampleLostStatus
{
};

//TODO: Fix when StatusDelegate is implemented
//typedef dds::core::status::TSubscriptionMatchedStatus<org::opensplice::core::SubscriptionMatchedStatusDelegate> SubscriptionMatchedStatus;
class SubscriptionMatchedStatus
{
};

} //namespace detail
} //namespace status
} //namespace core
} //namespace dds

/** @endcond */

#endif //EPROSIMA_DDS_CORE_STATUS_DETAIL_STATUS_HPP_
