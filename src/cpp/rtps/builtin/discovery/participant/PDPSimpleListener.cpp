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

#include "fastrtps/builtin/discovery/RTPSParticipant/PDPSimpleListener.h"

#include "fastrtps/builtin/discovery/RTPSParticipant/timedevent/RemoteRTPSParticipantLeaseDuration.h"

#include "fastrtps/builtin/discovery/RTPSParticipant/PDPSimple.h"

#include "fastrtps/builtin/discovery/endpoint/EDP.h"
#include "fastrtps/RTPSParticipant.h"
#include "fastrtps/reader/StatelessReader.h"
#include "fastrtps/writer/StatelessWriter.h"

#include "fastrtps/utils/RTPSLog.h"
#include "fastrtps/utils/eClock.h"
#include "fastrtps/utils/TimeConversion.h"

#include "fastrtps/pubsub/RTPSParticipantDiscoveryInfo.h"
#include "fastrtps/pubsub/RTPSParticipantListener.h"

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "PDPSimpleListener";

void PDPSimpleListener::onNewDataMessage()
{
	newAddedCache();
}

bool PDPSimpleListener::newAddedCache()
{
	const char* const METHOD_NAME = "newAddedCache";
	boost::lock_guard<Endpoint> guard(*mp_SPDP->mp_SPDPReader);
	logInfo(RTPS_PDP,"SPDP Message received",EPRO_CYAN);
	CacheChange_t* change = NULL;
	if(mp_SPDP->mp_SPDPReader->get_last_added_cache(&change))
	{
		if(change->kind == ALIVE)
		{
			//LOAD INFORMATION IN TEMPORAL RTPSParticipant PROXY DATA
			m_RTPSParticipantProxyData.clear();
			CDRMessage_t msg;
			msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND:LITTLEEND;
			msg.length = change->serializedPayload.length;
			memcpy(msg.buffer,change->serializedPayload.data,msg.length);
			if(m_RTPSParticipantProxyData.readFromCDRMessage(&msg))
			{
				//AFTER CORRECTLY READING IT
				//CHECK IF IS THE SAME RTPSParticipant
				change->instanceHandle = m_RTPSParticipantProxyData.m_key;
				if(m_RTPSParticipantProxyData.m_guid == mp_SPDP->mp_RTPSParticipant->getGuid())
				{
					logInfo(RTPS_PDP,"Message from own RTPSParticipant, ignoring",EPRO_CYAN)
					return true;
				}
				//LOOK IF IS AN UPDATED INFORMATION
				RTPSParticipantProxyData* pdata_ptr;
				bool found = false;
				for(std::vector<RTPSParticipantProxyData*>::iterator it = mp_SPDP->m_RTPSParticipantProxies.begin();
						it != mp_SPDP->m_RTPSParticipantProxies.end();++it)
				{
					if(m_RTPSParticipantProxyData.m_key == (*it)->m_key)
					{
						found = true;
						pdata_ptr = (*it);
						break;
					}
				}
				RTPSParticipantDiscoveryInfo info;
				info.m_guid = m_RTPSParticipantProxyData.m_guid;
				info.m_RTPSParticipantName = m_RTPSParticipantProxyData.m_RTPSParticipantName;
				info.m_propertyList = m_RTPSParticipantProxyData.m_properties.properties;
				info.m_userData = m_RTPSParticipantProxyData.m_userData;
				if(!found)
				{
					info.m_status = DISCOVERED_RTPSParticipant;
					//IF WE DIDNT FOUND IT WE MUST CREATE A NEW ONE
					RTPSParticipantProxyData* pdata = new RTPSParticipantProxyData();
					pdata->copy(m_RTPSParticipantProxyData);
					pdata_ptr = pdata;
					pdata_ptr->isAlive = true;
					this->mp_SPDP->m_RTPSParticipantProxies.push_back(pdata_ptr);
					pdata_ptr->mp_leaseDurationTimer = new RemoteRTPSParticipantLeaseDuration(mp_SPDP,
							pdata_ptr,
							mp_SPDP->mp_RTPSParticipant->getEventResource(),
							boost::posix_time::milliseconds(TimeConv::Time_t2MilliSecondsInt64(pdata_ptr->m_leaseDuration)));
					pdata_ptr->mp_leaseDurationTimer->restart_timer();
					mp_SPDP->assignRemoteEndpoints(pdata_ptr);
					mp_SPDP->announceRTPSParticipantState(false);
				}
				else
				{
					info.m_status = CHANGED_QOS_RTPSParticipant;
					pdata_ptr->updateData(m_RTPSParticipantProxyData);
					if(mp_SPDP->m_discovery.use_STATIC_EndpointDiscoveryProtocol)
						mp_SPDP->mp_EDP->assignRemoteEndpoints(&m_RTPSParticipantProxyData);
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
			this->mp_SPDP->removeRemoteRTPSParticipant(guid);
			RTPSParticipantDiscoveryInfo info;
			info.m_status = REMOVED_RTPSParticipant;
			info.m_guid = guid;
			if(this->mp_SPDP->getRTPSParticipant()->getListener()!=NULL)
				this->mp_SPDP->getRTPSParticipant()->getListener()->onRTPSParticipantDiscovery(
									this->mp_SPDP->getRTPSParticipant()->getUserRTPSParticipant(),
									info);
		}
	}
	else
	{
		logError(RTPS_PDP,"Error reading Parameters from CDRMessage",EPRO_CYAN);
		return false;
	}

	return true;
}

} /* namespace rtps */
} /* namespace eprosima */
