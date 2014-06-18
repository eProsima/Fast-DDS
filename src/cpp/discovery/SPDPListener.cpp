/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SPDPListener.cpp
 *
*/

#include "eprosimartps/discovery/SPDPListener.h"
#include "eprosimartps/discovery/data/DiscoveredParticipantData.h"
#include "eprosimartps/discovery/SimplePDP.h"
#include "eprosimartps/discovery/timedevent/ParticipantLeaseDuration.h"
#include "eprosimartps/Participant.h"
#include "eprosimartps/reader/StatelessReader.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/liveliness/WriterLiveliness.h"
#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/eClock.h"

namespace eprosima {
namespace rtps {

void SPDPListener::onNewDataMessage()
{
	newAddedCache();
}

bool SPDPListener::newAddedCache()
{
	boost::lock_guard<Endpoint> guard(*mp_SPDP->mp_SPDPReader);
	pInfo(CYAN<<"SPDPListener: SPDP Message received"<<DEF<<endl);
	CacheChange_t* change = NULL;
	if(mp_SPDP->mp_SPDPReader->get_last_added_cache(&change))
	{
		ParameterList_t param;
		CDRMessage_t msg;
		msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND:LITTLEEND;
		msg.length = change->serializedPayload.length;
		memcpy(msg.buffer,change->serializedPayload.data,msg.length);
		if(ParameterList::readParameterListfromCDRMsg(&msg,&param,&change->instanceHandle,NULL)>0)
		{
			DiscoveredParticipantData* pdata = new DiscoveredParticipantData();

			if(processParameterList(param,pdata))
			{
				change->instanceHandle = pdata->m_key;
			//	bool already_in_history = false;
				//Check if CacheChange_t with same Key is already in History:
				for(std::vector<CacheChange_t*>::iterator it = mp_SPDP->mp_SPDPReader->readerHistoryCacheBegin();
						it!=mp_SPDP->mp_SPDPReader->readerHistoryCacheEnd();++it)
				{
					if((*it)->instanceHandle == change->instanceHandle && (*it)->sequenceNumber.to64long() < change->sequenceNumber.to64long())
					{
						pDebugInfo("SEDP Subscription Listener:Removing older change with the same iHandle"<<endl);
						mp_SPDP->mp_SPDPReader->remove_change((*it)->sequenceNumber,(*it)->writerGUID);
					//	already_in_history = true;
						break;
					}
				}
				if(pdata->m_guidPrefix == mp_SPDP->mp_localDPData->m_guidPrefix)
				{
					//cout << "SMAE"<<endl;
					pInfo(CYAN<<"SPDPListener: Message from own participant, ignoring"<<DEF<<endl)
					//mp_SPDP->mp_SPDPReader->remove_change(change->sequenceNumber,change->writerGUID);
					return true;
				}
				//Look for the participant in my own list:
				DiscoveredParticipantData* pdata_ptr;
				bool found = false;
				for(std::vector<DiscoveredParticipantData*>::iterator it = mp_SPDP->m_discoveredParticipants.begin();
						it != mp_SPDP->m_discoveredParticipants.end();++it)
				{
					if(change->instanceHandle == (*it)->m_key)
					{
						found = true;
						pdata_ptr = (*it);
						delete(pdata);
						break;
					}
				}
				if(!found)
				{
					pdata_ptr = pdata;
					pdata_ptr->isAlive = true;
					this->mp_SPDP->m_discoveredParticipants.push_back(pdata_ptr);
					ParticipantLeaseDuration* pld = new ParticipantLeaseDuration((ParticipantDiscoveryProtocol*)mp_SPDP,
																				pdata_ptr->m_guidPrefix,
																				mp_SPDP->mp_participant->getEventResource(),
																				boost::posix_time::milliseconds(Time_t2MilliSec(pdata_ptr->leaseDuration)));
					this->mp_SPDP->m_discoveredParticipantsLeaseDurations.push_back(pld);
				}
				pdata_ptr->isAlive = true;
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
				eClock::my_sleep(250);
				this->mp_SPDP->announceParticipantState(false);
				eClock::my_sleep(250);
				//Inform EDP of new participant data:
				this->mp_SPDP->mp_EDP->assignRemoteEndpoints(pdata_ptr);
				if(this->mp_SPDP->getWriterLivelinessPtr() !=NULL)
					this->mp_SPDP->getWriterLivelinessPtr()->assignRemoteEndpoints(pdata_ptr);

				//If staticEDP, perform matching:
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
			param.deleteParams();
		}
		else
		{
			pError("SPDPListener: error reading Parameters from CDRMessage"<<endl);
			param.deleteParams();
			return false;
		}
	}
	return true;
}


bool SPDPListener::processParameterList(ParameterList_t& param,DiscoveredParticipantData* Pdata)
{
	for(std::vector<Parameter_t*>::iterator it = param.m_parameters.begin();
			it!=param.m_parameters.end();++it)
	{
	//	cout << "Parameter with Id: "<< (*it)->Pid << endl;
		switch((*it)->Pid)
		{
		case PID_KEY_HASH:
		{
			ParameterKey_t*p = (ParameterKey_t*)(*it);
			GUID_t guid;
			iHandle2GUID(guid,p->key);
			Pdata->m_guidPrefix = guid.guidPrefix;
			Pdata->m_key = p->key;
			break;
		}
		case PID_PROTOCOL_VERSION:
		{
			ProtocolVersion_t pv;
			PROTOCOLVERSION(pv);
			ParameterProtocolVersion_t * p = (ParameterProtocolVersion_t*)(*it);
			if(p->protocolVersion.m_major < pv.m_major)
			{
				return false;
			}
			Pdata->m_protocolVersion = p->protocolVersion;
			break;
		}
		case PID_VENDORID:
		{
			ParameterVendorId_t * p = (ParameterVendorId_t*)(*it);
			Pdata->m_VendorId[0] = p->vendorId[0];
			Pdata->m_VendorId[1] = p->vendorId[1];
			break;
		}
		case PID_EXPECTS_INLINE_QOS:
		{
			ParameterBool_t * p = (ParameterBool_t*)(*it);
			Pdata->m_expectsInlineQos = p->value;
			break;
		}
		case PID_PARTICIPANT_GUID:
		{
			ParameterGuid_t * p = (ParameterGuid_t*)(*it);
			Pdata->m_guidPrefix = p->guid.guidPrefix;
			Pdata->m_key = p->guid;
			break;
		}
		case PID_METATRAFFIC_MULTICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
			Pdata->m_metatrafficMulticastLocatorList.push_back(p->locator);
			break;
		}
		case PID_METATRAFFIC_UNICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
			Pdata->m_metatrafficUnicastLocatorList.push_back(p->locator);
			break;
		}
		case PID_DEFAULT_UNICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
			Pdata->m_defaultUnicastLocatorList.push_back(p->locator);
			break;
		}
		case PID_DEFAULT_MULTICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
			Pdata->m_defaultMulticastLocatorList.push_back(p->locator);
			break;
		}
		case PID_PARTICIPANT_LEASE_DURATION:
		{
			ParameterTime_t* p = (ParameterTime_t*)(*it);
			Pdata->leaseDuration = p->time;
			break;
		}
		case PID_BUILTIN_ENDPOINT_SET:
		{
			ParameterBuiltinEndpointSet_t* p = (ParameterBuiltinEndpointSet_t*)(*it);
			Pdata->m_availableBuiltinEndpoints = p->endpointSet;
			break;
		}
		case PID_ENTITY_NAME:
		{
			ParameterString_t* p = (ParameterString_t*)(*it);
			Pdata->m_participantName = p->m_string;
			break;
		}
		case PID_PROPERTY_LIST:
		{
			if(mp_SPDP->m_discovery.use_STATIC_EndpointDiscoveryProtocol)
			{
				ParameterPropertyList_t* p = (ParameterPropertyList_t*)(*it);
				uint16_t userId;
				EntityId_t entityId;
				for(std::vector<std::pair<std::string,std::string>>::iterator it = p->properties.begin();
						it != p->properties.end();++it)
				{
					std::stringstream ss;
					std::string first_str = it->first.substr(0, it->first.find("_"));
					if(first_str == "staticedp")
					{
						std::string type = it->first.substr(10, it->first.find("_"));
						first_str = it->first.substr(17, it->first.find("_"));
						ss.clear();	ss.str(std::string());
						ss << first_str;ss >> userId;
						ss.clear();ss.str(std::string());
						ss << it->second;
						int a,b,c,d;
						char ch;
						ss >> a >> ch >> b >> ch >> c >>ch >> d;
						entityId.value[0] = (octet)a;entityId.value[1] = (octet)b;
						entityId.value[2] = (octet)c;entityId.value[3] = (octet)d;
						assignUserId(type,userId,entityId,Pdata);
					}
				}
			}
			break;
		}
		default: break;
		}
	}

	return true;
}

void SPDPListener::assignUserId(std::string& type,uint16_t userId, EntityId_t& entityId,DiscoveredParticipantData* pdata)
{
	if(type == "reader")
	{
		for(std::vector<DiscoveredReaderData*>::iterator it = pdata->m_readers.begin();
				it!=pdata->m_readers.end();++it)
		{
			if((*it)->userDefinedId == userId)
			{
				(*it)->m_readerProxy.remoteReaderGuid.guidPrefix = pdata->m_guidPrefix;
				(*it)->m_readerProxy.remoteReaderGuid.entityId = entityId;
				(*it)->isAlive = true;
				return;
			}
		}
	}
	else if(type=="writer")
	{
		for(std::vector<DiscoveredWriterData*>::iterator it = pdata->m_writers.begin();
				it!=pdata->m_writers.end();++it)
		{
			if((*it)->userDefinedId == userId)
			{
				(*it)->m_writerProxy.remoteWriterGuid.guidPrefix = pdata->m_guidPrefix;
				(*it)->m_writerProxy.remoteWriterGuid.entityId = entityId;
				return;
			}
		}
	}
	pWarning("SPDPListener:StaticEDP userId not match any of the loaded ones"<<endl);

}

} /* namespace rtps */
} /* namespace eprosima */
