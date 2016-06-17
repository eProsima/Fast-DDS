/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSAsNonReliableSocketReader.hpp
 *
 */

#ifndef _TEST_PROFILING_RTPSASNONRELIABLESOCKETREADER_HPP_
#define _TEST_PROFILING_RTPSASNONRELIABLESOCKETREADER_HPP_

#include "RTPSAsSocketReader.hpp" 

class RTPSAsNonReliableSocketReader : public RTPSAsSocketReader
{
    public:
        void configReader(ReaderAttributes &/*rattr*/) {};

        void addRemoteWriter(RTPSReader* /*reader*/, std::string &/*ip*/, uint32_t /*port*/, GUID_t &/*guid*/) {}

        std::string getText()
        {
            return "RTPSAsNonReliableSocket";
        }
};

#endif // _TEST_PROFILING_RTPSASNONRELIABLESOCKETREADER_HPP_

