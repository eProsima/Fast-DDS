/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubData64kbReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBDATA64KBREADER_HPP_
#define _TEST_BLACKBOX_PUBSUBDATA64KBREADER_HPP_

#include "types/Data64kbType.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

#include <list>
#include <condition_variable>

using namespace eprosima::fastrtps;

class PubSubData64kbReader 
{
    public:
        class Listener: public SubscriberListener
        {
            public:
                Listener(PubSubData64kbReader &reader) : reader_(reader) {};
                ~Listener(){};
                void onNewDataMessage(Subscriber *sub);
                void onSubscriptionMatched(Subscriber* /*sub*/, MatchingInfo& info)
                {
                    if (info.status == MATCHED_MATCHING)
                        reader_.matched();
                    else
                        reader_.unmatched();
                }

            private:

                Listener& operator=(const Listener&) NON_COPYABLE_CXX11;

                PubSubData64kbReader &reader_;
        } listener_;

        PubSubData64kbReader();
        virtual ~PubSubData64kbReader();
        void init(uint16_t nmsgs);
        bool isInitialized() const { return initialized_; }
        void destroy();
        void newNumber(uint16_t number);
        std::list<uint16_t> getNonReceivedMessages();
        uint16_t lastvalue_;
        void block(uint16_t lastvalue, const std::chrono::seconds &seconds);
        void waitDiscovery();
        void waitRemoval();
        void matched();
        void unmatched();
        virtual void configSubscriber(SubscriberAttributes &sattr) = 0;

    private:

        PubSubData64kbReader& operator=(const PubSubData64kbReader&)NON_COPYABLE_CXX11;

        Participant *participant_;
        Subscriber *subscriber_;
        bool initialized_;
        std::list<uint16_t> msgs_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::mutex mutexDiscovery_;
        std::condition_variable cvDiscovery_;
        unsigned int matched_;
        Data64kbType type_;
};

#endif // _TEST_BLACKBOX_PUBSUBDATA64KBREADER_HPP_
