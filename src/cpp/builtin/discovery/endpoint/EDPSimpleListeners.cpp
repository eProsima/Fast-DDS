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
#include "eprosimartps/builtin/discovery/participant/PDPSimple.h"
#include "eprosimartps/Participant.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/common/types/InstanceHandle.h"


namespace eprosima {
namespace rtps {

void EDPSimplePUBReaderListener::onNewDataMessage()
{
	boost::lock_guard<Endpoint> guard(*this->mp_SEDP->mp_PubReader);
	pInfo(RTPS_CYAN<<"SEDP PUB Listener:onNewDataMessage"<<RTPS_DEF<<endl);
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
				if(m_writerProxyData.m_guid.guidPrefix == mp_SEDP->mp_participant->getGuid().guidPrefix)
				{
					pInfo(RTPS_CYAN<<"SEDP Pub Listener: Message from own participant, ignoring"<<RTPS_DEF<<endl)
							return;
				}
				//LOOK IF IS AN UPDATED INFORMATION
				WriterProxyData* wdata = NULL;
				ParticipantProxyData* pdata = NULL;
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
				else if(pdata == NULL) //PARTICIPANT NOT FOUND
				{
					pWarning("Publications Listener: received message from UNKNOWN participant, removing"<<endl);
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
			pInfo(RTPS_CYAN<<"Disposed Remote Writer, removing..."<<RTPS_DEF<<endl);
			GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
			this->mp_SEDP->removeWriterProxy(auxGUID);
		}
	}
	return;
}


void EDPSimpleSUBReaderListener::onNewDataMessage()
{
	boost::lock_guard<Endpoint> guard(*this->mp_SEDP->mp_SubReader);
	pInfo(RTPS_CYAN<<"SEDP SUB Listener:onNewDataMessage"<<RTPS_DEF<<endl);
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
				if(m_readerProxyData.m_guid.guidPrefix == mp_SEDP->mp_participant->getGuid().guidPrefix)
				{
					pInfo(RTPS_CYAN<<"SEDP Pub Listener: Message from own participant, ignoring"<<RTPS_DEF<<endl)
								return;
				}
				//LOOK IF IS AN UPDATED INFORMATION
				ReaderProxyData* rdata = NULL;
				ParticipantProxyData* pdata = NULL;
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
				else if(pdata == NULL) //PARTICIPANT NOT FOUND
				{
					pWarning("Publications Listener: received message from UNKNOWN participant, removing"<<endl);
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
			pInfo(RTPS_CYAN<<"Disposed Remote Writer, removing..."<<RTPS_DEF<<endl);
			GUID_t auxGUID = iHandle2GUID(change->instanceHandle);
			this->mp_SEDP->removeReaderProxy(auxGUID);
		}
	}
	return;
}

} /* namespace rtps */
} /* namespace eprosima */
