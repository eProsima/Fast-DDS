/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSWithRegistrationWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSWITHREGISTRATIONWRITER_HPP_
#define _TEST_BLACKBOX_RTPSWITHREGISTRATIONWRITER_HPP_

#include <fastrtps/rtps/rtps_fwd.h>
#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/rtps/writer/WriterListener.h>
#include <fastrtps/qos/WriterQos.h>
#include <fastrtps/attributes/TopicAttributes.h>

#include <string>
#include <list>
#include <condition_variable>

using namespace eprosima::fastrtps;
using namespace ::rtps;

class RTPSWithRegistrationWriter 
{
    class Listener :public WriterListener
    {
        public:

            Listener(RTPSWithRegistrationWriter &writer) : writer_(writer){};
            ~Listener(){};
            void onWriterMatched(RTPSWriter* /*writer*/, MatchingInfo& info)
            {
                if (info.status == MATCHED_MATCHING)
                    writer_.matched();
            }

        private:

            Listener& operator=(const Listener&) NON_COPYABLE_CXX11;

            RTPSWithRegistrationWriter &writer_;

    } listener_;

    public:
        RTPSWithRegistrationWriter();
        virtual ~RTPSWithRegistrationWriter();
        void init(bool async = false);
        bool isInitialized() const { return initialized_; }
        void send(const std::list<uint16_t> &msgs);
        void matched();
        void waitDiscovery();
        virtual void configWriter(WriterAttributes &wattr, eprosima::fastrtps::WriterQos &wqos) = 0;
        virtual void configTopic(TopicAttributes &tattr) = 0;

    private:

        RTPSWithRegistrationWriter& operator=(const RTPSWithRegistrationWriter&) NON_COPYABLE_CXX11;

        RTPSParticipant *participant_;
        RTPSWriter *writer_;
        WriterHistory *history_;
        bool initialized_;
        std::mutex mutex_;
        std::condition_variable cv_;
        unsigned int matched_;
};

#endif // _TEST_BLACKBOX_RTPSWITHREGISTRATIONWRITER_HPP_
