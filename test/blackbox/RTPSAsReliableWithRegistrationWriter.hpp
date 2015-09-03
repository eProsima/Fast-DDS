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

#ifndef _TEST_BLACKBOX_RTPSASRELIABLEWITHREGISTRATIONWRITER_HPP_
#define _TEST_BLACKBOX_RTPSASRELIABLEWITHREGISTRATIONWRITER_HPP_

#include "RTPSWithRegistrationWriter.hpp" 

class RTPSAsReliableWithRegistrationWriter : public RTPSWithRegistrationWriter
{
    public:
        void configWriter(WriterAttributes &wattr, eprosima::fastrtps::WriterQos &wqos)
        {
            wattr.endpoint.reliabilityKind = RELIABLE;
            wqos.m_reliability.kind = eprosima::fastrtps::RELIABLE_RELIABILITY_QOS;
        }
};

#endif // _TEST_BLACKBOX_RTPSASRELIABLEWITHREGISTRATIONWRITER_HPP_
