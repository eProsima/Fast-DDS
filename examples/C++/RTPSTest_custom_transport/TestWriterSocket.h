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

#ifndef TESTWRITERSOCKET_H_
#define TESTWRITERSOCKET_H_

#include <fastdds/rtps/participant/RTPSParticipant.h>
#include <fastdds/rtps/writer/RTPSWriter.h>


#include <string>
#include <cstdio>
#include <cstdint>

class TestWriterSocket
{
public:

    TestWriterSocket();
    virtual ~TestWriterSocket();
    eprosima::fastrtps::rtps::RTPSParticipant* mp_participant;
    eprosima::fastrtps::rtps::RTPSWriter* mp_writer;
    eprosima::fastrtps::rtps::WriterHistory* mp_history;
    bool init(
            std::string ip,
            uint32_t port);
    void run(
            uint16_t nmsgs);
};

#endif /* TESTWRITER_H_ */
