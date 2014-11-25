/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPSimpleListener.cpp
 *
 */

#include "fastrtps/rtps/builtin/discovery/endpoint/EDPSimpleListeners.h"

#include "fastrtps/rtps/builtin/discovery/endpoint/EDPSimple.h"
#include "fastrtps/rtps/builtin/discovery/participant/PDPSimple.h"
#include "fastrtps/rtps/participant/RTPSParticipantImpl.h"
#include "fastrtps/rtps/reader/StatefulReader.h"

#include "fastrtps/rtps/history/ReaderHistory.h"

#include "fastrtps/rtps/common/InstanceHandle.h"

//#include "fastrtps/rtps/builtin/data/WriterProxyData.h"
//#include "fastrtps/rtps/builtin/data/ReaderProxyData.h"
#include "fastrtps/rtps/builtin/data/ParticipantProxyData.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include "fastrtps/utils/RTPSLog.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

void EDPSimplePUBListener::onNewCacheChangeAdded(RTPSReader* reader, CacheChange_t* change)
{
	const char* const CLASS_NAME = "EDPSimplePUBListener";
	const char* const METHOD_NAME = "onNewCacheChangeAdded";
	boost::lock_guard<boost::recursive_mutex> guard(*this->mp_SEDP->mp_PubReader.first->getMutex());
	logInfo(RTPS_EDP,"");
	if(!computeKey(change))
	{
		logWarning(RTPS_EDP,"Received change with no Key");
	}
	if(change->kind == ALIVE)
	{
		//LOAD INFORMATION IN TEMPORAL WRITER PROXY DATA
		m_writerProxyData.clear();
		CDRMessage::initCDRMsg(&m_tempMsg);
		m_tempMsg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND:LITTLEEND;
		m_tempMsg.length = change->serializedPayload.length;
		memcpy(m_tempMsg.buffer,change->serializedPayload.data,m_tempMsg.length);
		if(m_writerProxyData.readFromCDRMessage(&m_tempMsg))
		{
			change->instanceHandle = m_writerProxyData.m_key;
			if(m_writerProxyData.m_guid.guidPrefix == mp_SEDP->mp_RTPSParticipant->getGuid().guidPrefix)
			{
				logInfo(RTPS_EDP,"Message from own RTPSParticipant, ignoring",C_CYAN);
				mp_SEDP->mp_PubReader.second->remove_change(change);
				return;
			}
			//LOOK IF IS AN UPDATED INFORMATION
			WriterProxyData* wdata = nullptr;
			ParticipantProxyData* pdata = nullptr;
			if(this->mp_SEDP->mp_PDP->addWriterProxyData(&m_writerProxyData,true,&wdata,&pdata)) //ADDED NEW DATA
			{
				//CHECK the locators:
				if(wdata->m_unicastLocatorList.empty() && wdata->m_multicastLocatorList.empty())
				{
					wdata->m_unicastLocatorList = pdata->m_defaultUnicastLocatorList;
					wdata->m_multicastLocatorList = pdata->m_defaultMulticastLocatorList;
				}
				wdata->m_isAlive = true;
				mp_SEDP->pairingWriterProxy(wdata);
			}
			else if(pdata == nullptr) //RTPSParticipant NOT FOUND
			{
				logWarning(RTPS_EDP,"Received message from UNKNOWN RTPSParticipant, removing");
				this->mp_SEDP->mp_PubReader.second->remove_change(change);
				return;
			}
			else //NOT ADDED BECAUSE IT WAS ALREADY THERE
			{
				for(auto ch = mp_SEDP->mp_PubReader.second->changesBegin();
						ch!=mp_SEDP->mp_PubReader.second->changesEnd();++ch)
				{
					if((*ch)->instanceHandle == change->instanceHandle)
						mp_SEDP->mp_PubReader.second->remove_change(*ch);
				}
				wdata->update(&m_writerProxyData);
				mp_SEDP->pairingWriterProxy(wdata);
			}
		}
	}
	else
	{
		//REMOVE WRITER FROM OUR READERS:
		logInfo(RTPS_EDP,"Disposed Remote Writer, removing...",C_CYAN);
		GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
		mp_SEDP->mp_PubReader.second->remove_change(change);
		this->mp_SEDP->removeWriterProxy(auxGUID);
	}
	return;
}

