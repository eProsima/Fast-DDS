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
	pInfo(RTPS_MAGENTA<< "Liveliness Reader:  onNewDataMessage"<<endl);
	CacheChange_t* change;
	GuidPrefix_t guidP;
	LivelinessQosPolicyKind livelinessKind;
	if(this->mp_WLP->mp_builtinParticipantMessageReader->get_last_added_cache(&change))
	{
		//Check the serializedPayload:
		if(change->serializedPayload.length>0)
		{
			for(uint8_t i =0;i<12;++i)
			{
				guidP.value[i] = change->serializedPayload.data[i];
			}
			livelinessKind = (LivelinessQosPolicyKind)change->serializedPayload.data[15];

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

		for(std::vector<RTPSReader*>::iterator rit = this->mp_WLP->mp_participant->userReadersListBegin();
				rit!=this->mp_WLP->mp_participant->userReadersListEnd();++rit)
		{
			if((*rit)->getStateType() == STATEFUL)
			{
				StatefulReader* SFR = (StatefulReader*)(*rit);
				for(std::vector<WriterProxy*>::iterator wit = SFR->MatchedWritersBegin();
						wit!=SFR->MatchedWritersEnd();++wit)
				{
					if((*wit)->m_data->m_qos.m_liveliness.kind == (livelinessKind-0x01))
					{
						if((*wit)->m_data->m_guid.guidPrefix == guidP)
						{
							pDebugInfo(RTPS_MAGENTA<<"Asserting liveliness of Writer: "<< (*wit)->m_data->m_guid<<RTPS_DEF<<endl;);
							(*wit)->assertLiveliness();
						}
					}
				}
			}
		}
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
