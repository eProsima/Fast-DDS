/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLPListener.cpp
 *
 */

#include "eprosimartps/builtin/liveliness/WLPListener.h"
#include "eprosimartps/builtin/liveliness/WLP.h"

#include "eprosimartps/builtin/discovery/participant/PDPSimple.h"

#include "eprosimartps/common/types/Guid.h"
#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/Participant.h"
#include "eprosimartps/reader/WriterProxyData.h"

#include "eprosimartps/reader/StatefulReader.h"

namespace eprosima {
namespace rtps {

WLPListener::WLPListener(WLP* plwp):
				mp_WLP(plwp)
{

}

WLPListener::~WLPListener()
{

}


typedef std::vector<WriterProxy*>::iterator WPIT;

void WLPListener::onNewDataMessage()
{
	boost::lock_guard<Endpoint> guard(*(Endpoint*)this->mp_WLP->mp_builtinParticipantMessageReader);
	pInfo(RTPS_MAGENTA<< "Liveliness Listener:  onNewDataMessage"<<endl);
	CacheChange_t* change;
	GuidPrefix_t guidP;
	LivelinessQosPolicyKind livelinessKind;
	while(this->mp_WLP->mp_builtinParticipantMessageReader->readNextCacheChange(&change))
	{
		//Check the serializedPayload:
		if(change->serializedPayload.length>0)
		{
			for(uint8_t i =0;i<12;++i)
			{
				guidP.value[i] = change->serializedPayload.data[i];
			}
			livelinessKind = (LivelinessQosPolicyKind)(change->serializedPayload.data[15]-0x01);
			pDebugInfo(RTPS_MAGENTA<<"Participant "<<guidP<< " assert liveliness of "<<((livelinessKind == 0x00)?"AUTOMATIC":"")<<((livelinessKind==0x01)?"MANUAL_BY_PARTICIPANT":"")<< " writers"<<endl);
		}
		else
		{
			if(!separateKey(change->instanceHandle,&guidP,&livelinessKind))
				return;
		}
		if(guidP == this->mp_WLP->mp_participant->getGuid().guidPrefix)
		{
			pDebugInfo(RTPS_MAGENTA<<"Message from own participant, ignoring"<<RTPS_DEF<<endl;);
			return;
		}
		this->mp_WLP->getBuiltinProtocols()->mp_PDP->assertRemoteWritersLiveliness(guidP,livelinessKind);
	}
	return;
}


bool WLPListener::processParameterList(ParameterList_t* param,GuidPrefix_t* guidP,LivelinessQosPolicyKind* liveliness)
{
	for(std::vector<Parameter_t*>::iterator it=param->m_parameters.begin();
			it!=param->m_parameters.end();++it)
	{
		switch((*it)->Pid)
		{
		case(PID_KEY_HASH):
			{
			ParameterKey_t* p = (ParameterKey_t*)(*it);
			return separateKey(p->key,guidP,liveliness);
			}
		default:
		{
			pWarning("WriterLivelinessListener: in this ParameterList should not be anything but the Key"<<endl;);
			break;
		}
		}
	}
	return false;
}

bool WLPListener::separateKey(InstanceHandle_t& key,GuidPrefix_t* guidP,LivelinessQosPolicyKind* liveliness)
{
	for(uint8_t i=0;i<12;++i)
	{
		guidP->value[i] = key.value[i];
	}
	*liveliness = (LivelinessQosPolicyKind)key.value[15];
	return true;
}



} /* namespace rtps */
} /* namespace eprosima */
