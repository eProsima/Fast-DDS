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
 * @file RTPSAsReliableSocketWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSASRELIABLESOCKETWRITER_HPP_
#define _TEST_BLACKBOX_RTPSASRELIABLESOCKETWRITER_HPP_

#include "RTPSAsSocketWriter.hpp" 

class RTPSAsReliableSocketWriter : public RTPSAsSocketWriter
{
    public:
        void configWriter(WriterAttributes &wattr)
        {
            wattr.endpoint.reliabilityKind = RELIABLE;
            wattr.endpoint.setEntityID(2);
        }

        void configRemoteReader(RemoteReaderAttributes &rattr, GUID_t &guid)
        {
            rattr.endpoint.reliabilityKind = RELIABLE;
            rattr.guid.guidPrefix.value[0] = guid.guidPrefix.value[0];
            rattr.guid.guidPrefix.value[1] = guid.guidPrefix.value[1];
            rattr.guid.guidPrefix.value[2] = guid.guidPrefix.value[2];
            rattr.guid.guidPrefix.value[3] = guid.guidPrefix.value[3];
            rattr.guid.guidPrefix.value[4] = guid.guidPrefix.value[4];
            rattr.guid.guidPrefix.value[5] = guid.guidPrefix.value[5];
            rattr.guid.guidPrefix.value[6] = guid.guidPrefix.value[6];
            rattr.guid.guidPrefix.value[7] = guid.guidPrefix.value[7];
            rattr.guid.guidPrefix.value[8] = 1;
            rattr.guid.guidPrefix.value[9] = 0;
            rattr.guid.guidPrefix.value[10] = 0;
            rattr.guid.guidPrefix.value[11] = 0;
            rattr.guid.entityId.value[0] = 0;
            rattr.guid.entityId.value[1] = 0;
            rattr.guid.entityId.value[2] = 1;
            rattr.guid.entityId.value[3] = 4;
        }

        std::string getText()
        {
            return "RTPSAsReliableSocket";
        }
};

#endif // _TEST_BLACKBOX_RTPSASRELIABLESOCKETWRITER_HPP_
