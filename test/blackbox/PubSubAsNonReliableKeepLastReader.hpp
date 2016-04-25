/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubAsNonReliableKeepLastReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBASNONRELIABLEKEEPLASTREADER_HPP_
#define _TEST_BLACKBOX_PUBSUBASNONRELIABLEKEEPLASTREADER_HPP_

#include "PubSubKeepLastReader.hpp" 
#include <boost/asio.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>

class PubSubAsNonReliableKeepLastReader : public PubSubKeepLastReader
{
    public:
        void configSubscriber(SubscriberAttributes& sattr)
        {
            std::ostringstream t;

            t << "PubSubAsNonReliableKeepLast_" << boost::asio::ip::host_name() << "_" << boost::interprocess::ipcdetail::get_current_process_id();

            sattr.topic.topicName = t.str();
        };
};

#endif // _TEST_BLACKBOX_PUBSUBASNONRELIABLEKEEPLASTREADER_HPP_

