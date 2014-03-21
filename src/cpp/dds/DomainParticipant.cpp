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

#include "eprosimartps/dds/DomainParticipant.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"
#include "eprosimartps/writer/StatefulWriter.h"

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

Publisher* DomainParticipant::createPublisher(Participant* p,WriterParams_t WParam)
{
	pInfo("Creating Publisher"<<endl)
	dds::DomainParticipant *dp;
	dp = dds::DomainParticipant::getInstance();
	//Look for the correct type registration
	TypeReg_t typeR;
	std::vector<TypeReg_t>::iterator it;
	bool found = false;
	for(it=dp->typesRegistered.begin();it!=dp->typesRegistered.end();it++)
	{
		if(it->dataType == WParam.topicDataType)
		{
			typeR = *it;
			found = true;
			break;
		}
	}
	if(!found)
		return NULL;
	if(WParam.stateKind == STATELESS)
	{
		StatelessWriter* SW = new StatelessWriter();
		if(!p->createStatelessWriter(SW,WParam))
			return NULL;
		Publisher* Pub = new Publisher((RTPSWriter*)SW);
		SW->Pub = Pub;
		Pub->topicName = WParam.topicName;
		Pub->topicDataType = WParam.topicDataType;
		Pub->type = typeR;
		return Pub;
	}
	else if(WParam.stateKind == STATEFUL)
	{
		StatefulWriter* SF = new StatefulWriter();
		if(!p->createStatefulWriter(SF,WParam))
			return NULL;
		Publisher* Pub = new Publisher((RTPSWriter*)SF);
		SF->Pub = Pub;
		Pub->topicName = WParam.topicName;
		Pub->topicDataType = WParam.topicDataType;
		Pub->type = typeR;
		return Pub;
	}
	return NULL;
}

Subscriber* DomainParticipant::createSubscriber(Participant* p,	ReaderParams_t RParam) {
	//Look for the correct type registration
	dds::DomainParticipant *dp;
			dp = dds::DomainParticipant::getInstance();
	TypeReg_t typeR;
	std::vector<TypeReg_t>::iterator it;
	bool found = false;
	for(it=dp->typesRegistered.begin();it!=dp->typesRegistered.end();it++)
	{
		if(it->dataType == RParam.topicDataType)
		{
			typeR = *it;
			found = true;
			break;
		}
	}
	if(!found)
		return NULL;
	if(RParam.stateKind == STATELESS)
	{
		StatelessReader* SR= new StatelessReader();
		if(!p->createStatelessReader(SR,RParam))
			return NULL;
		Subscriber* Sub = new Subscriber((RTPSReader*) SR);
		SR->Sub = Sub;
		Sub->topicName = RParam.topicName;
		Sub->topicDataType = RParam.topicDataType;
		Sub->type = typeR;
		return Sub;
	}
	return NULL;
}

Participant* DomainParticipant::createParticipant(ParticipantParams_t PParam)
{
	Participant* p = new Participant(PParam);
	return p;
}


bool DomainParticipant::registerType(std::string in_str,
		void (*serialize)(SerializedPayload_t* data, void*),
		void (*deserialize)(SerializedPayload_t* data, void*),
		void (*getKey)(void*,InstanceHandle_t*),
		int32_t size) {
	dds::DomainParticipant *dp;
		dp = dds::DomainParticipant::getInstance();
	std::vector<TypeReg_t>::iterator it;
	for(it = dp->typesRegistered.begin();it!=dp->typesRegistered.end();it++)
	{
		if(it->dataType == in_str)
			return false;
	}

	TypeReg_t type;
	type.dataType = in_str;
	type.serialize = serialize;
	type.deserialize = deserialize;
	type.getKey = getKey;
	type.byte_size = size;
	dp->typesRegistered.push_back(type);

	return true;
}

} /* namespace dds */
} /* namespace eprosima */


