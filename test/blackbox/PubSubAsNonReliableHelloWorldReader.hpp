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

class PubSubAsNonReliableHelloWorldReader : public PubSubHelloWorldReader
{
    public:
        void configSubscriber(SubscriberAttributes &/*rattr*/) {};
};

#endif // _TEST_BLACKBOX_PUBSUBASNONRELIABLEHELLOWORLDREADER_HPP_

