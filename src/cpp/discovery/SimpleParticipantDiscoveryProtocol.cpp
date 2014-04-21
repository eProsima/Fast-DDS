/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimpleDiscoveryParticipantProtocol.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/discovery/SimpleParticipantDiscoveryProtocol.h"
#include "eprosimartps/Participant.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"
namespace eprosima {
namespace rtps {

SimpleParticipantDiscoveryProtocol::SimpleParticipantDiscoveryProtocol(Participant* p):
		mp_Participant(p),
		m_DPDMsgHeader(RTPSMESSAGE_HEADER_SIZE),
		m_listener(this)
{
	// TODO Auto-generated constructor stub
	m_resendData = NULL;
	m_SPDPbPWriter = NULL;
	m_SPDPbPReader = NULL;
	m_SPDP_WELL_KNOWN_MULTICAST_PORT = 0;
	m_SPDP_WELL_KNOWN_UNICAST_PORT = 0;
	m_domainId = 0;
	m_hasChanged_DPD = true;
	//new_change_added_ptr = (void *())boost::bind(SimpleParticipantDiscoveryProtocol::new_change_added,this);
}

SimpleParticipantDiscoveryProtocol::~SimpleParticipantDiscoveryProtocol()
{
	delete(m_resendData);
	delete(m_SPDPbPWriter);
	delete(m_SPDPbPReader);
}

bool SimpleParticipantDiscoveryProtocol::initSPDP(uint16_t domainId,
		uint16_t participantId, uint16_t resendDataPeriod_sec)
{
	m_domainId = domainId;
	DomainParticipant* dp = DomainParticipant::getInstance();
	m_SPDP_WELL_KNOWN_MULTICAST_PORT = dp->getPortBase()
									+ dp->getDomainIdGain() * domainId
									+ dp->getOffsetd0();
	m_SPDP_WELL_KNOWN_UNICAST_PORT =  dp->getPortBase()
										+ dp->getDomainIdGain() * m_domainId
										+ dp->getOffsetd1()
										+ dp->getParticipantIdGain() * participantId;


	m_DPD.m_proxy.m_guidPrefix = mp_Participant->m_guid.guidPrefix;

	//FIXME: register type correctly
	//dp->registerType("DiscoveredParticipantData",&this->ser,&this->deser,&this->getKey);

	CDRMessage::initCDRMsg(&m_DPDMsgHeader);
	RTPSMessageCreator::addHeader(&m_DPDMsg,m_DPD.m_proxy.m_guidPrefix);


	Locator_t multiLocator;
	multiLocator.kind = LOCATOR_KIND_UDPv4;
	multiLocator.port = m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	multiLocator.set_IP4_address(239,255,0,1);
	m_DPD.m_proxy.m_metatrafficMulticastLocatorList.push_back(multiLocator);

	std::vector<Locator_t> locators;
	DomainParticipant::getIPAddress(&locators);
	for(std::vector<Locator_t>::iterator it=locators.begin();it!=locators.end();++it)
	{
		it->port = m_SPDP_WELL_KNOWN_UNICAST_PORT;
		m_DPD.m_proxy.m_metatrafficUnicastLocatorList.push_back(*it);
		m_DPD.m_proxy.m_defaultUnicastLocatorList.push_back(*it);
	}

	m_DPD.leaseDuration.seconds = 100;



	//SPDP BUILTIN PARTICIPANT WRITER
	WriterParams_t Wparam;
	Wparam.pushMode = true;
	Wparam.historySize = 1;
	//Locators where it is going to listen
	Wparam.multicastLocatorList = m_DPD.m_proxy.m_metatrafficMulticastLocatorList;
	Wparam.unicastLocatorList = m_DPD.m_proxy.m_metatrafficUnicastLocatorList;
	Wparam.topicName = "DCPSParticipant";
	Wparam.topicDataType = "DiscoveredParticipantData";
	Wparam.topicKind = WITH_KEY;
	mp_Participant->createStatelessWriter(&m_SPDPbPWriter,Wparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE);
	//m_SPDPbPWriter = new StatelessWriter(Wparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE);
	m_SPDPbPWriter->m_guid.entityId = ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER;
	ReaderLocator multiReaderLoc;
	multiReaderLoc.expectsInlineQos = false;
	multiReaderLoc.locator = multiLocator;
	m_SPDPbPWriter->reader_locator_add(multiReaderLoc);

	m_SPDPbPWriter->mp_send_thr->m_send_socket.set_option( boost::asio::ip::udp::socket::reuse_address( true ) );
	 boost::asio::ip::address address = boost::asio::ip::address::from_string("239.255.0.1");
	m_SPDPbPWriter->mp_send_thr->m_send_socket.set_option( boost::asio::ip::multicast::join_group( address ) );


	//SPDP BUILTIN PARTICIPANT READER
	ReaderParams_t Rparam;
	Rparam.historySize = 100;
	//Locators where it is going to listen
	Rparam.multicastLocatorList = m_DPD.m_proxy.m_metatrafficMulticastLocatorList;
	Rparam.unicastLocatorList = m_DPD.m_proxy.m_metatrafficUnicastLocatorList;
	Rparam.topicKind = WITH_KEY;
	Rparam.topicName = "DCPSParticipant";
	Rparam.topicDataType = "DiscoveredParticipantData";
	mp_Participant->createStatelessReader(&m_SPDPbPReader,Rparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE);
	m_SPDPbPReader->m_guid.entityId = ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER;
	m_SPDPbPReader->m_listener = &this->m_listener;


	this->sendDPDMsg();


	m_resendData = new ResendDiscoveryDataPeriod(this,boost::posix_time::milliseconds(resendDataPeriod_sec*1000));
	m_resendData->restart_timer();



	return true;
}

bool SimpleParticipantDiscoveryProtocol::updateParamList()
{
	m_DPDAsParamList.allQos.deleteParams();
	m_DPDAsParamList.allQos.resetList();
	m_DPDAsParamList.inlineQos.resetList();
	bool valid = QosList::addQos(&m_DPDAsParamList,PID_PROTOCOL_VERSION,m_DPD.m_proxy.m_protocolVersion);
	valid &=QosList::addQos(&m_DPDAsParamList,PID_VENDORID,m_DPD.m_proxy.m_VendorId);
	valid &=QosList::addQos(&m_DPDAsParamList,PID_EXPECTS_INLINE_QOS,m_DPD.m_proxy.m_expectsInlineQos);
	valid &=QosList::addQos(&m_DPDAsParamList,PID_PARTICIPANT_GUID,mp_Participant->m_guid);
	for(std::vector<Locator_t>::iterator it=m_DPD.m_proxy.m_metatrafficMulticastLocatorList.begin();
			it!=m_DPD.m_proxy.m_metatrafficMulticastLocatorList.end();++it)
	{
		valid &=QosList::addQos(&m_DPDAsParamList,PID_METATRAFFIC_MULTICAST_LOCATOR,*it);
	}
	for(std::vector<Locator_t>::iterator it=m_DPD.m_proxy.m_metatrafficUnicastLocatorList.begin();
			it!=m_DPD.m_proxy.m_metatrafficUnicastLocatorList.end();++it)
	{
		valid &=QosList::addQos(&m_DPDAsParamList,PID_METATRAFFIC_UNICAST_LOCATOR,*it);
	}
	for(std::vector<Locator_t>::iterator it=m_DPD.m_proxy.m_defaultUnicastLocatorList.begin();
			it!=m_DPD.m_proxy.m_defaultUnicastLocatorList.end();++it)
	{
		valid &=QosList::addQos(&m_DPDAsParamList,PID_DEFAULT_UNICAST_LOCATOR,*it);
	}
	valid &=QosList::addQos(&m_DPDAsParamList,PID_PARTICIPANT_LEASE_DURATION,m_DPD.leaseDuration);
	valid &=QosList::addQos(&m_DPDAsParamList,PID_BUILTIN_ENDPOINT_SET,m_DPD.m_proxy.m_availableBuiltinEndpoints);
	valid &=QosList::addQos(&m_DPDAsParamList,PID_ENTITY_NAME,this->mp_Participant->m_participantName);


	valid &=ParameterList::updateCDRMsg(&m_DPDAsParamList.allQos,EPROSIMA_ENDIAN);

	return valid;

}

bool SimpleParticipantDiscoveryProtocol::sendDPDMsg()
{
	boost::lock_guard<SimpleParticipantDiscoveryProtocol> guard(*this);
	CacheChange_t* change=NULL;
	if(m_DPDAsParamList.allQos.m_hasChanged || m_hasChanged_DPD)
	{
		m_SPDPbPWriter->m_writer_cache.remove_all_changes();
		m_SPDPbPWriter->new_change(ALIVE,NULL,&change);
		for(uint8_t i = 0;i<12;++i)
			change->instanceHandle.value[i] = this->mp_Participant->m_guid.guidPrefix.value[i];
		for(uint8_t i = 12;i<16;++i)
			change->instanceHandle.value[i] = this->mp_Participant->m_guid.entityId.value[i];
		if(updateParamList())
		{
		change->serializedPayload.encapsulation = EPROSIMA_ENDIAN == BIGEND ? PL_CDR_BE: PL_CDR_LE;
		change->serializedPayload.length = m_DPDAsParamList.allQos.m_cdrmsg.length;
		memcpy(change->serializedPayload.data,m_DPDAsParamList.allQos.m_cdrmsg.buffer,change->serializedPayload.length);
		m_SPDPbPWriter->m_writer_cache.add_change(change);
		}
		else
		{
			pWarning("Parameter List update failed"<<endl);
			return false;
		}
	}
	else
	{
		if(!m_SPDPbPWriter->m_writer_cache.get_last_added_cache(&change))
		{
			pWarning("Error getting last added change"<<endl);
			return false;
		}
	}
	m_SPDPbPWriter->unsent_change_add(change);
	return true;
}

bool SimpleParticipantDiscoveryProtocol::updateDPDMsg()
{
	m_SPDPbPWriter->m_writer_cache.remove_all_changes();
	CacheChange_t* change=NULL;
	m_SPDPbPWriter->new_change(ALIVE,NULL,&change);
	//FIXME: update change serialized payload
	if(EPROSIMA_ENDIAN == BIGEND)
		change->serializedPayload.encapsulation = PL_CDR_BE;
	else
		change->serializedPayload.encapsulation = PL_CDR_LE;

	updateParamList();

	change->serializedPayload.length = m_DPDAsParamList.allQos.m_cdrmsg.length;
	memcpy(change->serializedPayload.data,m_DPDAsParamList.allQos.m_cdrmsg.buffer,change->serializedPayload.length);

	m_SPDPbPWriter->m_writer_cache.add_change(change);

	CDRMessage::initCDRMsg(&m_DPDMsg);
	RTPSMessageCreator::addHeader(&m_DPDMsg,m_DPD.m_proxy.m_guidPrefix);
	RTPSMessageCreator::addSubmessageInfoTS_Now(&m_DPDMsg,false);

	m_SPDPbPWriter->m_writer_cache.get_last_added_cache(&change);
	RTPSMessageCreator::addSubmessageData(&m_DPDMsg,change,WITH_KEY,c_EntityId_Unknown,NULL);
	m_hasChanged_DPD = false;
	return true;
}

void SimpleParticipantDiscoveryProtocol::new_change_added()
{
	pInfo("New SPDP Message received"<<endl);
	CacheChange_t* change = NULL;
	if(m_SPDPbPReader->m_reader_cache.get_last_added_cache(&change))
	{
		ParameterList_t param;
		CDRMessage_t msg;
		msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND:LITTLEEND;

		msg.length = change->serializedPayload.length;
		//cout << "msg length: " << msg.length << endl;
		memcpy(msg.buffer,change->serializedPayload.data,msg.length);
		ParameterList::readParameterListfromCDRMsg(&msg,&param,NULL,NULL);
		DiscoveredParticipantData pdata;
		if(processParameterList(param,&pdata))
		{
			pDebugInfo("ParameterList correctly processed"<<endl);
			for(uint8_t i = 0;i<12;++i)
				change->instanceHandle.value[i] = pdata.m_proxy.m_guidPrefix.value[i];
			for(uint8_t i = 12;i<16;++i)
				change->instanceHandle.value[i] = this->mp_Participant->m_guid.entityId.value[i];
			bool from_me = true;
			for(uint8_t i =0;i<12;i++)
			{
				//cout << (int)change->instanceHandle.value[i] << "//"<< (int)this->mp_Participant->m_guid.guidPrefix.value[i] << endl;
				if(change->instanceHandle.value[i] != this->mp_Participant->m_guid.guidPrefix.value[i])
				{
					from_me = false;
					break;
				}
			}

			if(from_me)
			{
				pInfo("SPDP Message from own participant"<<endl);
				m_SPDPbPReader->m_reader_cache.remove_change(change->sequenceNumber,change->writerGUID);
				return;
			}
			bool found = false;
			for(std::vector<DiscoveredParticipantData>::iterator it = m_matched_participants.begin();
					it!=m_matched_participants.end();++it)
			{
				if(it->m_proxy.m_guidPrefix == pdata.m_proxy.m_guidPrefix)
				{
					*it = pdata;
					found = true;
					break;
				}
			}
			if(!found)
			{
				m_matched_participants.push_back(pdata);
				//Create Reader Locators
				ReaderLocator rl;
				rl.expectsInlineQos = pdata.m_proxy.m_expectsInlineQos;
				for(std::vector<Locator_t>::iterator it = pdata.m_proxy.m_metatrafficUnicastLocatorList.begin();
						it!=pdata.m_proxy.m_metatrafficUnicastLocatorList.end();++it)
				{
					rl.locator = *it;
					m_SPDPbPWriter->reader_locator_add(rl);
				}
				for(std::vector<Locator_t>::iterator it = pdata.m_proxy.m_metatrafficMulticastLocatorList.begin();
						it!=pdata.m_proxy.m_metatrafficMulticastLocatorList.end();++it)
				{
					rl.locator = *it;
					m_SPDPbPWriter->reader_locator_add(rl);
				}
			}
			else
			{
				for(std::vector<CacheChange_t*>::iterator it = m_SPDPbPReader->m_reader_cache.m_changes.begin();
						it!=m_SPDPbPReader->m_reader_cache.m_changes.end();++it)
				{
					if((*it)->instanceHandle == change->instanceHandle)
					{
						m_SPDPbPReader->m_reader_cache.remove_change(it);
						break;
					}
				}
			}
		}
		else
		{
			pDebugInfo("Error Processing parameter List"<<endl);
			m_SPDPbPReader->m_reader_cache.remove_change(change->sequenceNumber,change->writerGUID);
		}

	}

}


bool SimpleParticipantDiscoveryProtocol::processParameterList(ParameterList_t& param,
		DiscoveredParticipantData* Pdata)
{
	//cout << "Size of param list:  "<< param.m_parameters.size() << endl;
	for(std::vector<Parameter_t*>::iterator it = param.m_parameters.begin();
			it!=param.m_parameters.end();++it)
	{
		//cout << "Parameter with PID: "<< (int)(*it)->Pid << endl;
		switch((*it)->Pid)
		{
		case PID_PROTOCOL_VERSION:
		{
			ProtocolVersion_t pv;
			PROTOCOLVERSION(pv);
			ParameterProtocolVersion_t * p = (ParameterProtocolVersion_t*)(*it);
			if(p->protocolVersion.m_major < pv.m_major)
			{
				return false;
			}
			Pdata->m_proxy.m_protocolVersion = p->protocolVersion;
			break;
		}
		case PID_VENDORID:
		{
			ParameterVendorId_t * p = (ParameterVendorId_t*)(*it);
			Pdata->m_proxy.m_VendorId[0] = p->vendorId[0];
			Pdata->m_proxy.m_VendorId[1] = p->vendorId[1];
			break;
		}
		case PID_EXPECTS_INLINE_QOS:
		{
			ParameterBool_t * p = (ParameterBool_t*)(*it);
			Pdata->m_proxy.m_expectsInlineQos = p->value;
			break;
		}
		case PID_PARTICIPANT_GUID:
		{
			ParameterGuid_t * p = (ParameterGuid_t*)(*it);
			Pdata->m_proxy.m_guidPrefix = p->guid.guidPrefix;
			break;
		}
		case PID_METATRAFFIC_MULTICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
			Pdata->m_proxy.m_metatrafficMulticastLocatorList.push_back(p->locator);
			break;
		}
		case PID_METATRAFFIC_UNICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
						Pdata->m_proxy.m_metatrafficUnicastLocatorList.push_back(p->locator);
			break;
		}
		case PID_DEFAULT_UNICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
						Pdata->m_proxy.m_defaultUnicastLocatorList.push_back(p->locator);
			break;
		}
		case PID_DEFAULT_MULTICAST_LOCATOR:
		{
			ParameterLocator_t* p = (ParameterLocator_t*)(*it);
			Pdata->m_proxy.m_defaultMulticastLocatorList.push_back(p->locator);
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
			Pdata->m_proxy.m_availableBuiltinEndpoints = p->endpointSet;
			break;
		}
		case PID_ENTITY_NAME:
		{
			ParameterString_t* p = (ParameterString_t*)(*it);
			Pdata->m_proxy.m_participantName = p->m_string;
			break;
		}
		default: break;
		}
	}
	return true;
}


} /* namespace rtps */
} /* namespace eprosima */
