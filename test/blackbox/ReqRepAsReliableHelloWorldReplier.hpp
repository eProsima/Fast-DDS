/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReqRepAsReliableHelloWorldReplier.hpp
 *
 */

#ifndef _TEST_BLACKBOX_REQREPASRELIABLEHELLOWORLDREPLIER_HPP_
#define _TEST_BLACKBOX_REQREPASRELIABLEHELLOWORLDREPLIER_HPP_

#include "ReqRepHelloWorldReplier.hpp" 

class ReqRepAsReliableHelloWorldReplier : public ReqRepHelloWorldReplier
{
    public:
        void configSubscriber(SubscriberAttributes &rattr)
        {
            rattr.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
        };

        void configPublisher(PublisherAttributes &puattr)
        {
            puattr.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        }
};

#endif // _TEST_BLACKBOX_REQREPASRELIABLEHELLOWORLDREPLIER_HPP_

