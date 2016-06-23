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
 * @file RTPSAsNonReliableSocketWriter.hpp
 *
 */

#ifndef _TEST_PROFILING_RTPSASNONRELIABLESOCKETWRITER_HPP_
#define _TEST_PROFILING_RTPSASNONRELIABLESOCKETWRITER_HPP_

#include "RTPSAsSocketWriter.hpp" 

class RTPSAsNonReliableSocketWriter : public RTPSAsSocketWriter
{
    public:
        void configWriter(WriterAttributes &wattr)
        {
            wattr.endpoint.reliabilityKind = BEST_EFFORT;
        }

        void configRemoteReader(RemoteReaderAttributes &/*rattr*/, GUID_t &/*ĝuid*/)
        {
        }

        std::string getText()
        {
            return "RTPSAsNonReliableSocket";
        }
};

#endif // _TEST_PROFILING_RTPSASNONRELIABLESOCKETWRITER_HPP_

