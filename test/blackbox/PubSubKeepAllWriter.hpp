/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubKeepAllWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBKEEPALLWRITER_HPP_
#define _TEST_BLACKBOX_PUBSUBKEEPALLWRITER_HPP_

#include "types/HelloWorldType.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include <string>
#include <list>
#include <condition_variable>

using namespace eprosima::fastrtps::rtps;

class PubSubKeepAllWriter 
{
    class Listener :public PublisherListener
    {
        public:

            Listener(PubSubKeepAllWriter &writer) : writer_(writer){};
            ~Listener(){};
            void onPublicationMatched(Publisher* /*pub*/, MatchingInfo &info)
            {
                if (info.status == MATCHED_MATCHING)
                    writer_.matched();
                else
                    writer_.unmatched();
            }

        private:

            Listener& operator=(const Listener&) NON_COPYABLE_CXX11;

            PubSubKeepAllWriter &writer_;

    } listener_;

    public:
        PubSubKeepAllWriter();
        virtual ~PubSubKeepAllWriter();
        void init();
        bool isInitialized() const { return initialized_; }
        void destroy();
        void send(const std::list<uint16_t> &msgs);
        void matched();
        void unmatched();
        void waitDiscovery();
        void waitRemoval();

    private:

        PubSubKeepAllWriter& operator=(const PubSubKeepAllWriter&)NON_COPYABLE_CXX11;

        Participant *participant_;
        Publisher *publisher_;
        bool initialized_;
        std::mutex mutex_;
        std::condition_variable cv_;
        unsigned int matched_;
        HelloWorldType type_;
};

#endif // _TEST_BLACKBOX_PUBSUBKEEPALLWRITER_HPP_
