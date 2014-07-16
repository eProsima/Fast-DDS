/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PDPSimple.cpp
 *
 */

#include "eprosimartps/builtin/discovery/participant/PDPSimple.h"
#include "eprosimartps/dds/DomainParticipant.h"
#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/Participant.h"
#include "eprosimartps/ParticipantProxyData.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"



using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

PDPSimple::PDPSimple(BuiltinProtocols* built):
						mp_builtin(built),
						mp_participant(NULL),
						mp_SPDPWriter(NULL),
						mp_SPDPReader(NULL),
						mp_EDP(NULL),
						m_hasChangedLocalPDP(true),
						mp_resendParticipantTimer(NULL),
						m_listener(this)
{

}

PDPSimple::~PDPSimple() {
	// TODO Auto-generated destructor stub
}

bool PDPSimple::initPDP(const DiscoveryAttributes& attributes,uint32_t participantID)
{
	pInfo(RTPS_B_CYAN<<"Beginning ParticipantDiscoveryProtocol Initialization"<<RTPS_DEF<<endl);
	m_discovery = attributes;

	if(!createSPDPEndpoints())
		return false;
	mp_builtin->updateMetatrafficLocators();
	this->mp_SPDPReader->lock();
	this->mp_SPDPWriter->lock();

	m_participantProxies.push_back(new ParticipantProxyData());
	m_participantProxies.front()->initializeData(mp_participant,this);

	//INIT EDP
	if(m_discovery.use_STATIC_EndpointDiscoveryProtocol)
	{
		mp_EDP = (EndpointDiscoveryProtocol*)new EDPStatic(this);
	}
	else if(m_discovery.use_SIMPLE_EndpointDiscoveryProtocol)
	{
		mp_EDP = (EndpointDiscoveryProtocol*)new EDPSimple(this);
	}
	else
	{
		pWarning("No EndpointDiscoveryProtocol defined"<<endl);
		return false;
	}

	this->announceParticipantState(true);
	mp_resendParticipantTimer = new ResendParticipantProxyDataPeriod(this,mp_participant->getEventResource(),
								boost::posix_time::milliseconds(Time_t2MilliSec(m_discovery.resendDiscoveryParticipantDataPeriod)));
	mp_resendParticipantTimer->restart_timer();

	eClock::my_sleep(100);


	this->mp_SPDPReader->unlock();
	this->mp_SPDPWriter->unlock();
	return true;
}

void PDPSimple::stopParticipantAnnouncement()
{
	mp_resendParticipantTimer->stop_timer();
}

void PDPSimple::resetParticipantAnnouncement()
{
	mp_resendParticipantTimer->restart_timer();
}

void PDPSimple::announceParticipantState(bool new_change)
{
	pInfo("Announcing Participant State"<<endl);
	CacheChange_t* change = NULL;
	if(new_change || m_hasChangedLocalPDP)
	{
		m_participantProxies.front()->updateData(mp_participant,this);
		if(mp_SPDPWriter->getHistoryCacheSize() > 0)
			mp_SPDPWriter->removeMinSeqCacheChange();
		mp_SPDPWriter->new_change(ALIVE,NULL,&change);
		change->instanceHandle = m_participantProxies.front()->m_key;
		if(m_participantProxies.front()->toParameterList())
		{
			change->serializedPayload.encapsulation = EPROSIMA_ENDIAN == BIGEND ? PL_CDR_BE: PL_CDR_LE;
			change->serializedPayload.length = m_participantProxies.front()->m_QosList.allQos.m_cdrmsg.length;
			memcpy(change->serializedPayload.data,m_participantProxies.front()->m_QosList.allQos.m_cdrmsg.buffer,change->serializedPayload.length);
			mp_SPDPWriter->add_change(change);
		}
	}
	else
	{
		if(!mp_SPDPWriter->get_last_added_cache(&change))
		{
			pWarning("Error getting last added change"<<endl);
			return;
		}
	}
	mp_SPDPWriter->unsent_change_add(change);
}



bool PDPSimple::createSPDPEndpoints()
{
	pInfo(RTPS_CYAN<<"Creating SPDP Endpoints"<<endl);
	//SPDP BUILTIN PARTICIPANT WRITER
	PublisherAttributes Wparam;
	Wparam.pushMode = true;
	//Wparam.historyMaxSize = 1;
	//Locators where it is going to listen
	Wparam.topic.topicName = "DCPSParticipant";
	Wparam.topic.topicDataType = "DiscoveredParticipantData";
	Wparam.topic.topicKind = WITH_KEY;
	Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Wparam.topic.historyQos.depth = 1;
	Wparam.topic.resourceLimitsQos.max_instances = 1;
	Wparam.topic.resourceLimitsQos.max_samples_per_instance = 1;
	Wparam.topic.resourceLimitsQos.max_samples = 2;
	Wparam.topic.resourceLimitsQos.allocated_samples = 2;
	Wparam.payloadMaxSize = 1000;
	Wparam.userDefinedId = -99;
	Wparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
	RTPSWriter* wout;
	if(mp_participant->createWriter(&wout,Wparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE,true,STATELESS,NULL,NULL,c_EntityId_SPDPWriter))
	{
		mp_SPDPWriter = dynamic_cast<StatelessWriter*>(wout);
		for(LocatorListIterator lit = mp_builtin->m_metatrafficMulticastLocatorList.begin();
				lit!=mp_builtin->m_metatrafficMulticastLocatorList.end();++lit)
			mp_SPDPWriter->reader_locator_add(*lit,false);
	}
	else
	{
		pError("SimplePDP Writer creation failed"<<endl);
		return false;
	}
	//SPDP BUILTIN PARTICIPANT READER
	SubscriberAttributes Rparam;
	//	Rparam.historyMaxSize = 100;
	//  Locators where it is going to listen
	Rparam.multicastLocatorList = mp_builtin->m_metatrafficMulticastLocatorList;
	Rparam.unicastLocatorList = mp_builtin->m_metatrafficUnicastLocatorList;
	Rparam.topic.topicKind = WITH_KEY;
	Rparam.topic.topicName = "DCPSParticipant";
	Rparam.topic.topicDataType = "DiscoveredParticipantData";
	Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	Rparam.topic.historyQos.depth = 1;
	Rparam.topic.resourceLimitsQos.max_instances = 1000;
	Rparam.topic.resourceLimitsQos.max_samples_per_instance = 1;
	Rparam.topic.resourceLimitsQos.max_samples = 1000;
	Rparam.topic.resourceLimitsQos.allocated_samples = 500;
	Rparam.payloadMaxSize = 1000;
	Rparam.userDefinedId = -99;
	Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
	RTPSReader* rout;
	if(mp_participant->createReader(&rout,Rparam,DISCOVERY_PARTICIPANT_DATA_MAX_SIZE,
			true,STATELESS,(DDSTopicDataType*)&m_topicDataType,(SubscriberListener*)&this->m_listener,c_EntityId_SPDPReader))
	{
		mp_SPDPReader = dynamic_cast<StatelessReader*>(rout);
		//mp_SPDPReader->setListener();
	}
	else
	{
		pError("SimplePDP Reader creation failed"<<endl);
		return false;
	}

	pInfo(RTPS_CYAN<< "SPDP Endpoints creation finished"<<RTPS_DEF<<endl)
	return true;
}



} /* namespace rtps */
} /* namespace eprosima */
