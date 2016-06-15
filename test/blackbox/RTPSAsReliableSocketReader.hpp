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
 * @file RTPSAsReliableSocketReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSASRELIABLESOCKETREADER_HPP_
#define _TEST_BLACKBOX_RTPSASRELIABLESOCKETREADER_HPP_

#include "RTPSAsSocketReader.hpp" 

#include <fastrtps/rtps/reader/RTPSReader.h>

class RTPSAsReliableSocketReader : public RTPSAsSocketReader
{
    public:
        void configReader(ReaderAttributes &rattr)
        {
            rattr.endpoint.reliabilityKind = RELIABLE;
            rattr.endpoint.setEntityID(1);
        };

        void addRemoteWriter(RTPSReader* reader, std::string &ip, uint32_t port, GUID_t &guid)
        {
            RemoteWriterAttributes wattr;
            Locator_t loc;
            loc.set_IP4_address(ip);
            loc.port = port;
            wattr.endpoint.multicastLocatorList.push_back(loc);
            wattr.endpoint.reliabilityKind = RELIABLE;
            wattr.guid.guidPrefix.value[0] = guid.guidPrefix.value[0];
            wattr.guid.guidPrefix.value[1] = guid.guidPrefix.value[1];
            wattr.guid.guidPrefix.value[2] = guid.guidPrefix.value[2];
            wattr.guid.guidPrefix.value[3] = guid.guidPrefix.value[3];
            wattr.guid.guidPrefix.value[4] = guid.guidPrefix.value[4];
            wattr.guid.guidPrefix.value[5] = guid.guidPrefix.value[5];
            wattr.guid.guidPrefix.value[6] = guid.guidPrefix.value[6];
            wattr.guid.guidPrefix.value[7] = guid.guidPrefix.value[7];
            wattr.guid.guidPrefix.value[8] = 2;
            wattr.guid.guidPrefix.value[9] = 0;
            wattr.guid.guidPrefix.value[10] = 0;
            wattr.guid.guidPrefix.value[11] = 0;
            wattr.guid.entityId.value[0] = 0;
            wattr.guid.entityId.value[1] = 0;
            wattr.guid.entityId.value[2] = 2;
            wattr.guid.entityId.value[3] = 3;
            reader->matched_writer_add(wattr);
        }

        std::string getText()
        {
            return "RTPSAsReliableSocket";
        }
};

#endif // _TEST_BLACKBOX_RTPSASRELIABLESOCKETREADER_HPP_
