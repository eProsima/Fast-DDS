/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubAsReliableData64kbWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBASRELIABLEDATA64KBWRITER_HPP_
#define _TEST_BLACKBOX_PUBSUBASRELIABLEDATA64KBWRITER_HPP_

#include "PubSubData64kbWriter.hpp" 
#include <boost/asio.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>

class PubSubAsReliableData64kbWriter : public PubSubData64kbWriter
{
    public:
        void configPublisher(PublisherAttributes &puattr)
        {
            puattr.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
            puattr.topic.topicName = "PubSubAsReliableData64kb_" + boost::asio::ip::host_name();
            puattr.topic.topicName += "_" + boost::interprocess::ipcdetail::get_current_process_id();
        }
};

#endif // _TEST_BLACKBOX_PUBSUBASRELIABLEDATA64KBWRITER_HPP_
