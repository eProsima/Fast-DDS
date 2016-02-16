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
#include <boost/asio.hpp>

class ReqRepAsReliableHelloWorldReplier : public ReqRepHelloWorldReplier
{
    public:
        void configSubscriber(SubscriberAttributes &rattr, const std::string& suffix)
        {
            rattr.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
            rattr.topic.topicName = "ReqRepAsReliableHelloworld_" + boost::asio::ip::host_name();
            rattr.topic.topicName += "_" + suffix;
        };

        void configPublisher(PublisherAttributes &puattr, const std::string& suffix)
        {
            puattr.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
            puattr.topic.topicName = "ReqRepAsReliableHelloworld_" + boost::asio::ip::host_name();
            puattr.topic.topicName += "_" + suffix;
        }
};

#endif // _TEST_BLACKBOX_REQREPASRELIABLEHELLOWORLDREPLIER_HPP_

