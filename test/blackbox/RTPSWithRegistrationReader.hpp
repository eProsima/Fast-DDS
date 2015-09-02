/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSWithRegistrationReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSWITHREGISTRATIONREADER_HPP_
#define _TEST_BLACKBOX_RTPSWITHREGISTRATIONREADER_HPP_

#include <fastrtps/rtps/rtps_fwd.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>

#include <list>
#include <condition_variable>

using namespace eprosima::fastrtps::rtps;

class RTPSWithRegistrationReader 
{
    public:
        class Listener: public ReaderListener
        {
            public:
                Listener(RTPSWithRegistrationReader &reader) : reader_(reader) {};
                ~Listener(){};
                void onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change);
                void onReaderMatched(RTPSReader* /*reader*/, MatchingInfo& info)
                {
                    if (info.status == MATCHED_MATCHING)
                        reader_.matched();
                }

            private:

                Listener& operator=(const Listener&) NON_COPYABLE_CXX11;

                RTPSWithRegistrationReader &reader_;
        } listener_;

        RTPSWithRegistrationReader();
        virtual ~RTPSWithRegistrationReader();
        void init(uint32_t port, uint16_t nmsgs);
        bool isInitialized() const { return initialized_; }
        void newNumber(uint16_t number);
        std::list<uint16_t> getNonReceivedMessages();
        uint16_t lastvalue_;
        void block(uint16_t lastvalue, const std::chrono::seconds &seconds);
        void waitDiscovery();
        void matched();
        virtual void configReader(ReaderAttributes &rattr) = 0;

    private:

        RTPSWithRegistrationReader& operator=(const RTPSWithRegistrationReader&) NON_COPYABLE_CXX11;

        RTPSParticipant *participant_;
        RTPSReader *reader_;
        ReaderHistory *history_;
        bool initialized_;
        std::list<uint16_t> msgs_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::mutex mutexDiscovery_;
        std::condition_variable cvDiscovery_;
        unsigned int matched_;
};

#endif // _TEST_BLACKBOX_RTPSWITHREGISTRATIONREADER_HPP_
