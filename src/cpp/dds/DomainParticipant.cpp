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
#include "eprosimartps/reader/StatefulReader.h"
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

Publisher* DomainParticipant::createPublisher(Participant* p,const WriterParams_t& WParam)
{
	pInfo("Creating Publisher"<<endl)
	dds::DomainParticipant *dp;
	dp = dds::DomainParticipant::getInstance();
	//Look for the correct type registration
	TypeReg_t typeR;
	std::vector<TypeReg_t>::iterator it;
	bool found = false;
	for(it=dp->typesRegistered.begin();it!=dp->typesRegistered.end();++it)
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
	if(typeR.serialize == NULL || typeR.deserialize==NULL)
	{
		pError("Serialization and deserialization functions cannot be NULL"<<endl);
		return NULL;
	}
	if(WParam.topicKind == WITH_KEY && typeR.getKey == NULL)
	{
		pError("Keyed Topic needs getKey function"<<endl);
		return NULL;
	}
	Publisher* Pub = NULL;
	if(WParam.stateKind == STATELESS)
	{
		StatelessWriter* SW;
		if(!p->createStatelessWriter(&SW,WParam,typeR.byte_size))
			return NULL;
		Pub = new Publisher((RTPSWriter*)SW);
		pDebugInfo("Publisher in topic: "<<Pub->getTopicName()<<" created."<<endl);
		SW->m_Pub = Pub;

		Pub->type = typeR;
		SW->m_type = typeR;

	}
	else if(WParam.stateKind == STATEFUL)
	{
		StatefulWriter* SF;
		if(!p->createStatefulWriter(&SF,WParam,typeR.byte_size))
			return NULL;
		Pub = new Publisher((RTPSWriter*)SF);
		SF->m_Pub = Pub;
		Pub->type = typeR;
		SF->m_type = typeR;

	}
	if(Pub!=NULL)
		{pInfo(B_YELLOW<<"PUBLISHER CREATED"<<DEF<<endl);}
	else
		{pError("Publisher not created"<<endl);}
	return Pub;
}

Subscriber* DomainParticipant::createSubscriber(Participant* p,	const ReaderParams_t& RParam) {
	//Look for the correct type registration
	pInfo("Creating Subscriber"<<endl;);
	dds::DomainParticipant *dp;
	dp = dds::DomainParticipant::getInstance();
	TypeReg_t typeR;
	std::vector<TypeReg_t>::iterator it;
	bool found = false;
	for(it=dp->typesRegistered.begin();it!=dp->typesRegistered.end();++it)
	{
		if(it->dataType == RParam.topicDataType)
		{
			typeR = *it;
			found = true;
			break;
		}
	}
	if(!found)
	{
		pError("Type Not registered"<<endl);
		return NULL;
	}
	if(typeR.serialize == NULL || typeR.deserialize==NULL)
	{
		pError("Serialization and deserialization functions cannot be NULL"<<endl);
		return NULL;
	}
	if(RParam.topicKind == WITH_KEY && typeR.getKey == NULL)
	{
		pError("Keyed Topic needs getKey function"<<endl);
		return NULL;
	}
	Subscriber* Sub = NULL;
	if(RParam.stateKind == STATELESS)
	{
		pDebugInfo("Stateless"<<endl);
		StatelessReader* SR;
		if(!p->createStatelessReader(&SR,RParam,typeR.byte_size))
		{
			pError("Error creating subscriber"<<endl);
			return NULL;
		}
		Sub = new Subscriber((RTPSReader*) SR);
		SR->mp_Sub = Sub;
		Sub->topicName = RParam.topicName;
		Sub->topicDataType = RParam.topicDataType;
		Sub->type = typeR;
	}
	else if(RParam.stateKind == STATEFUL)
	{
		pDebugInfo("Stateful"<<endl);
		StatefulReader* SFR;
		if(!p->createStatefulReader(&SFR,RParam,typeR.byte_size))
		{
			pError("Error creating subscriber"<<endl);
			return NULL;
		}
		Sub = new Subscriber((RTPSReader*) SFR);
		SFR->mp_Sub = Sub;
		Sub->topicName = RParam.topicName;
		Sub->topicDataType = RParam.topicDataType;
		Sub->type = typeR;
	}
	if(Sub!=NULL)
		{pInfo(B_YELLOW<<"SUBSCRIBER CORRECTLY CREATED"<<DEF<<endl);}
	else
		{pError("Subscriber not created"<<endl);}
	return Sub;
}

Participant* DomainParticipant::createParticipant(const ParticipantParams_t& PParam)
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
	for(it = dp->typesRegistered.begin();it!=dp->typesRegistered.end();++it)
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


