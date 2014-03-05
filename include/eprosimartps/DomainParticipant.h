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
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include <iostream>

#include "rtps_all.h"

#include "Publisher.h"
#include "Subscriber.h"
#include "Participant.h"

#ifndef DOMAINPARTICIPANT_H_
#define DOMAINPARTICIPANT_H_


namespace eprosima {
namespace dds {



class DomainParticipant {
private:
	uint32_t id;
    static bool instanceFlag;
    static DomainParticipant *single;
    DomainParticipant()
    {
        id = 0;//private constructor
    }
    std::vector<TypeReg_t> typesRegistered;
public:
    static Publisher* createPublisher(Participant* p,WriterParams_t WParam);
    static Subscriber* createSubscriber(Participant* p,ReaderParams_t RParam);
    static bool registerType(std::string in_str, void (*serialize)(SerializedPayload_t*data,void*)
    		,void (*deserialize)(SerializedPayload_t*data,void*),int32_t size);


    static DomainParticipant* getInstance();
    uint32_t getNewId();
    ~DomainParticipant()
    {
    	instanceFlag = false;
    }
};

} /* namespace dds */
} /* namespace eprosima */

#endif /* DOMAINPARTICIPANT_H_ */
