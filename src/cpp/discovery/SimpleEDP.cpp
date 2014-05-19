/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimpleEDP.cpp
 *
 *  Created on: May 16, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/discovery/SimpleEDP.h"
#include "eprosimartps/discovery/ParticipantDiscoveryProtocol.h"
#include "eprosimartps/Participant.h"

#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/reader/StatefulReader.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

SimpleEDP::SimpleEDP():
				 mp_PubWriter(NULL),mp_SubWriter(NULL), mp_TopWriter(NULL),
				 mp_PubReader(NULL),mp_SubReader(NULL), mp_TopReader(NULL)
{


}

SimpleEDP::~SimpleEDP()
{

}

bool SimpleEDP::initEDP(DiscoveryAttributes& attributes)
{
	m_discovery = attributes;

	if(!createSEDPEndpoints())
	{
		pError("Problem creation SimpleEDP endpoints"<<endl);
		return false;
	}
	return true;
}

bool SimpleEDP::createSEDPEndpoints()
{
	PublisherAttributes Wparam;
	SubscriberAttributes Rparam;
	bool created = true;
	if(m_discovery.m_simpleEDP.use_Publication_Writer)
	{
		Wparam.historyMaxSize = 100;
		Wparam.pushMode = true;
		Wparam.reliability.reliabilityKind = RELIABLE;
		Wparam.topic.topicName = "DCPSPublication";
		Wparam.topic.topicKind = WITH_KEY;
		Wparam.topic.topicDataType = "DiscoveredWriterData";
		Wparam.userDefinedId = -1;
		Wparam.unicastLocatorList = this->mp_DPDP->mp_localPDP->m_metatrafficUnicastLocatorList;
		Wparam.multicastLocatorList = this->mp_DPDP->mp_localPDP->m_metatrafficMulticastLocatorList;
		created &=mp_participant->createStatefulWriter(&mp_PubWriter,Wparam,DISCOVERY_PUBLICATION_DATA_MAX_SIZE);
		mp_PubWriter->m_guid.entityId = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER;
	}
	if(m_discovery.m_simpleEDP.use_Publication_Reader)
	{
		Rparam.historyMaxSize = 100;
		Rparam.expectsInlineQos = false;
		Rparam.reliability.reliabilityKind = RELIABLE;
		Rparam.topic.topicName = "DCPSPublication";
		Rparam.topic.topicKind = WITH_KEY;
		Rparam.topic.topicDataType = "DiscoveredWriterData";
		Rparam.userDefinedId = -1;
		Rparam.unicastLocatorList = this->mp_DPDP->mp_localPDP->m_metatrafficUnicastLocatorList;
		Rparam.multicastLocatorList = this->mp_DPDP->mp_localPDP->m_metatrafficMulticastLocatorList;
		created &=mp_participant->createStatefulReader(&mp_PubReader,Rparam,DISCOVERY_PUBLICATION_DATA_MAX_SIZE);
		mp_PubWriter->m_guid.entityId = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER;
	}
	if(m_discovery.m_simpleEDP.use_Subscription_Writer)
	{
		Wparam.historyMaxSize = 100;
		Wparam.pushMode = true;
		Wparam.reliability.reliabilityKind = RELIABLE;
		Wparam.topic.topicName = "DCPSSubscription";
		Wparam.topic.topicKind = WITH_KEY;
		Wparam.topic.topicDataType = "DiscoveredReaderData";
		Wparam.userDefinedId = -1;
		Wparam.unicastLocatorList = this->mp_DPDP->mp_localPDP->m_metatrafficUnicastLocatorList;
		Wparam.multicastLocatorList = this->mp_DPDP->mp_localPDP->m_metatrafficMulticastLocatorList;
		created &=mp_participant->createStatefulWriter(&mp_SubWriter,Wparam,DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE);
		mp_SubWriter->m_guid.entityId = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER;
	}
	if(m_discovery.m_simpleEDP.use_Subscription_Reader)
	{
		Rparam.historyMaxSize = 100;
		Rparam.expectsInlineQos = false;
		Rparam.reliability.reliabilityKind = RELIABLE;
		Rparam.topic.topicName = "DCPSSubscription";
		Rparam.topic.topicKind = WITH_KEY;
		Rparam.topic.topicDataType = "DiscoveredReaderData";
		Rparam.userDefinedId = -1;
		Rparam.unicastLocatorList = this->mp_DPDP->mp_localPDP->m_metatrafficUnicastLocatorList;
		Rparam.multicastLocatorList = this->mp_DPDP->mp_localPDP->m_metatrafficMulticastLocatorList;
		created &=mp_participant->createStatefulReader(&mp_PubReader,Rparam,DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE);
		mp_PubWriter->m_guid.entityId = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER;
	}
	if(m_discovery.m_simpleEDP.use_Topic_Writer)
	{
		Wparam.historyMaxSize = 100;
		Wparam.pushMode = true;
		Wparam.reliability.reliabilityKind = RELIABLE;
		Wparam.topic.topicName = "DCPSTopic";
		Wparam.topic.topicKind = WITH_KEY;
		Wparam.topic.topicDataType = "DiscoveredTopicData";
		Wparam.userDefinedId = -1;
		Wparam.unicastLocatorList = this->mp_DPDP->mp_localPDP->m_metatrafficUnicastLocatorList;
		Wparam.multicastLocatorList = this->mp_DPDP->mp_localPDP->m_metatrafficMulticastLocatorList;
		created &=mp_participant->createStatefulWriter(&mp_SubWriter,Wparam,DISCOVERY_TOPIC_DATA_MAX_SIZE);
		mp_SubWriter->m_guid.entityId = ENTITYID_SEDP_BUILTIN_TOPIC_WRITER;
	}
	if(m_discovery.m_simpleEDP.use_Topic_Reader)
	{
		Rparam.historyMaxSize = 100;
		Rparam.expectsInlineQos = false;
		Rparam.reliability.reliabilityKind = RELIABLE;
		Rparam.topic.topicName = "DCPSTopic";
		Rparam.topic.topicKind = WITH_KEY;
		Rparam.topic.topicDataType = "DiscoveredTopicData";
		Rparam.userDefinedId = -1;
		Rparam.unicastLocatorList = this->mp_DPDP->mp_localPDP->m_metatrafficUnicastLocatorList;
		Rparam.multicastLocatorList = this->mp_DPDP->mp_localPDP->m_metatrafficMulticastLocatorList;
		created &=mp_participant->createStatefulReader(&mp_PubReader,Rparam,DISCOVERY_TOPIC_DATA_MAX_SIZE);
		mp_PubWriter->m_guid.entityId = ENTITYID_SEDP_BUILTIN_TOPIC_READER;
	}
	return created;
}

