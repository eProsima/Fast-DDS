/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * DomainParticipant.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/DomainParticipant.h"

namespace eprosima {
namespace dds {

bool DomainParticipant::instanceFlag = false;
DomainParticipant* DomainParticipant::single = NULL;
DomainParticipant* DomainParticipant::getInstance()
{
    if(! instanceFlag)
    {
        single = new DomainParticipant();
        instanceFlag = true;
        return single;
    }
    else
    {
        return single;
    }
}

uint32_t DomainParticipant::getNewId()
{
	return ++id;
}

} /* namespace dds */
} /* namespace eprosima */
