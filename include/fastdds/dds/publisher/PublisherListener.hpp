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
 * @file PublisherListener.hpp
 */

#ifndef _FASTDDS_PUBLISHERLISTENER_HPP_
#define _FASTDDS_PUBLISHERLISTENER_HPP_

#include <fastrtps/qos/DeadlineMissedStatus.h>
#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class Publisher;

/**
 * Class PublisherListener, allows the end user to implement callbacks triggered by certain events.
 * @ingroup FASTDDS_MODULE
 */
class RTPS_DllAPI PublisherListener : public DataWriterListener
{
public:

    PublisherListener()
    {
    }

    virtual ~PublisherListener()
    {
    }

    /**
     * This method is called when the Publisher is matched (or unmatched) against an endpoint.
     * @param pub Pointer to the associated Publisher
     * @param info Information regarding the matched subscriber
     */
    virtual void on_publication_matched(
            Publisher* pub,
            const fastdds::dds::PublicationMatchedStatus& info)
    {
        (void)pub;
        (void)info;
    }

    /**
     * A method called when a deadline is missed
     * @param pub Pointer to the associated Publisher
     * @param status The deadline missed status
     */
    virtual void on_offered_deadline_missed(
            Publisher* pub,
            const fastrtps::OfferedDeadlineMissedStatus& status)
    {
        (void)pub;
        (void)status;
    }

    /**
     * @brief Method called when the livelivess of a publisher is lost
     * @param pub The publisher
     * @param status The liveliness lost status
     */
    virtual void on_liveliness_lost(
            Publisher* pub,
            const LivelinessLostStatus& status)
    {
        (void)pub;
        (void)status;
    }

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_PUBLISHERLISTENER_HPP_ */
