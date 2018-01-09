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
 * @file RTPSAsSocketWriter.hpp
 *
 */

#ifndef _TEST_PROFILING_RTPSASSOCKETWRITER_HPP_
#define _TEST_PROFILING_RTPSASSOCKETWRITER_HPP_

#include <fastrtps/rtps/rtps_fwd.h>
#include <fastrtps/rtps/attributes/WriterAttributes.h>

#include <string>
#include <list>



class RTPSAsSocketWriter 
{
    public:
        RTPSAsSocketWriter();
        virtual ~RTPSAsSocketWriter();
        void init(std::string ip, uint32_t port);
        bool isInitialized() const { return initialized_; }
        void send(const std::list<uint16_t> &msgs);
        virtual void configWriter(WriterAttributes &wattr) = 0;
        virtual void configRemoteReader(RemoteReaderAttributes &rattr, GUID_t &guid) = 0;
        virtual std::string getText() = 0;

    private:

        RTPSParticipant *participant_;
        RTPSWriter *writer_;
        WriterHistory *history_;
        bool initialized_;
        std::string text_;
        uint32_t domainId_;
        std::string hostname_;
};

#endif // _TEST_PROFILING_RTPSASSOCKETWRITER_HPP_
