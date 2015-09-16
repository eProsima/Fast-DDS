/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReqRepAsReliableHelloWorldRequester.hpp
 *
 */

#ifndef _TEST_BLACKBOX_REQREPASRELIABLEHELLOWORLDREQUESTER_HPP_
#define _TEST_BLACKBOX_REQREPASRELIABLEHELLOWORLDREQUESTER_HPP_

#include "ReqRepHelloWorldRequester.hpp" 

class ReqRepAsReliableHelloWorldRequester : public ReqRepHelloWorldRequester
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

#endif // _TEST_BLACKBOX_REQREPASRELIABLEHELLOWORLDREQUESTER_HPP_

