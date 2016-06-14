/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSAsSocketReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSASSOCKETREADER_HPP_
#define _TEST_BLACKBOX_RTPSASSOCKETREADER_HPP_

#include <fastrtps/rtps/rtps_fwd.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/rtps/common/SequenceNumber.h>

#include <list>
#include <condition_variable>

using namespace eprosima::fastrtps::rtps;

class RTPSAsSocketReader 
{
    public:
        class Listener: public ReaderListener
        {
            public:
                Listener(RTPSAsSocketReader &reader) : reader_(reader) {};
                ~Listener(){};
                void onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change);

            private:

                Listener& operator=(const Listener&)NON_COPYABLE_CXX11;

                RTPSAsSocketReader &reader_;
        } listener_;

        RTPSAsSocketReader();
        virtual ~RTPSAsSocketReader();
        void init(std::string &ip, uint32_t port, uint16_t nmsgs);
        bool isInitialized() const { return initialized_; }
        void newNumber(uint16_t number);
        std::list<uint16_t> getNonReceivedMessages();
        uint16_t lastvalue_;
        void block(uint16_t lastvalue, const std::chrono::seconds &seconds);
        virtual void configReader(ReaderAttributes &rattr) = 0;
        virtual void addRemoteWriter(RTPSReader *reader, std::string &ip, uint32_t port, GUID_t &guid) = 0;
        virtual std::string getText() = 0;

    private:

        RTPSAsSocketReader& operator=(const RTPSAsSocketReader&) NON_COPYABLE_CXX11;

        RTPSParticipant *participant_;
        RTPSReader *reader_;
        ReaderHistory *history_;
        bool initialized_;
        std::list<uint16_t> msgs_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::string text_;
        uint32_t domainId_;
        std::string hostname_;
        std::string word_;
        SequenceNumber_t last_seq;
};

#endif // _TEST_BLACKBOX_RTPSASSOCKETREADER_HPP_
