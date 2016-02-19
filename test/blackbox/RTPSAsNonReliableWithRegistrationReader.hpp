/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSAsNonReliableWithRegistrationReader.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSASNONRELIABLEWITHREGISTRATIONREADER_HPP_
#define _TEST_BLACKBOX_RTPSASNONRELIABLEWITHREGISTRATIONREADER_HPP_

#include "RTPSWithRegistrationReader.hpp" 
#include <boost/asio.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>

class RTPSAsNonReliableWithRegistrationReader : public RTPSWithRegistrationReader
{
    public:
        void configReader(ReaderAttributes &/*rattr*/, eprosima::fastrtps::ReaderQos& /*rqos*/) {};

        void configTopic(TopicAttributes &tattr)
        {
            tattr.topicName = "RTPSAsNonReliableWithRegistration_" + boost::asio::ip::host_name();
            tattr.topicName += "_" + boost::interprocess::ipcdetail::get_current_process_id();
        };
};

#endif // _TEST_BLACKBOX_RTPSASNONRELIABLEWITHREGISTRATIONREADER_HPP_

