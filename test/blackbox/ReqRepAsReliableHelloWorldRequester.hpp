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
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4005)
#endif  // _MSC_VER
#include <boost/asio.hpp>
#ifdef _MSC_VER
# pragma warning(pop)
#endif  // _MSC_VER
#include <boost/interprocess/detail/os_thread_functions.hpp>

class ReqRepAsReliableHelloWorldRequester : public ReqRepHelloWorldRequester
{
    public:
        void configSubscriber(SubscriberAttributes &rattr, const std::string& suffix)
        {
            rattr.qos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;

            std::ostringstream t;

            t << "ReqRepAsReliableHelloworld_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id() << "_" << suffix;

            rattr.topic.topicName = t.str();
        };

        void configPublisher(PublisherAttributes &puattr, const std::string& suffix)
        {
            puattr.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;

            std::ostringstream t;

            t << "ReqRepAsReliableHelloworld_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id() << "_" << suffix;

            puattr.topic.topicName = t.str();
        }
};

#endif // _TEST_BLACKBOX_REQREPASRELIABLEHELLOWORLDREQUESTER_HPP_

