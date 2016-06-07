/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSAsNonReliableSocketWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSASNONRELIABLESOCKETWRITER_HPP_
#define _TEST_BLACKBOX_RTPSASNONRELIABLESOCKETWRITER_HPP_

#include "RTPSAsSocketWriter.hpp" 
#include <fastrtps/rtps/flowcontrol/ThroughputController.h>

class RTPSAsNonReliableSocketWriter : public RTPSAsSocketWriter
{
    public:
        void configWriter(WriterAttributes &wattr)
        {
            wattr.endpoint.reliabilityKind = BEST_EFFORT;
            wattr.terminalThroughputController = terminalThroughputController;
        }

        void configRemoteReader(RemoteReaderAttributes &/*rattr*/, GUID_t &/*ĝuid*/)
        {
        }

        std::string getText()
        {
            return "RTPSAsNonReliableSocket";
        }

        void addThroughputControllerDescriptorToWriterAttributes(uint32_t sizeToClear, uint32_t refreshTimeMS)
        {
            terminalThroughputController.sizeToClear = sizeToClear;
            terminalThroughputController.refreshTimeMS = refreshTimeMS;
        }

        ThroughputControllerDescriptor terminalThroughputController;
};

#endif // _TEST_BLACKBOX_RTPSASNONRELIABLESOCKETWRITER_HPP_

