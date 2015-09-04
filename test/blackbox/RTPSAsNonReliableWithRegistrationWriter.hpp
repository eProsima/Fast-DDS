/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSAsNonReliableWithRegistrationWriter.hpp
 *
 */

#ifndef _TEST_BLACKBOX_RTPSASNONRELIABLEWITHREGISTRATIONWRITER_HPP_
#define _TEST_BLACKBOX_RTPSASNONRELIABLEWITHREGISTRATIONWRITER_HPP_

#include "RTPSWithRegistrationWriter.hpp" 

class RTPSAsNonReliableWithRegistrationWriter : public RTPSWithRegistrationWriter
{
    public:
        void configWriter(WriterAttributes &wattr, eprosima::fastrtps::WriterQos &wqos)
        {
            wattr.endpoint.reliabilityKind = BEST_EFFORT;
            wqos.m_reliability.kind = eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS;
        }
};

#endif // _TEST_BLACKBOX_RTPSASNONRELIABLEWITHREGISTRATIONWRITER_HPP_

