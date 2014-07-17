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


#include "eprosimartps/builtin/discovery/participant/PDPSimple.h"

#include "eprosimartps/Participant.h"
#include "eprosimartps/ParticipantProxyData.h"

#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"

#include "eprosimartps/reader/WriterProxyData.h"
#include "eprosimartps/writer/ReaderProxyData.h"

#include "eprosimartps/utils/RTPSLog.h"

namespace eprosima {
namespace rtps {

EDPSimple::EDPSimple(PDPSimple* p,ParticipantImpl* part):
		EDP(p,part),
		mp_PubWriter(NULL),mp_SubWriter(NULL),
		mp_PubReader(NULL),mp_SubReader(NULL),
		m_listeners(this)

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
		Wparam.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
		Wparam.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
		Wparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		created &=this->mp_participant->createWriter(&waux,Wparam,DISCOVERY_PUBLICATION_DATA_MAX_SIZE,true,STATEFUL,NULL,NULL,c_EntityId_SEDPPubWriter);
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
		Rparam.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
		Rparam.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
		Rparam.userDefinedId = -1;
		Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		created &=this->mp_participant->createReader(&raux,Rparam,DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE,
				true,STATEFUL,(DDSTopicDataType*)&m_subReaderTopicDataType,(SubscriberListener*)&m_listeners.m_subReaderListener,c_EntityId_SEDPSubReader);
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
		Rparam.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
		Rparam.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
		Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		created &=this->mp_participant->createReader(&raux,Rparam,DISCOVERY_PUBLICATION_DATA_MAX_SIZE,
				true,STATEFUL,(DDSTopicDataType*)&m_pubReaderTopicDataType,(SubscriberListener*)&m_listeners.m_pubReaderListener,c_EntityId_SEDPPubReader);
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
		Wparam.unicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficUnicastLocatorList;
		Wparam.multicastLocatorList = this->mp_PDP->getLocalParticipantProxyData()->m_metatrafficMulticastLocatorList;
		Wparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		created &=this->mp_participant->createWriter(&waux,Wparam,DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE,true,STATEFUL,NULL,NULL,c_EntityId_SEDPSubWriter);
		if(created)
		{
			mp_SubWriter = dynamic_cast<StatefulWriter*>(waux);
			pInfo(RTPS_CYAN<<"SEDP Subscription Writer created"<<RTPS_DEF<<endl);
		}
	}
	pInfo(RTPS_CYAN<<"SimpleEDP Endpoints creation finished"<<RTPS_DEF<<endl);
	return created;
}


bool EDPSimple::processLocalReaderProxyData(ReaderProxyData* rdata)
{
	if(mp_SubWriter !=NULL)
	{
		CacheChange_t* change = NULL;
		if(mp_SubWriter->new_change(ALIVE,NULL,&change))
		{
			change->instanceHandle = rdata->m_key;
			rdata->toParameterList();
			ParameterList::updateCDRMsg(&rdata->m_parameterList,EPROSIMA_ENDIAN);
			change->serializedPayload.encapsulation = EPROSIMA_ENDIAN == BIGEND ? PL_CDR_BE: PL_CDR_LE;
			change->serializedPayload.length = rdata->m_parameterList.m_cdrmsg.length;
			memcpy(change->serializedPayload.data,rdata->m_parameterList.m_cdrmsg.buffer,change->serializedPayload.length);
			mp_SubWriter->add_change(change);
			mp_SubWriter->unsent_change_add(change);
			return true;
		}
		return false;
	}
	return true;
}
bool EDPSimple::processLocalWriterProxyData(WriterProxyData* wdata)
{
	if(mp_PubWriter !=NULL)
	{
		CacheChange_t* change = NULL;
		if(mp_PubWriter->new_change(ALIVE,NULL,&change))
		{
			change->instanceHandle = wdata->m_key;
			wdata->toParameterList();
			ParameterList::updateCDRMsg(&wdata->m_parameterList,EPROSIMA_ENDIAN);
			change->serializedPayload.encapsulation = EPROSIMA_ENDIAN == BIGEND ? PL_CDR_BE: PL_CDR_LE;
			change->serializedPayload.length = wdata->m_parameterList.m_cdrmsg.length;
			memcpy(change->serializedPayload.data,wdata->m_parameterList.m_cdrmsg.buffer,change->serializedPayload.length);
			mp_SubWriter->add_change(change);
			mp_SubWriter->unsent_change_add(change);
			return true;
		}
		return false;
	}
	return true;
}

