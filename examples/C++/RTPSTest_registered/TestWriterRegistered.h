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
 * @file TestWriterRegistered.h
 *
 */

#ifndef TESTWRITERREGISTERED_H_
#define TESTWRITERREGISTERED_H_

#include "fastrtps/rtps/rtps_fwd.h"


#include "fastrtps/rtps/writer/WriterListener.h"

class TestWriterRegistered {
public:
    TestWriterRegistered();
    virtual ~TestWriterRegistered();
    eprosima::fastrtps::rtps::RTPSParticipant* mp_participant;
    eprosima::fastrtps::rtps::RTPSWriter* mp_writer;
    eprosima::fastrtps::rtps::WriterHistory* mp_history;
    bool init(); //Initialize writer
    bool reg(); //Register the Writer
    void run(uint16_t samples); //Run the Writer
    class MyListener :public eprosima::fastrtps::rtps::WriterListener
    {
    public:
        MyListener():n_matched(0){};
        ~MyListener(){};
        void onWriterMatched(
                eprosima::fastrtps::rtps::RTPSWriter*,
                eprosima::fastrtps::rtps::MatchingInfo& info) override
        {
            if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
                ++n_matched;
        }
        int n_matched;
    }m_listener;
};

#endif /* TESTWRITER_H_ */
