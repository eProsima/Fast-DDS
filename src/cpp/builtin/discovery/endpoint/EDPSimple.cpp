/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPSimple.cpp
 *
 */

#include "eprosimartps/builtin/discovery/endpoint/EDPSimple.h"
#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/Participant.h"
#include "eprosimartps/ParticipantProxyData.h"


namespace eprosima {
namespace rtps {

EDPSimple::EDPSimple(PDPSimple* p):
								EDP(p),
								mp_PubWriter(NULL),mp_SubWriter(NULL),// mp_TopWriter(NULL),
								mp_PubReader(NULL),mp_SubReader(NULL)// mp_TopReader(NULL),

{
	// TODO Auto-generated constructor stub

}

EDPSimple::~EDPSimple() {
	// TODO Auto-generated destructor stub
}


bool EDPSimple::initEDP(DiscoveryAttributes& attributes)
{
	pInfo(RTPS_B_CYAN<<"Initializing EndpointDiscoveryProtocol"<<endl);
	m_discovery = attributes;

	if(!createSEDPEndpoints())
	{
		pError("Problem creation SimpleEDP endpoints"<<endl);
		return false;
	}
	return true;
}


bool EDPSimple::createSEDPEndpoints()
{
	pInfo(RTPS_CYAN<<"Creating SEDP Endpoints"<<RTPS_DEF<<endl);
	PublisherAttributes Wparam;
	SubscriberAttributes Rparam;
	bool created = true;
	RTPSReader* raux;
	RTPSWriter* waux;
	if(m_discovery.m_simpleEDP.use_PublicationWriterANDSubscriptionReader)
	{
		//	Wparam.historyMaxSize = 100;
		Wparam.pushMode = true;
		Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		Wparam.topic.topicName = "DCPSPublication";
		Wparam.topic.topicKind = WITH_KEY;
		Wparam.topic.topicDataType = "DiscoveredWriterData";
		Wparam.userDefinedId = -99;
		Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
		Wparam.topic.historyQos.depth = 1;
		Wparam.topic.resourceLimitsQos.max_instances = 1000;
		Wparam.topic.resourceLimitsQos.max_samples_per_instance = 1;
		Wparam.topic.resourceLimitsQos.max_samples = 1000;
		Wparam.topic.resourceLimitsQos.allocated_samples = 500;
		Wparam.payloadMaxSize = 2000;
		Wparam.unicastLocatorList = this->mp_PDP->m_participantProxies.front()->m_metatrafficUnicastLocatorList;
		Wparam.multicastLocatorList = this->mp_PDP->m_participantProxies.front()->m_metatrafficMulticastLocatorList;
		Wparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		created &=this->mp_PDP->mp_participant->createWriter(&waux,Wparam,DISCOVERY_PUBLICATION_DATA_MAX_SIZE,true,STATEFUL,NULL,NULL,c_EntityId_SEDPPubWriter);
		if(created)
		{
			mp_PubWriter = dynamic_cast<StatefulWriter*>(waux);
			pInfo(RTPS_CYAN<<"SEDP Publication Writer created"<<RTPS_DEF<<endl);
		}
		//Rparam.historyMaxSize = 100;
		Rparam.expectsInlineQos = false;
		Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		Rparam.topic.topicName = "DCPSSubscription";
		Rparam.topic.topicKind = WITH_KEY;
		Rparam.topic.topicDataType = "DiscoveredReaderData";
		Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
		Rparam.topic.historyQos.depth = 1;
		Rparam.topic.resourceLimitsQos.max_instances = 1000000;
		Rparam.topic.resourceLimitsQos.max_samples_per_instance = 1;
		Rparam.topic.resourceLimitsQos.max_samples = 1000000;
		Rparam.topic.resourceLimitsQos.allocated_samples = 1000;
		Rparam.payloadMaxSize = 2000;
		Rparam.unicastLocatorList = this->mp_PDP->m_participantProxies.front()->m_metatrafficUnicastLocatorList;
		Rparam.multicastLocatorList = this->mp_PDP->m_participantProxies.front()->m_metatrafficMulticastLocatorList;
		Rparam.userDefinedId = -1;
		Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		created &=this->mp_PDP->mp_participant->createReader(&raux,Rparam,DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE,
				true,STATEFUL,(DDSTopicDataType*)&m_SubReaderDataType,(SubscriberListener*)&m_listeners.m_SubListener,c_EntityId_SEDPSubReader);
		if(created)
		{
			mp_SubReader = dynamic_cast<StatefulReader*>(raux);
			pInfo(RTPS_CYAN<<"SEDP Subscription Reader created"<<RTPS_DEF<<endl);
		}
	}
	if(m_discovery.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter)
	{
		//Rparam.historyMaxSize = 100;
		Rparam.expectsInlineQos = false;
		Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		Rparam.topic.topicName = "DCPSPublication";
		Rparam.topic.topicKind = WITH_KEY;
		Rparam.topic.topicDataType = "DiscoveredWriterData";
		Rparam.userDefinedId = -99;
		Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
		Rparam.topic.historyQos.depth = 1;
		Rparam.topic.resourceLimitsQos.max_instances = 1000000;
		Rparam.topic.resourceLimitsQos.max_samples_per_instance = 1;
		Rparam.topic.resourceLimitsQos.max_samples = 1000000;
		Rparam.topic.resourceLimitsQos.allocated_samples = 1000;
		Rparam.payloadMaxSize = 2000;
		Rparam.unicastLocatorList = this->mp_PDP->m_participantProxies.front()->m_metatrafficUnicastLocatorList;
		Rparam.multicastLocatorList = this->mp_PDP->m_participantProxies.front()->m_metatrafficMulticastLocatorList;
		Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		created &=this->mp_PDP->mp_participant->createReader(&raux,Rparam,DISCOVERY_PUBLICATION_DATA_MAX_SIZE,
				true,STATEFUL,(DDSTopicDataType*)&m_PubReaderDataType,(SubscriberListener*)&m_listeners.m_PubListener,c_EntityId_SEDPPubReader);
		if(created)
		{
			mp_PubReader = dynamic_cast<StatefulReader*>(raux);
			pInfo(RTPS_CYAN<<"SEDP Publication Reader created"<<RTPS_DEF<<endl);
		}
		//Wparam.historyMaxSize = 100;
		Wparam.pushMode = true;
		Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		Wparam.topic.topicName = "DCPSSubscription";
		Wparam.topic.topicKind = WITH_KEY;
		Wparam.topic.topicDataType = "DiscoveredReaderData";
		Wparam.userDefinedId = -1;
		Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
		Wparam.topic.historyQos.depth = 1;
		Wparam.topic.resourceLimitsQos.max_instances = 1000;
		Wparam.topic.resourceLimitsQos.max_samples_per_instance = 1;
		Wparam.topic.resourceLimitsQos.max_samples = 1000;
		Wparam.topic.resourceLimitsQos.allocated_samples = 500;
		Wparam.payloadMaxSize = 2000;
		Wparam.unicastLocatorList = this->mp_PDP->m_participantProxies.front()->m_metatrafficUnicastLocatorList;
		Wparam.multicastLocatorList = this->mp_PDP->m_participantProxies.front()->m_metatrafficMulticastLocatorList;
		Wparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		created &=this->mp_PDP->mp_participant->createWriter(&waux,Wparam,DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE,true,STATEFUL,NULL,NULL,c_EntityId_SEDPSubWriter);
		if(created)
		{
			mp_SubWriter = dynamic_cast<StatefulWriter*>(waux);
			pInfo(RTPS_CYAN<<"SEDP Subscription Writer created"<<RTPS_DEF<<endl);
		}
	}
	pInfo(RTPS_CYAN<<"SimpleEDP Endpoints creation finished"<<RTPS_DEF<<endl);
	return created;
}

} /* namespace rtps */
} /* namespace eprosima */
