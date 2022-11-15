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

#include "fastdds/rtps/participant/RTPSParticipantListener.h"
#include "fastrtps/rtps/writer/WriterListener.h"

class TestWriterRegistered
{
public:

    TestWriterRegistered();
    virtual ~TestWriterRegistered();
    eprosima::fastrtps::rtps::RTPSParticipant* mp_participant;
    eprosima::fastrtps::rtps::RTPSWriter* mp_writer;
    eprosima::fastrtps::rtps::WriterHistory* mp_history;
    bool init(const bool &enable_dsp2p_lease); //Initialize writer
    bool reg(); //Register the Writer
    void run(
            uint16_t samples, uint16_t interval);  //Run the Writer
    class MyListener : public eprosima::fastrtps::rtps::WriterListener
    {
    public:

        MyListener()
            : n_matched(0)
        {
        }

        ~MyListener()
        {
        }

        void onWriterMatched(
                eprosima::fastrtps::rtps::RTPSWriter*,
                eprosima::fastrtps::rtps::MatchingInfo& info) override
        {
            if (info.status == eprosima::fastrtps::rtps::MATCHED_MATCHING)
            {
                ++n_matched;
            }
        }

        int n_matched;
    }
    m_listener;
    class MyParticipantListener : public eprosima::fastrtps::rtps::RTPSParticipantListener
    {
    public:

        MyParticipantListener()
        {
        }

        ~MyParticipantListener()
        {
        }

        void onParticipantDiscovery(
                eprosima::fastrtps::rtps::RTPSParticipant* participant, 
                eprosima::fastrtps::rtps::ParticipantDiscoveryInfo&& info) override
        {
            (void)participant;
            if (info.status == info.DISCOVERED_PARTICIPANT)
            {
                std::cout << "Participant with name " << info.info.m_participantName
                    << " and GUID " << info.info.m_guid
                    << " matched." << std::endl;
            }
            else if (info.status == info.DROPPED_PARTICIPANT)
            {
                std::cout << "Participant with name " << info.info.m_participantName
                    << " and GUID " << info.info.m_guid
                    << " dropped." << std::endl;
            }
            else if (info.status == info.REMOVED_PARTICIPANT)
            {
                std::cout << "Participant with name " << info.info.m_participantName
                    << " and GUID " << info.info.m_guid
                    << " removed." << std::endl;
            } else
            {
                std::cout << "Participant with name " << info.info.m_participantName
                    << " and GUID " << info.info.m_guid
                    << " changed QOS." << std::endl;
            }
        }
    }
    m_participant_listener;
};

#endif /* TESTWRITER_H_ */
