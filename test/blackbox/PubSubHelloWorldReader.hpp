/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubHelloWorldReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBHELLOWORLDREADER_HPP_
#define _TEST_BLACKBOX_PUBSUBHELLOWORLDREADER_HPP_

#include "types/HelloWorldType.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

#include <list>
#include <condition_variable>

using namespace eprosima::fastrtps;

class PubSubHelloWorldReader 
{
    public:
        class Listener: public SubscriberListener
        {
            public:
                Listener(PubSubHelloWorldReader &reader) : reader_(reader) {};
                ~Listener(){};
                void onNewDataMessage(Subscriber *sub);
                void onSubscriptionMatched(Subscriber* /*sub*/, MatchingInfo& info)
                {
                    if (info.status == MATCHED_MATCHING)
                        reader_.matched();
                }

            private:

                Listener& operator=(const Listener&) NON_COPYABLE_CXX11;

                PubSubHelloWorldReader &reader_;
        } listener_;

        PubSubHelloWorldReader();
        virtual ~PubSubHelloWorldReader();
        void init(uint16_t nmsgs);
        bool isInitialized() const { return initialized_; }
        void newNumber(uint16_t number);
        std::list<uint16_t> getNonReceivedMessages();
        uint16_t lastvalue_;
        void block(uint16_t lastvalue, const std::chrono::seconds &seconds);
        void waitDiscovery();
        void matched();
        virtual void configSubscriber(SubscriberAttributes &sattr) = 0;

    private:

        PubSubHelloWorldReader& operator=(const PubSubHelloWorldReader&)NON_COPYABLE_CXX11;

        Participant *participant_;
        Subscriber *subscriber_;
        bool initialized_;
        std::list<uint16_t> msgs_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::mutex mutexDiscovery_;
        std::condition_variable cvDiscovery_;
        unsigned int matched_;
        HelloWorldType type_;
};

#endif // _TEST_BLACKBOX_PUBSUBHELLOWORLDREADER_HPP_
