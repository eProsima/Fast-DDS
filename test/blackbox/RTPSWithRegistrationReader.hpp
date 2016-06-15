// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file RTPSWithRegistrationReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSWITHREGISTRATIONREADER_HPP_
#define _TEST_BLACKBOX_RTPSWITHREGISTRATIONREADER_HPP_

#include <fastrtps/rtps/rtps_fwd.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/qos/ReaderQos.h>
#include <fastrtps/attributes/TopicAttributes.h>

#include <list>
#include <condition_variable>

using namespace eprosima::fastrtps;
using namespace ::rtps;

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
        virtual void configReader(ReaderAttributes &rattr, eprosima::fastrtps::ReaderQos &rqos) = 0;
        virtual void configTopic(TopicAttributes &tattr) = 0;

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
