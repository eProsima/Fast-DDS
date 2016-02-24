/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PDPSimpleListener.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimpleListener.h>

#include <fastrtps/rtps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.h>

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include "../../../participant/RTPSParticipantImpl.h"

#include <fastrtps/rtps/builtin/discovery/endpoint/EDP.h>
#include <fastrtps/rtps/reader/RTPSReader.h>

#include <fastrtps/rtps/history/ReaderHistory.h>

#include <fastrtps/utils/TimeConversion.h>
//
#include <fastrtps/rtps/participant/RTPSParticipantDiscoveryInfo.h>
#include <fastrtps/rtps/participant/RTPSParticipantListener.h>


#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <fastrtps/utils/RTPSLog.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "PDPSimpleListener";


void PDPSimpleListener::onNewCacheChangeAdded(RTPSReader* reader, const CacheChange_t* const change_in)
{
    const char* const METHOD_NAME = "onNewCacheChangeAdded";
    CacheChange_t* change = (CacheChange_t*)(change_in);
    boost::lock_guard<boost::recursive_mutex> rguard(*reader->getMutex());
    logInfo(RTPS_PDP,"SPDP Message received",C_CYAN);
    if(change->instanceHandle == c_InstanceHandle_Unknown)
    {
        if(!this->getKey(change))
        {
            logWarning(RTPS_PDP,"Problem getting the key of the change, removing",C_CYAN);
            this->mp_SPDP->mp_SPDPReaderHistory->remove_change(change);
            return;
        }
    }
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
            if(m_ParticipantProxyData.m_guid == mp_SPDP->getRTPSParticipant()->getGuid())
            {
                logInfo(RTPS_PDP,"Message from own RTPSParticipant, removing",C_CYAN);
                this->mp_SPDP->mp_SPDPReaderHistory->remove_change(change);
                return;
            }
            //LOOK IF IS AN UPDATED INFORMATION
            ParticipantProxyData* pdata_ptr = nullptr;
            bool found = false;
            boost::lock_guard<boost::recursive_mutex> guard(*mp_SPDP->getMutex());
            for (auto it = mp_SPDP->m_participantProxies.begin();
                    it != mp_SPDP->m_participantProxies.end();++it)
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
            info.m_RTPSParticipantName = m_ParticipantProxyData.m_participantName;
            info.m_propertyList = m_ParticipantProxyData.m_properties.properties;
            info.m_userData = m_ParticipantProxyData.m_userData;
            if(!found)
            {
                info.m_status = DISCOVERED_RTPSPARTICIPANT;
                //IF WE DIDNT FOUND IT WE MUST CREATE A NEW ONE
                ParticipantProxyData* pdata = new ParticipantProxyData();
                boost::lock_guard<boost::recursive_mutex> pguard(*pdata->mp_mutex);
                pdata->copy(m_ParticipantProxyData);
                pdata_ptr = pdata;
                pdata_ptr->isAlive = true;
                pdata_ptr->mp_leaseDurationTimer = new RemoteParticipantLeaseDuration(mp_SPDP,
                        pdata_ptr,
                        TimeConv::Time_t2MilliSecondsDouble(pdata_ptr->m_leaseDuration));
                pdata_ptr->mp_leaseDurationTimer->restart_timer();
                this->mp_SPDP->m_participantProxies.push_back(pdata_ptr);
                mp_SPDP->assignRemoteEndpoints(pdata_ptr);
                mp_SPDP->announceParticipantState(false);
            }
            else
            {
                info.m_status = CHANGED_QOS_RTPSPARTICIPANT;
                boost::lock_guard<boost::recursive_mutex> pguard(*pdata_ptr->mp_mutex);
                pdata_ptr->updateData(m_ParticipantProxyData);
                if(mp_SPDP->m_discovery.use_STATIC_EndpointDiscoveryProtocol)
                    mp_SPDP->mp_EDP->assignRemoteEndpoints(&m_ParticipantProxyData);
            }
            if(this->mp_SPDP->getRTPSParticipant()->getListener()!=nullptr)
                this->mp_SPDP->getRTPSParticipant()->getListener()->onRTPSParticipantDiscovery(
                        this->mp_SPDP->getRTPSParticipant()->getUserRTPSParticipant(),
                        info);
            pdata_ptr->isAlive = true;
        }
    }
    else
    {
        GUID_t guid;
        iHandle2GUID(guid,change->instanceHandle);
        this->mp_SPDP->removeRemoteParticipant(guid);
        RTPSParticipantDiscoveryInfo info;
        info.m_status = REMOVED_RTPSPARTICIPANT;
        info.m_guid = guid;
        if(this->mp_SPDP->getRTPSParticipant()->getListener()!=nullptr)
            this->mp_SPDP->getRTPSParticipant()->getListener()->onRTPSParticipantDiscovery(
                    this->mp_SPDP->getRTPSParticipant()->getUserRTPSParticipant(),
                    info);
    }
    
    //Remove change form history.
    this->mp_SPDP->mp_SPDPReaderHistory->remove_change(change);

	return;
}

bool PDPSimpleListener::getKey(CacheChange_t* change)
{
	SerializedPayload_t* pl = &change->serializedPayload;
	CDRMessage::initCDRMsg(&aux_msg);
    // TODO CHange because it create a buffer to remove after.
    free(aux_msg.buffer);
	aux_msg.buffer = pl->data;
	aux_msg.length = pl->length;
	aux_msg.max_size = pl->max_size;
	aux_msg.msg_endian = pl->encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
	bool valid = false;
	uint16_t pid;
	uint16_t plength;
	while(aux_msg.pos < aux_msg.length)
	{
		valid = true;
		valid&=CDRMessage::readUInt16(&aux_msg,(uint16_t*)&pid);
		valid&=CDRMessage::readUInt16(&aux_msg,&plength);
		if(pid == PID_SENTINEL)
		{
			break;
		}
		if(pid == PID_PARTICIPANT_GUID)
		{
			valid &= CDRMessage::readData(&aux_msg,change->instanceHandle.value,16);
			aux_msg.buffer = nullptr;
			return true;
		}
		if(pid == PID_KEY_HASH)
		{
			valid &= CDRMessage::readData(&aux_msg,change->instanceHandle.value,16);
			aux_msg.buffer = nullptr;
			return true;
		}
		aux_msg.pos+=plength;
	}
	aux_msg.buffer = nullptr;
	return false;
}



}
} /* namespace rtps */
} /* namespace eprosima */
