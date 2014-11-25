/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PDPSimpleListener.cpp
 *
 */

#include "fastrtps/rtps/builtin/discovery/participant/PDPSimpleListener.h"

//#include "fastrtps/rtps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.h"
//
#include "fastrtps/rtps/builtin/discovery/participant/PDPSimple.h"
#include "fastrtps/rtps/participant/RTPSParticipantImpl.h"
//
//#include "fastrtps/builtin/discovery/endpoint/EDP.h"
//#include "fastrtps/RTPSParticipant.h"
//#include "fastrtps/reader/StatelessReader.h"
//#include "fastrtps/writer/StatelessWriter.h"
//
//#include "fastrtps/utils/RTPSLog.h"
//#include "fastrtps/utils/eClock.h"
//#include "fastrtps/utils/TimeConversion.h"
//
//#include "fastrtps/pubsub/RTPSParticipantDiscoveryInfo.h"
//#include "fastrtps/pubsub/RTPSParticipantListener.h"


#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include "fastrtps/utils/RTPSLog.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "PDPSimpleListener";


void PDPSimpleListener::onNewCacheChangeAdded(RTPSReader* reader,CacheChange_t* change)
{
	const char* const METHOD_NAME = "newAddedCache";
	boost::lock_guard<boost::recursive_mutex> guard(*reader->getMutex());
	logInfo(RTPS_PDP,"SPDP Message received",C_CYAN);
	if(change->kind == ALIVE)
	{
		//LOAD INFORMATION IN TEMPORAL RTPSParticipant PROXY DATA
		m_ParticipantProxyData.clear();
		CDRMessage_t msg;
		msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND:LITTLEEND;
		msg.length = change->serializedPayload.length;
		memcpy(msg.buffer,change->serializedPayload.data,msg.length);
		if(m_ParticipantProxyData.readFromCDRMessage(&msg))
		{
			//AFTER CORRECTLY READING IT
			//CHECK IF IS THE SAME RTPSParticipant
			change->instanceHandle = m_ParticipantProxyData.m_key;
			if(m_ParticipantProxyData.m_guid == mp_SPDP->getParticipant()->getGuid())
			{
				logInfo(RTPS_PDP,"Message from own RTPSParticipant, ignoring",C_CYAN)
						return;
			}
			//LOOK IF IS AN UPDATED INFORMATION
			ParticipantProxyData* pdata_ptr;
			bool found = false;
			for(auto it : mp_SPDP->m_participantProxies)
			{
				if(m_ParticipantProxyData.m_key == (*it)->m_key)
				{
					found = true;
					pdata_ptr = (*it);
					break;
				}
			}
			RTPSParticipantDiscoveryInfo info;
			info.m_guid = m_ParticipantProxyData.m_guid;
			info.m_RTPSParticipantName = m_ParticipantProxyData.m_RTPSParticipantName;
			info.m_propertyList = m_ParticipantProxyData.m_properties.properties;
			info.m_userData = m_ParticipantProxyData.m_userData;
			if(!found)
			{
				info.m_status = DISCOVERED_PARTICIPANT;
				//IF WE DIDNT FOUND IT WE MUST CREATE A NEW ONE
				ParticipantProxyData* pdata = new ParticipantProxyData();
				pdata->copy(m_ParticipantProxyData);
				pdata_ptr = pdata;
				pdata_ptr->isAlive = true;
				this->mp_SPDP->m_participantProxies.push_back(pdata_ptr);
				pdata_ptr->mp_leaseDurationTimer = new RemoteParticipantLeaseDuration(mp_SPDP,
						pdata_ptr,
						mp_SPDP->mp_RTPSParticipant->getEventResource(),
						boost::posix_time::milliseconds(TimeConv::Time_t2MilliSecondsInt64(pdata_ptr->m_leaseDuration)));
				pdata_ptr->mp_leaseDurationTimer->restart_timer();
				mp_SPDP->assignRemoteEndpoints(pdata_ptr);
				mp_SPDP->announceRTPSParticipantState(false);
			}
			else
			{
				info.m_status = CHANGED_QOS_PARTICIPANT;
				pdata_ptr->updateData(m_ParticipantProxyData);
				if(mp_SPDP->m_discovery.use_STATIC_EndpointDiscoveryProtocol)
					mp_SPDP->mp_EDP->assignRemoteEndpoints(&m_ParticipantProxyData);
			}
			if(this->mp_SPDP->getRTPSParticipant()->getListener()!=NULL)
				this->mp_SPDP->getRTPSParticipant()->getListener()->onRTPSParticipantDiscovery(
						this->mp_SPDP->getRTPSParticipant()->getUserRTPSParticipant(),
						info);
			pdata_ptr->isAlive = true;
		}
	}
	else
	{
		//pWarning("Implement CHANGE KIND NOT ALIVE IN SPDPLISTENER"<<endl);
		GUID_t guid;
		iHandle2GUID(guid,change->instanceHandle);
		this->mp_SPDP->removeRemoteParticipant(guid);
		RTPSParticipantDiscoveryInfo info;
		info.m_status = REMOVED_PARTICIPANT;
		info.m_guid = guid;
		if(this->mp_SPDP->getRTPSParticipantImpl()->getListener()!=nullptr)
			this->mp_SPDP->getRTPSParticipantImpl()->getListener()->onRTPSParticipantDiscovery(
					this->mp_SPDP->getRTPSParticipant()->getUserRTPSParticipant(),
					info);
	}
	//}
	//else
	//{
	//	logError(RTPS_PDP,"Error reading Parameters from CDRMessage",C_CYAN);
	//	return false;
	//}

	return;
}

}
} /* namespace rtps */
} /* namespace eprosima */
