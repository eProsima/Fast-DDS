/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReqRepHelloWorldReplier.hpp
 *
 */

#ifndef _TEST_BLACKBOX_REQREPHELLOWORLDREPLIER_HPP_
#define _TEST_BLACKBOX_REQREPHELLOWORLDREPLIER_HPP_

#include "types/HelloWorldType.h"

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/subscriber/SubscriberListener.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include <list>
#include <condition_variable>

using namespace eprosima::fastrtps;

class ReqRepHelloWorldReplier 
{
    public:

        class ReplyListener: public SubscriberListener
        {
            public:
                ReplyListener(ReqRepHelloWorldReplier &replier) : replier_(replier) {};
                ~ReplyListener(){};
                void onNewDataMessage(Subscriber *sub);
                void onSubscriptionMatched(Subscriber* /*sub*/, MatchingInfo& info)
                {
                    if (info.status == MATCHED_MATCHING)
                        replier_.matched();
                }

            private:

                ReplyListener& operator=(const ReplyListener&) NON_COPYABLE_CXX11;

                ReqRepHelloWorldReplier &replier_;
        } request_listener_;

        class RequestListener : public PublisherListener
    {
        public:

            RequestListener(ReqRepHelloWorldReplier &replier) : replier_(replier){};
            ~RequestListener(){};
            void onPublicationMatched(Publisher* /*pub*/, MatchingInfo &info)
            {
                if (info.status == MATCHED_MATCHING)
                    replier_.matched();
            }

        private:

            RequestListener& operator=(const RequestListener&) NON_COPYABLE_CXX11;

            ReqRepHelloWorldReplier &replier_;

    } reply_listener_;

        ReqRepHelloWorldReplier();
        virtual ~ReqRepHelloWorldReplier();
        void init();
        bool isInitialized() const { return initialized_; }
        void newNumber(SampleIdentity sample_identity, uint16_t number);
        void waitDiscovery();
        void matched();
        virtual void configSubscriber(SubscriberAttributes &sattr, const std::string& suffix) = 0;
        virtual void configPublisher(PublisherAttributes &puattr, const std::string& suffix) = 0;

    private:

        ReqRepHelloWorldReplier& operator=(const ReqRepHelloWorldReplier&)NON_COPYABLE_CXX11;

        Participant *participant_;
        Subscriber *request_subscriber_;
        Publisher *reply_publisher_;
        bool initialized_;
        std::mutex mutexDiscovery_;
        std::condition_variable cvDiscovery_;
        unsigned int matched_;
        HelloWorldType type_;
};

#endif // _TEST_BLACKBOX_REQREPHELLOWORLDREPLIER_HPP_
