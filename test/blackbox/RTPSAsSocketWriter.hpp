/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSAsSocketWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSASSOCKETWRITER_HPP_
#define _TEST_BLACKBOX_RTPSASSOCKETWRITER_HPP_

#include <fastrtps/rtps/rtps_fwd.h>
#include <fastrtps/rtps/attributes/WriterAttributes.h>

#include <string>
#include <list>

using namespace eprosima::fastrtps::rtps;

class RTPSAsSocketWriter 
{
    public:
        RTPSAsSocketWriter();
        virtual ~RTPSAsSocketWriter();
        void init(std::string ip, uint32_t port, bool async = false);
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

#endif // _TEST_BLACKBOX_RTPSASSOCKETWRITER_HPP_
