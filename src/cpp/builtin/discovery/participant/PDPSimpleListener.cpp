/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PDPSimpleListener.cpp
 *
 */

#include "eprosimartps/builtin/discovery/participant/PDPSimpleListener.h"

#include "eprosimartps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.h"

#include "eprosimartps/builtin/discovery/participant/PDPSimple.h"

#include "eprosimartps/builtin/discovery/endpoint/EDP.h"
#include "eprosimartps/Participant.h"
#include "eprosimartps/reader/StatelessReader.h"
#include "eprosimartps/writer/StatelessWriter.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/eClock.h"
#include "eprosimartps/utils/TimeConversion.h"
namespace eprosima {
namespace rtps {

void PDPSimpleListener::onNewDataMessage()
{
	newAddedCache();
}

bool PDPSimpleListener::newAddedCache()
{
	boost::lock_guard<Endpoint> guard(*mp_SPDP->mp_SPDPReader);
	pInfo(RTPS_CYAN<<"SPDPListener: SPDP Message received"<<RTPS_DEF<<endl);
	CacheChange_t* change = NULL;
	if(mp_SPDP->mp_SPDPReader->get_last_added_cache(&change))
	{
		if(change->kind == ALIVE)
		{
			//LOAD INFORMATION IN TEMPORAL PARTICIPANT PROXY DATA
			m_participantProxyData.clear();
			CDRMessage_t msg;
			msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND:LITTLEEND;
			msg.length = change->serializedPayload.length;
			memcpy(msg.buffer,change->serializedPayload.data,msg.length);
			if(m_participantProxyData.readFromCDRMessage(&msg))
			{
				//AFTER CORRECTLY READING IT
				//CHECK IF IS THE SAME PARTICIPANT
				change->instanceHandle = m_participantProxyData.m_key;
				if(m_participantProxyData.m_guid == mp_SPDP->mp_participant->getGuid())
				{
					pInfo(RTPS_CYAN<<"SPDPListener: Message from own participant, ignoring"<<RTPS_DEF<<endl)
													return true;
				}
				//LOOK IF IS AN UPDATED INFORMATION
				ParticipantProxyData* pdata_ptr;
				bool found = false;
				for(std::vector<ParticipantProxyData*>::iterator it = mp_SPDP->m_participantProxies.begin();
						it != mp_SPDP->m_participantProxies.end();++it)
				{
					if(m_participantProxyData.m_key == (*it)->m_key)
					{
						found = true;
						pdata_ptr = (*it);
						break;
					}
				}
				if(!found)
				{
					//IF WE DIDNT FOUND IT WE MUST CREATE A NEW ONE
					ParticipantProxyData* pdata = new ParticipantProxyData();
					pdata->copy(m_participantProxyData);
					pdata_ptr = pdata;
					pdata_ptr->isAlive = true;
					this->mp_SPDP->m_participantProxies.push_back(pdata_ptr);
					pdata_ptr->mp_leaseDurationTimer = new RemoteParticipantLeaseDuration(mp_SPDP,
							pdata_ptr,
							mp_SPDP->mp_participant->getEventResource(),
							boost::posix_time::milliseconds(TimeConv::Time_t2MilliSecondsInt64(pdata_ptr->m_leaseDuration)));
					pdata_ptr->mp_leaseDurationTimer->restart_timer();
					mp_SPDP->assignRemoteEndpoints(pdata_ptr);
					mp_SPDP->announceParticipantState(false);
				}
				else
				{
					pdata_ptr->updateData(m_participantProxyData);
					if(mp_SPDP->m_discovery.use_STATIC_EndpointDiscoveryProtocol)
						mp_SPDP->mp_EDP->assignRemoteEndpoints(&m_participantProxyData);
				}
				pdata_ptr->isAlive = true;
			}
		}
		else
		{
			pWarning("Implement CHANGE KIND NOT ALIVE IN SPDPLISTENER"<<endl);
		}
	}
	else
	{
		pError("SPDPListener: error reading Parameters from CDRMessage"<<endl);
		return false;
	}

	return true;
}

} /* namespace rtps */
} /* namespace eprosima */
