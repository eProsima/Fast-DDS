/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubAsReliableHelloWorldWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBASRELIABLEHELLOWORLDWRITER_HPP_
#define _TEST_BLACKBOX_PUBSUBASRELIABLEHELLOWORLDWRITER_HPP_

#include "PubSubHelloWorldWriter.hpp" 
#include <boost/asio.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>

class PubSubAsReliableHelloWorldWriter : public PubSubHelloWorldWriter
{
    public:
        void configPublisher(PublisherAttributes &puattr)
        {
            puattr.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
            puattr.topic.topicName = "PubSubAsReliableHelloworld_" + boost::asio::ip::host_name();
            puattr.topic.topicName += "_" + boost::interprocess::ipcdetail::get_current_process_id();
        }
};

#endif // _TEST_BLACKBOX_PUBSUBASRELIABLEHELLOWORLDWRITER_HPP_
