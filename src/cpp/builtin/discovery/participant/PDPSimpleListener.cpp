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

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/eClock.h"
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
						pdata_ptr->m_guid,
						mp_SPDP->mp_participant->getEventResource(),
						boost::posix_time::milliseconds(Time_t2MilliSec(pdata_ptr->m_leaseDuration)));
				pdata_ptr->mp_leaseDurationTimer->restart_timer();
			}
			else
			{
				pdata_ptr->updateData(m_participantProxyData);
			}
			pdata_ptr->isAlive = true;
			//FIXME: PUT THIS INSIDE ASSIGN REMOTE ENDPOINTS
			for(LocatorListIterator it = pdata_ptr->m_metatrafficUnicastLocatorList.begin();
					it!=pdata_ptr->m_metatrafficUnicastLocatorList.end();++it)
			{
				this->mp_SPDP->mp_SPDPWriter->reader_locator_add(*it,pdata_ptr->m_expectsInlineQos);
			}
			for(LocatorListIterator it = pdata_ptr->m_metatrafficMulticastLocatorList.begin();
					it!=pdata_ptr->m_metatrafficMulticastLocatorList.end();++it)
			{
				this->mp_SPDP->mp_SPDPWriter->reader_locator_add(*it,pdata_ptr->m_expectsInlineQos);
			}
			//TILL HERE
			eClock::my_sleep(250);
			this->mp_SPDP->announceParticipantState(false);
			eClock::my_sleep(250);

			//Inform EDP of new participant data:
			this->mp_SPDP->mp_EDP->assignRemoteEndpoints(pdata_ptr);
			if(this->mp_SPDP->getWriterLivelinessPtr() !=NULL)
				this->mp_SPDP->getWriterLivelinessPtr()->assignRemoteEndpoints(pdata_ptr);

			//If staticEDP, perform matching:
			//FIXME: PUT HIS INSISE REMOTE ENDPOINTS FOR STATIC DISCOVERY
			if(this->mp_SPDP->m_discovery.use_STATIC_EndpointDiscoveryProtocol)
			{
				for(std::vector<RTPSReader*>::iterator it = this->mp_SPDP->mp_participant->userReadersListBegin();
						it!=this->mp_SPDP->mp_participant->userReadersListEnd();++it)
				{
					if((*it)->getUserDefinedId() > 0)
						this->mp_SPDP->mp_EDP->localReaderMatching(*it,false);
				}
				for(std::vector<RTPSWriter*>::iterator it = this->mp_SPDP->mp_participant->userWritersListBegin();
						it!=this->mp_SPDP->mp_participant->userWritersListEnd();++it)
				{
					if((*it)->getUserDefinedId() > 0)
						this->mp_SPDP->mp_EDP->localWriterMatching(*it,false);
				}
			}


			mp_SPDP->announceParticipantState(false);
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
