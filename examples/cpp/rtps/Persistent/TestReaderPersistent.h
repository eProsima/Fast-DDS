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

#ifndef FASTDDS_EXAMPLES_CPP_RTPS_PERSISTENT__TESTREADERPERSISTENT_H
#define FASTDDS_EXAMPLES_CPP_RTPS_PERSISTENT__TESTREADERPERSISTENT_H

#include <fastdds/rtps/reader/ReaderListener.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {
class RTPSParticipant;
class ReaderHistory;
class RTPSReader;
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

class TestReaderPersistent
{
public:

    TestReaderPersistent();
    virtual ~TestReaderPersistent();
    eprosima::fastdds::rtps::RTPSParticipant* mp_participant;
    eprosima::fastdds::rtps::RTPSReader* mp_reader;
    eprosima::fastdds::rtps::ReaderHistory* mp_history;
    bool init(); //Initialization
    bool reg(); //Register
    void run(); //Run
    class MyListener : public eprosima::fastdds::rtps::ReaderListener
    {
    public:

        MyListener()
            : n_received(0)
            , n_matched(0)
        {
        }

        ~MyListener()
        {
        }

        void on_new_cache_change_added(
                eprosima::fastdds::rtps::RTPSReader* reader,
                const eprosima::fastdds::rtps::CacheChange_t* const change) override;
        void on_reader_matched(
                eprosima::fastdds::rtps::RTPSReader*,
                const eprosima::fastdds::rtps::MatchingInfo& info) override

        {
            if (info.status == eprosima::fastdds::rtps::MATCHED_MATCHING)
            {
                n_matched++;
            }
        }

        uint32_t n_received;
        uint32_t n_matched;
    }
    m_listener;
};

#endif // FASTDDS_EXAMPLES_CPP_RTPS_PERSISTENT__TESTREADERPERSISTENT_H
