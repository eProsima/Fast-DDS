/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

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