void SimpleEDP::assignRemoteEndpoints(DiscoveredParticipantData* pdata)
{
	uint32_t endp = pdata->m_availableBuiltinEndpoints;
	uint32_t auxendp = endp;
	auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
	if(auxendp!=0) //Exist Pub Announcer
	{
		WriterProxy_t wp;
		wp.remoteWriterGuid.guidPrefix = pdata->m_guidPrefix;
		wp.remoteWriterGuid.entityId = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER;
		wp.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		wp.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		mp_PubReader->matched_writer_add(&wp);
	}
	auxendp = endp;
	auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
	if(auxendp!=0) //Exist Pub Announcer
	{
		ReaderProxy_t rp;
		rp.expectsInlineQos = false;
		rp.m_reliablility = RELIABLE;
		rp.remoteReaderGuid.guidPrefix = pdata->m_guidPrefix;
		rp.remoteReaderGuid.entityId = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER;
		rp.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		rp.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		mp_PubWriter->matched_reader_add(rp);
	}
	auxendp = endp;
	auxendp &=DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
	if(auxendp!=0) //Exist Pub Announcer
	{
		WriterProxy_t wp;
		wp.remoteWriterGuid.guidPrefix = pdata->m_guidPrefix;
		wp.remoteWriterGuid.entityId = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER;
		wp.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		wp.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		mp_SubReader->matched_writer_add(&wp);
	}
	auxendp = endp;
	auxendp &=DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
	if(auxendp!=0) //Exist Pub Announcer
	{
		ReaderProxy_t rp;
		rp.expectsInlineQos = false;
		rp.m_reliablility = RELIABLE;
		rp.remoteReaderGuid.guidPrefix = pdata->m_guidPrefix;
		rp.remoteReaderGuid.entityId = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER;
		rp.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		rp.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		mp_SubWriter->matched_reader_add(rp);
	}
//	auxendp = endp;
//	auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
//	if(auxendp!=0) //Exist Pub Announcer
//	{
//		int uax =0;
//	}
//	auxendp = endp;
//	auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
//	if(auxendp!=0) //Exist Pub Announcer
//	{
//		int uax =0;
//	}
}

} /* namespace rtps */
} /* namespace eprosima */