static inline bool compute_key(CDRMessage_t* aux_msg,CacheChange_t* change)
{
	if(change->instanceHandle == c_InstanceHandle_Unknown)
	{
		SerializedPayload_t* pl = &change->serializedPayload;
		CDRMessage::initCDRMsg(aux_msg);
		aux_msg->buffer = pl->data;
		aux_msg->length = pl->length;
		aux_msg->max_size = pl->max_size;
		aux_msg->msg_endian = pl->encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
		bool valid = false;
		uint16_t pid;
		uint16_t plength;
		while(aux_msg->pos < aux_msg->length)
		{
			valid = true;
			valid&=CDRMessage::readUInt16(aux_msg,(uint16_t*)&pid);
			valid&=CDRMessage::readUInt16(aux_msg,&plength);
			if(pid == PID_SENTINEL)
			{
				break;
			}
			if(pid == PID_KEY_HASH)
			{
				valid &= CDRMessage::readData(aux_msg,change->instanceHandle.value,16);
				return true;
			}
			if(pid == PID_ENDPOINT_GUID)
			{
				valid &= CDRMessage::readData(aux_msg,change->instanceHandle.value,16);
				return true;
			}
			aux_msg->pos+=plength;
		}
		return false;
	}
	return true;
}

bool EDPSimplePUBListener::computeKey(CacheChange_t* change)
{
	return compute_key(&aux_msg,change);
}

bool EDPSimpleSUBListener::computeKey(CacheChange_t* change)
{
	return compute_key(&aux_msg,change);
}

void EDPSimpleSUBListener::onNewCacheChangeAdded(RTPSReader* reader, CacheChange_t* change)
{
	const char* const CLASS_NAME = "EDPSimpleSUBListener";
	const char* const METHOD_NAME = "onNewCacheChangeAdded";
	boost::lock_guard<boost::recursive_mutex> guard(*this->mp_SEDP->mp_SubReader.first->getMutex());
	logInfo(RTPS_EDP,"");
	if(!computeKey(change))
	{
		logWarning(RTPS_EDP,"Received change with no Key");
	}
	if(change->kind == ALIVE)
	{
		//LOAD INFORMATION IN TEMPORAL WRITER PROXY DATA
		m_readerProxyData.clear();
		CDRMessage::initCDRMsg(&m_tempMsg);
		m_tempMsg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND:LITTLEEND;
		m_tempMsg.length = change->serializedPayload.length;
		memcpy(m_tempMsg.buffer,change->serializedPayload.data,m_tempMsg.length);
		if(m_readerProxyData.readFromCDRMessage(&m_tempMsg))
		{
			change->instanceHandle = m_readerProxyData.m_key;
			if(m_readerProxyData.m_guid.guidPrefix == mp_SEDP->mp_RTPSParticipant->getGuid().guidPrefix)
			{
				logInfo(RTPS_EDP,"From own RTPSParticipant, ignoring",C_CYAN);
				mp_SEDP->mp_SubReader.second->remove_change(change);
				return;
			}
			//LOOK IF IS AN UPDATED INFORMATION
			ReaderProxyData* rdata = nullptr;
			ParticipantProxyData* pdata = nullptr;
			if(this->mp_SEDP->mp_PDP->addReaderProxyData(&m_readerProxyData,true,&rdata,&pdata)) //ADDED NEW DATA
			{
				//CHECK the locators:
				if(rdata->m_unicastLocatorList.empty() && rdata->m_multicastLocatorList.empty())
				{
					rdata->m_unicastLocatorList = pdata->m_defaultUnicastLocatorList;
					rdata->m_multicastLocatorList = pdata->m_defaultMulticastLocatorList;
				}
				rdata->m_isAlive = true;
				mp_SEDP->pairingReaderProxy(rdata);
			}
			else if(pdata == nullptr) //RTPSParticipant NOT FOUND
			{
				logWarning(RTPS_EDP,"From UNKNOWN RTPSParticipant, removing");
				this->mp_SEDP->mp_SubReader.second->remove_change(change);
				return;
			}
			else //NOT ADDED BECAUSE IT WAS ALREADY THERE
			{
				for(auto ch = mp_SEDP->mp_SubReader.second->changesBegin();
						ch!=mp_SEDP->mp_SubReader.second->changesEnd();++ch)
				{
					if((*ch)->instanceHandle == change->instanceHandle)
						mp_SEDP->mp_SubReader.second->remove_change(*ch);
				}
				rdata->update(&m_readerProxyData);
				mp_SEDP->pairingReaderProxy(rdata);
			}
		}
	}
	else
	{
		//REMOVE WRITER FROM OUR READERS:
		logInfo(RTPS_EDP,"Disposed Remote Reader, removing...",C_CYAN);
		GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
		mp_SEDP->mp_SubReader.second->remove_change(change);
		this->mp_SEDP->removeReaderProxy(auxGUID);
	}
	return;
}

} /* namespace rtps */
}
} /* namespace eprosima */
