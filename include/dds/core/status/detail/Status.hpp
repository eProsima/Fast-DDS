#ifndef OMG_DDS_CORE_STATUS_DETAIL_STATUS_HPP_
#define OMG_DDS_CORE_STATUS_DETAIL_STATUS_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
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
 */

#include <dds/core/status/detail/TStatusImpl.hpp>
#include <org/opensplice/core/status/StatusDelegate.hpp>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds { namespace core { namespace status { namespace detail {
    typedef dds::core::status::TInconsistentTopicStatus< org::opensplice::core::InconsistentTopicStatusDelegate >
    InconsistentTopicStatus;

    typedef dds::core::status::TLivelinessChangedStatus <org::opensplice::core::LivelinessChangedStatusDelegate>
    LivelinessChangedStatus;

    typedef dds::core::status::TLivelinessLostStatus<org::opensplice::core::LivelinessLostStatusDelegate>
    LivelinessLostStatus;

    typedef dds::core::status::TOfferedDeadlineMissedStatus<org::opensplice::core::OfferedDeadlineMissedStatusDelegate>
    OfferedDeadlineMissedStatus;

    typedef dds::core::status::TOfferedIncompatibleQosStatus<org::opensplice::core::OfferedIncompatibleQosStatusDelegate>
    OfferedIncompatibleQosStatus;

    typedef dds::core::status::TPublicationMatchedStatus<org::opensplice::core::PublicationMatchedStatusDelegate>
    PublicationMatchedStatus;

    typedef dds::core::status::TSampleRejectedStatus< org::opensplice::core::SampleRejectedStatusDelegate >
    SampleRejectedStatus;

    typedef dds::core::status::TRequestedDeadlineMissedStatus<org::opensplice::core::RequestedDeadlineMissedStatusDelegate>
    RequestedDeadlineMissedStatus;

    typedef dds::core::status::TRequestedIncompatibleQosStatus<org::opensplice::core::RequestedIncompatibleQosStatusDelegate>
    RequestedIncompatibleQosStatus;

    typedef dds::core::status::TSampleLostStatus<org::opensplice::core::SampleLostStatusDelegate>
    SampleLostStatus;

    typedef dds::core::status::TSubscriptionMatchedStatus<org::opensplice::core::SubscriptionMatchedStatusDelegate>
    SubscriptionMatchedStatus;
} } } } // namespace dds::core::status::detail

/** @endcond */

#endif /* OMG_DDS_CORE_STATUS_DETAIL_STATUS_HPP_ */
