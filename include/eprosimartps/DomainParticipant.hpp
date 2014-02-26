/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * DomainParticipant.h
 *
 *  Created on: Feb 26, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */


#include <iostream>

using namespace std;

#ifndef DOMAINPARTICIPANT_H_
#define DOMAINPARTICIPANT_H_

namespace eprosima {
namespace rtps {

class DomainParticipant
{
private:
	uint32_t id;
    static bool instanceFlag;
    static DomainParticipant *single;
    DomainParticipant()
    {
        id = 0;//private constructor
    }
public:
    static DomainParticipant* getInstance();
    uint32_t getNewId();
    ~DomainParticipant()
    {
        instanceFlag = false;
    }
};

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



} /* namespace rtps */
} /* namespace eprosima */

#endif /* DOMAINPARTICIPANT_H_ */
