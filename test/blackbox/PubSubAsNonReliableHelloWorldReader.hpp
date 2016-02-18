/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubAsNonReliableHelloWorldReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBASNONRELIABLEHELLOWORLDREADER_HPP_
#define _TEST_BLACKBOX_PUBSUBASNONRELIABLEHELLOWORLDREADER_HPP_

#include "PubSubHelloWorldReader.hpp" 
#include <boost/asio.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>

class PubSubAsNonReliableHelloWorldReader : public PubSubHelloWorldReader
{
    public:
        void configSubscriber(SubscriberAttributes& sattr)
        {
            sattr.topic.topicName = "PubSubAsNonReliableHelloworld_" + boost::asio::ip::host_name();
            sattr.topic.topicName += "_" + boost::interprocess::ipcdetail::get_current_process_id();
        };
};

#endif // _TEST_BLACKBOX_PUBSUBASNONRELIABLEHELLOWORLDREADER_HPP_

