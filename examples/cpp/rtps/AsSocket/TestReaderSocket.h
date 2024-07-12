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
 * @file TestReaderSocket.h
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_RTPS_AS_SOCKET__TESTREADERSOCKET_H
#define FASTDDS_EXAMPLES_CPP_RTPS_AS_SOCKET__TESTREADERSOCKET_H

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

class TestReaderSocket
{
public:

    TestReaderSocket();

    virtual ~TestReaderSocket();

    eprosima::fastdds::rtps::RTPSParticipant* mp_participant;

    eprosima::fastdds::rtps::RTPSReader* mp_reader;

    eprosima::fastdds::rtps::ReaderHistory* mp_history;

    bool init(
            std::string ip,
            uint32_t port);

    void run();

    class MyListener : public eprosima::fastdds::rtps::ReaderListener
    {
    public:

        MyListener()
            : m_received(0)
        {
        }

        ~MyListener()
        {
        }

        void on_new_cache_change_added(
                eprosima::fastdds::rtps::RTPSReader* reader,
                const eprosima::fastdds::rtps::CacheChange_t* const change) override;

        uint32_t m_received;
    }
    m_listener;

};

#endif // FASTDDS_EXAMPLES_CPP_RTPS_AS_SOCKET__TESTREADERSOCKET_H
