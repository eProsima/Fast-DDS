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

class PubSubAsReliableHelloWorldWriter : public PubSubHelloWorldWriter
{
    public:
        void configPublisher(PublisherAttributes &puattr)
        {
            puattr.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
        }
};

#endif // _TEST_BLACKBOX_PUBSUBASRELIABLEHELLOWORLDWRITER_HPP_

