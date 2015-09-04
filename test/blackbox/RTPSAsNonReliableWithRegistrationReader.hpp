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

class RTPSAsNonReliableWithRegistrationReader : public RTPSWithRegistrationReader
{
    public:
        void configReader(ReaderAttributes &/*rattr*/, eprosima::fastrtps::ReaderQos& /*rqos*/) {};
};

#endif // _TEST_BLACKBOX_RTPSASNONRELIABLEWITHREGISTRATIONREADER_HPP_