bool EDPSimple::removeLocalWriter(RTPSWriter* W)
{
	if(mp_PubWriter!=NULL)
	{
		CacheChange_t* change = NULL;
		if(mp_PubWriter->new_change(NOT_ALIVE_DISPOSED_UNREGISTERED,NULL,&change))
		{
			change->instanceHandle = W->getGuid();
			mp_PubWriter->add_change(change);
			mp_PubWriter->unsent_change_add(change);
		}
	}
	return unpairWriterProxy(W->getGuid());
}

bool EDPSimple::removeLocalReader(RTPSReader* R)
{
	if(mp_SubWriter!=NULL)
	{
		CacheChange_t* change = NULL;
		if(mp_SubWriter->new_change(NOT_ALIVE_DISPOSED_UNREGISTERED,NULL,&change))
		{
			change->instanceHandle = R->getGuid();
			mp_SubWriter->add_change(change);
			mp_SubWriter->unsent_change_add(change);
		}
	}
	return unpairReaderProxy(R->getGuid());
}



void EDPSimple::assignRemoteEndpoints(ParticipantProxyData* pdata)
{
	pInfo(RTPS_CYAN<<"SimpleEDP:assignRemoteEndpoints: new DPD received, adding remote endpoints to our SimpleEDP endpoints"<<RTPS_DEF<<endl);
	uint32_t endp = pdata->m_availableBuiltinEndpoints;
	uint32_t auxendp = endp;
	auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
	if(auxendp!=0 && mp_PubReader!=NULL) //Exist Pub Writer and i have pub reader
	{
		pDebugInfo(RTPS_CYAN<<"Adding SEDP Pub Writer to my Pub Reader"<<RTPS_DEF<<endl);
		WriterProxyData* wp = new WriterProxyData();
		wp->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		wp->m_guid.entityId = c_EntityId_SEDPPubWriter;
		wp->m_unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		wp->m_multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		wp->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		wp->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		pdata->m_builtinWriters.push_back(wp);
		mp_PubReader->matched_writer_add(wp);
	}
	auxendp = endp;
	auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
	if(auxendp!=0 && mp_PubWriter!=NULL) //Exist Pub Detector
	{
		pDebugInfo(RTPS_CYAN<<"Adding SEDP Pub Reader to my Pub Writer"<<RTPS_DEF<<endl);
		ReaderProxyData* rp = new ReaderProxyData();
		rp->m_expectsInlineQos = false;
		rp->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		rp->m_guid.entityId = c_EntityId_SEDPPubReader;
		rp->m_unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		rp->m_multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		rp->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		rp->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		pdata->m_builtinReaders.push_back(rp);
		mp_PubWriter->matched_reader_add(rp);
	}
	auxendp = endp;
	auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
	if(auxendp!=0 && mp_SubReader!=NULL) //Exist Pub Announcer
	{
		pDebugInfo(RTPS_CYAN<<"Adding SEDP Sub Writer to my Sub Reader"<<RTPS_DEF<<endl);
		WriterProxyData* wp = new WriterProxyData();
		wp->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		wp->m_guid.entityId = c_EntityId_SEDPSubWriter;
		wp->m_unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		wp->m_multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		wp->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		wp->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		pdata->m_builtinWriters.push_back(wp);
		mp_SubReader->matched_writer_add(wp);
	}
	auxendp = endp;
	auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
	if(auxendp!=0 && mp_SubWriter!=NULL) //Exist Pub Announcer
	{
		pDebugInfo(RTPS_CYAN<<"Adding SEDP Sub Reader to my Sub Writer"<<RTPS_DEF<<endl);
		ReaderProxyData* rp = new ReaderProxyData();
		rp->m_expectsInlineQos = false;
		rp->m_guid.guidPrefix = pdata->m_guid.guidPrefix;
		rp->m_guid.entityId = c_EntityId_SEDPSubReader;
		rp->m_unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		rp->m_multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		rp->m_qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
		rp->m_qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		pdata->m_builtinReaders.push_back(rp);
		mp_SubWriter->matched_reader_add(rp);
	}
}






} /* namespace rtps */
} /* namespace eprosima */
