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


/**
 * @file
 */

#ifndef OSPL_DDS_PUB_DETAIL_DISCOVERY_HPP_
#define OSPL_DDS_PUB_DETAIL_DISCOVERY_HPP_

#include <dds/pub/DataWriter.hpp>

/**
 * @cond
 * Ignore this file in the API
 */

namespace dds
{
namespace pub
{

template <typename FwdIterator>
void ignore(const dds::domain::DomainParticipant& dp, FwdIterator begin, FwdIterator end)
{
    ISOCPP_THROW_EXCEPTION(ISOCPP_UNSUPPORTED_ERROR, "Function not currently supported");
}

template <typename T>
::dds::core::InstanceHandleSeq
matched_subscriptions(const dds::pub::DataWriter<T>& dw)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(dw);
    return dw.delegate()->matched_subscriptions();
}

template <typename T, typename FwdIterator>
uint32_t
matched_subscriptions(const dds::pub::DataWriter<T>& dw,
                      FwdIterator begin, uint32_t max_size)
{
	ISOCPP_REPORT_STACK_DDS_BEGIN(dw);
    return dw.delegate()->matched_subscriptions(begin, max_size);
}

template <typename T>
const dds::topic::SubscriptionBuiltinTopicData
matched_subscription_data(const dds::pub::DataWriter<T>& dw,
                          const ::dds::core::InstanceHandle& h)
{
	ISOCPP_REPORT_STACK_DDS_BEGIN(dw);
    return dw.delegate()->matched_subscription_data(h);
}

}
}

/** @endcond */

#endif /* OSPL_DDS_PUB_DETAIL_DISCOVERY_HPP_ */
