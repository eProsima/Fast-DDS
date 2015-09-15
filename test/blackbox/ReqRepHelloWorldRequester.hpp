/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReqRepHelloWorldRequester.hpp
 *
 */

#ifndef _TEST_BLACKBOX_REQREPHELLOWORLDREQUESTER_HPP_
#define _TEST_BLACKBOX_REQREPHELLOWORLDREQUESTER_HPP_

#include "types/HelloWorldType.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include <list>
#include <condition_variable>

using namespace eprosima::fastrtps;

class ReqRepHelloWorldRequester 
{
    public:

        class ReplyListener: public SubscriberListener
        {
            public:
                ReplyListener(ReqRepHelloWorldRequester &requester) : requester_(requester) {};
                ~ReplyListener(){};
                void onNewDataMessage(Subscriber *sub);
                void onSubscriptionMatched(Subscriber* /*sub*/, MatchingInfo& info)
                {
                    if (info.status == MATCHED_MATCHING)
                        requester_.matched();
                }

            private:

                ReplyListener& operator=(const ReplyListener&) NON_COPYABLE_CXX11;

                ReqRepHelloWorldRequester &requester_;
        } reply_listener_;

        class RequestListener : public PublisherListener
    {
        public:

            RequestListener(ReqRepHelloWorldRequester &requester) : requester_(requester){};
            ~RequestListener(){};
            void onPublicationMatched(Publisher* /*pub*/, MatchingInfo &info)
            {
                if (info.status == MATCHED_MATCHING)
                    requester_.matched();
            }

        private:

            RequestListener& operator=(const RequestListener&) NON_COPYABLE_CXX11;

            ReqRepHelloWorldRequester &requester_;

    } request_listener_;

        ReqRepHelloWorldRequester();
        virtual ~ReqRepHelloWorldRequester();
        void init();
        bool isInitialized() const { return initialized_; }
        void newNumber(SampleIdentity related_sample_identity, uint16_t number);
        void block(const std::chrono::seconds &seconds);
        void waitDiscovery();
        void matched();
        void send(const uint16_t number);
        virtual void configSubscriber(SubscriberAttributes &sattr) = 0;
        virtual void configPublisher(PublisherAttributes &puattr) = 0;

    private:

        ReqRepHelloWorldRequester& operator=(const ReqRepHelloWorldRequester&)NON_COPYABLE_CXX11;

        uint16_t current_number_;
        uint16_t number_received_;
        Participant *participant_;
        Subscriber *reply_subscriber_;
        Publisher *request_publisher_;
        bool initialized_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::mutex mutexDiscovery_;
        std::condition_variable cvDiscovery_;
        unsigned int matched_;
        HelloWorldType type_;
        SampleIdentity related_sample_identity_;
};

#endif // _TEST_BLACKBOX_REQREPHELLOWORLDREQUESTER_HPP_
