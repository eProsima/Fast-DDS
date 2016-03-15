/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PubSubData64kbWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_PUBSUBDATA64KBWRITER_HPP_
#define _TEST_BLACKBOX_PUBSUBDATA64KBWRITER_HPP_

#include "types/Data64kbType.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include <string>
#include <list>
#include <condition_variable>

using namespace eprosima::fastrtps::rtps;

class PubSubData64kbWriter 
{
    class Listener :public PublisherListener
    {
        public:

            Listener(PubSubData64kbWriter &writer) : writer_(writer){};
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

            PubSubData64kbWriter &writer_;

    } listener_;

    public:
        PubSubData64kbWriter();
        virtual ~PubSubData64kbWriter();
        void init(bool async = false);
        bool isInitialized() const { return initialized_; }
        void destroy();
        void send(const std::list<uint16_t> &msgs);
        void matched();
        void unmatched();
        void waitDiscovery();
        void waitRemoval();
        virtual void configPublisher(PublisherAttributes &puattr) = 0;

    private:

        PubSubData64kbWriter& operator=(const PubSubData64kbWriter&)NON_COPYABLE_CXX11;

        Participant *participant_;
        Publisher *publisher_;
        bool initialized_;
        std::mutex mutex_;
        std::condition_variable cv_;
        unsigned int matched_;
        Data64kbType type_;
};

#endif // _TEST_BLACKBOX_PUBSUBDATA64KBWRITER_HPP_
