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
 * @file TestReaderPersistent.h
 *
 */

#ifndef TESTREADERPERSISTENT_H_
#define TESTREADERPERSISTENT_H_

#include <fastrtps/rtps/rtps_fwd.h>


#include <fastrtps/rtps/reader/ReaderListener.h>

class TestReaderPersistent
{
public:
    TestReaderPersistent();
    virtual ~TestReaderPersistent();
    eprosima::fastrtps::rtps::RTPSParticipant* mp_participant;
    eprosima::fastrtps::rtps::RTPSReader* mp_reader;
    eprosima::fastrtps::rtps::ReaderHistory* mp_history;
    bool init(); //Initialization
    bool reg(); //Register
    void run(); //Run
    class MyListener:public eprosima::fastrtps::rtps::ReaderListener
    {
    public:
        MyListener():n_received(0),n_matched(0){};
        ~MyListener(){};
        void onNewCacheChangeAdded(
                eprosima::fastrtps::rtps::RTPSReader* reader,
                const eprosima::fastrtps::rtps::CacheChange_t* const change) override;
        void onReaderMatched(
                eprosima::fastrtps::rtps::RTPSReader*,
                eprosima::fastrtps::rtps::MatchingInfo& info) override
        {
            if(info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING) n_matched++;
        };
        uint32_t n_received;
        uint32_t n_matched;
    }m_listener;
};

#endif /* TESTREADERPERSISTENT_H_ */
