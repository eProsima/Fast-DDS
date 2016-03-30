/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubKeepLastReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBKEEPLASTREADER_HPP_
#define _TEST_BLACKBOX_PUBSUBKEEPLASTREADER_HPP_

#include "types/HelloWorldType.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/SubscriberAttributes.h>

#include <list>
#include <condition_variable>

using namespace eprosima::fastrtps;

class PubSubKeepLastReader 
{
    public:
        class Listener: public SubscriberListener
        {
            public:
                Listener(PubSubKeepLastReader &reader) : reader_(reader) {};
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

                PubSubKeepLastReader &reader_;
        } listener_;

        PubSubKeepLastReader();
        virtual ~PubSubKeepLastReader();
        void init(uint16_t nmsgs);
        bool isInitialized() const { return initialized_; }
        void destroy();
        void newNumber(uint16_t number);
        std::list<uint16_t> getNonReceivedMessages();
        uint16_t lastvalue_;
        void read(uint16_t lastvalue, const std::chrono::seconds &seconds);
        void waitDiscovery();
        void waitRemoval();
        void matched();
        void unmatched();
        void data_received();

    private:

        PubSubKeepLastReader& operator=(const PubSubKeepLastReader&)NON_COPYABLE_CXX11;

        Participant *participant_;
        Subscriber *subscriber_;
        bool initialized_;
        std::list<uint16_t> msgs_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::mutex mutexDiscovery_;
        std::condition_variable cvDiscovery_;
        unsigned int matched_;
        unsigned int data_received_;
        HelloWorldType type_;
};

#endif // _TEST_BLACKBOX_PUBSUBKEEPLASTREADER_HPP_
