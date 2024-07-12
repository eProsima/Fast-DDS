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
 * @file TestWriterSocket.h
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_RTPS_AS_SOCKET__TESTWRITERSOCKET_H
#define FASTDDS_EXAMPLES_CPP_RTPS_AS_SOCKET__TESTWRITERSOCKET_H

#include <cstdint>
#include <cstdio>
#include <string>

namespace eprosima {
namespace fastdds {
namespace rtps {
class RTPSParticipant;
class WriterHistory;
class RTPSWriter;
} // namespace rtps
} // namespace fastdds
} // namespace eprosima

class TestWriterSocket
{

public:

    TestWriterSocket();

    virtual ~TestWriterSocket();

    eprosima::fastdds::rtps::RTPSParticipant* mp_participant;

    eprosima::fastdds::rtps::RTPSWriter* mp_writer;

    eprosima::fastdds::rtps::WriterHistory* mp_history;

    bool init(
            std::string ip,
            uint32_t port);

    void run(
            uint16_t nmsgs);
};

#endif // FASTDDS_EXAMPLES_CPP_RTPS_AS_SOCKET__TESTWRITERSOCKET_H
