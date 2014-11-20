/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPSimpleListener.cpp
 *
 */

#include "eprosimartps/builtin/discovery/endpoint/EDPSimpleListeners.h"

#include "eprosimartps/builtin/discovery/endpoint/EDPSimple.h"
#include "eprosimartps/builtin/discovery/RTPSParticipant/PDPSimple.h"
#include "eprosimartps/RTPSParticipant.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/common/types/InstanceHandle.h"


namespace eprosima {
namespace rtps {

void EDPSimplePUBReaderListener::onNewDataMessage()
{
	const char* const CLASS_NAME = "EDPSimplePUBReaderListener";
	const char* const METHOD_NAME = "onNewDataMessage";
	boost::lock_guard<Endpoint> guard(*this->mp_SEDP->mp_PubReader);
	logInfo(RTPS_EDP,"");
	CacheChange_t* change;
	if(this->mp_SEDP->mp_PubReader->get_last_added_cache(&change))
	{
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
					logInfo(RTPS_EDP,"Message from own RTPSParticipant, ignoring",EPRO_CYAN)
							return;
				}
				//LOOK IF IS AN UPDATED INFORMATION
				WriterProxyData* wdata = NULL;
				RTPSParticipantProxyData* pdata = NULL;
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
				else if(pdata == NULL) //RTPSParticipant NOT FOUND
				{
					logWarning(RTPS_EDP,"Received message from UNKNOWN RTPSParticipant, removing");
					this->mp_SEDP->mp_PubReader->change_removed_by_history(change);
					return;
				}
				else //NOT ADDED BECAUSE IT WAS ALREADY THERE
				{
					wdata->update(&m_writerProxyData);
					mp_SEDP->pairingWriterProxy(wdata);
				}
			}
		}
		else
		{
			//REMOVE WRITER FROM OUR READERS:
			logInfo(RTPS_EDP,"Disposed Remote Writer, removing...",EPRO_CYAN);
			GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
			this->mp_SEDP->removeWriterProxy(auxGUID);
		}
	}
	return;
}


void EDPSimpleSUBReaderListener::onNewDataMessage()
{
	const char* const CLASS_NAME = "EDPSimpleSUBReaderListener";
	const char* const METHOD_NAME = "onNewDataMessage";
	boost::lock_guard<Endpoint> guard(*this->mp_SEDP->mp_SubReader);
	logInfo(RTPS_EDP,"");
	CacheChange_t* change;
	if(this->mp_SEDP->mp_SubReader->get_last_added_cache(&change))
	{
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
					logInfo(RTPS_EDP,"From own RTPSParticipant, ignoring",EPRO_CYAN);
					return;
				}
				//LOOK IF IS AN UPDATED INFORMATION
				ReaderProxyData* rdata = NULL;
				RTPSParticipantProxyData* pdata = NULL;
				if(this->mp_SEDP->mp_PDP->addReaderProxyData(&m_readerProxyData,true,&rdata,&pdata)) //ADDED NEW DATA
				{
					//CHECK the locators:
					if(rdata->m_unicastLocatorList.empty())
						rdata->m_unicastLocatorList = pdata->m_defaultUnicastLocatorList;
					if(rdata->m_multicastLocatorList.empty())
						rdata->m_multicastLocatorList = pdata->m_defaultMulticastLocatorList;
					rdata->m_isAlive = true;
					mp_SEDP->pairingReaderProxy(rdata);
				}
				else if(pdata == NULL) //RTPSParticipant NOT FOUND
				{
					logWarning(RTPS_EDP,"From UNKNOWN RTPSParticipant, removing");
					this->mp_SEDP->mp_PubReader->change_removed_by_history(change);
					return;
				}
				else //NOT ADDED BECAUSE IT WAS ALREADY THERE
				{
					rdata->update(&m_readerProxyData);
					mp_SEDP->pairingReaderProxy(rdata);
				}
			}
		}
		else
		{
			//REMOVE WRITER FROM OUR READERS:
			logInfo(RTPS_EDP,"Disposed Remote Reader, removing...",EPRO_CYAN);
			GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
			this->mp_SEDP->removeReaderProxy(auxGUID);
		}
	}
	return;
}

} /* namespace rtps */
} /* namespace eprosima */
