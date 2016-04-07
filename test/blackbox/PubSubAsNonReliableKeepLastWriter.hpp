/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubAsNonReliableKeepLastWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBASNONRELIABLEKEEPLASTWRITER_HPP_
#define _TEST_BLACKBOX_PUBSUBASNONRELIABLEKEEPLASTWRITER_HPP_

#include "PubSubKeepLastWriter.hpp" 
#include <boost/asio.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>

class PubSubAsNonReliableKeepLastWriter : public PubSubKeepLastWriter
{
    public:
        void configPublisher(PublisherAttributes &puattr)
        {
            puattr.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;

            std::ostringstream t;

            t << "PubSubAsNonReliableKeepLast_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();

            puattr.topic.topicName = t.str();
        }
};

#endif // _TEST_BLACKBOX_PUBSUBASNONRELIABLEKEEPLASTWRITER_HPP_
