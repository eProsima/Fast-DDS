/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

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
};

#endif // _TEST_BLACKBOX_RTPSASRELIABLESOCKETREADER_HPP_
