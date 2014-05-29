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
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"

#include "eprosimartps/discovery/data/DiscoveredData.h"
#include "eprosimartps/discovery/data/DiscoveredWriterData.h"
#include "eprosimartps/discovery/data/DiscoveredReaderData.h"
#include "eprosimartps/discovery/data/DiscoveredParticipantData.h"

#include "eprosimartps/dds/PublisherListener.h"
#include "eprosimartps/dds/SubscriberListener.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/IPFinder.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

SimpleEDP::SimpleEDP(ParticipantDiscoveryProtocol* p):
				EndpointDiscoveryProtocol(p),
				 mp_PubWriter(NULL),mp_SubWriter(NULL),// mp_TopWriter(NULL),
				 mp_PubReader(NULL),mp_SubReader(NULL),// mp_TopReader(NULL),
				 m_listeners(this)
{


}

SimpleEDP::~SimpleEDP()
{

}

bool SimpleEDP::initEDP(DiscoveryAttributes& attributes)
{
	pInfo(B_CYAN<<"Initializing EndpointDiscoveryProtocol"<<endl);
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
	pInfo(CYAN<<"Creating SEDP Endpoints"<<DEF<<endl);
	PublisherAttributes Wparam;
	SubscriberAttributes Rparam;
	bool created = true;
	RTPSReader* raux;
	RTPSWriter* waux;
	if(m_discovery.m_simpleEDP.use_Publication_Writer)
	{
		Wparam.historyMaxSize = 100;
		Wparam.pushMode = true;
		Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		Wparam.topic.topicName = "DCPSPublication";
		Wparam.topic.topicKind = WITH_KEY;
		Wparam.topic.topicDataType = "DiscoveredWriterData";
		Wparam.userDefinedId = -1;
		Wparam.unicastLocatorList = this->mp_PDP->mp_localDPData->m_metatrafficUnicastLocatorList;
		Wparam.multicastLocatorList = this->mp_PDP->mp_localDPData->m_metatrafficMulticastLocatorList;
		created &=this->mp_PDP->mp_participant->createWriter(&waux,Wparam,DISCOVERY_PUBLICATION_DATA_MAX_SIZE,true,STATEFUL,NULL,NULL,c_EntityId_SEDPPubWriter);
		if(created)
		{
			mp_PubWriter = dynamic_cast<StatefulWriter*>(waux);
			pInfo(CYAN<<"SEDP Publication Writer created"<<DEF<<endl);
		}
	}
	if(m_discovery.m_simpleEDP.use_Publication_Reader)
	{
		Rparam.historyMaxSize = 100;
		Rparam.expectsInlineQos = false;
		Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		Rparam.topic.topicName = "DCPSPublication";
		Rparam.topic.topicKind = WITH_KEY;
		Rparam.topic.topicDataType = "DiscoveredWriterData";
		Rparam.userDefinedId = -1;
		Rparam.unicastLocatorList = this->mp_PDP->mp_localDPData->m_metatrafficUnicastLocatorList;
		Rparam.multicastLocatorList = this->mp_PDP->mp_localDPData->m_metatrafficMulticastLocatorList;
		created &=this->mp_PDP->mp_participant->createReader(&raux,Rparam,DISCOVERY_PUBLICATION_DATA_MAX_SIZE,true,STATEFUL,NULL,NULL,c_EntityId_SEDPPubReader);
		if(created)
		{
			mp_PubReader = dynamic_cast<StatefulReader*>(raux);
			mp_PubReader->setListener((SubscriberListener*)&m_listeners.m_PubListener);
			pInfo(CYAN<<"SEDP Publication Reader created"<<DEF<<endl);
		}
	}
	if(m_discovery.m_simpleEDP.use_Subscription_Writer)
	{
		Wparam.historyMaxSize = 100;
		Wparam.pushMode = true;
		Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		Wparam.topic.topicName = "DCPSSubscription";
		Wparam.topic.topicKind = WITH_KEY;
		Wparam.topic.topicDataType = "DiscoveredReaderData";
		Wparam.userDefinedId = -1;
		Wparam.unicastLocatorList = this->mp_PDP->mp_localDPData->m_metatrafficUnicastLocatorList;
		Wparam.multicastLocatorList = this->mp_PDP->mp_localDPData->m_metatrafficMulticastLocatorList;
		created &=this->mp_PDP->mp_participant->createWriter(&waux,Wparam,DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE,true,STATEFUL,NULL,NULL,c_EntityId_SEDPSubWriter);
		if(created)
		{
			mp_SubWriter = dynamic_cast<StatefulWriter*>(waux);
			pInfo(CYAN<<"SEDP Subscription Writer created"<<DEF<<endl);
		}
	}
	if(m_discovery.m_simpleEDP.use_Subscription_Reader)
	{
		Rparam.historyMaxSize = 100;
		Rparam.expectsInlineQos = false;
		Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
		Rparam.topic.topicName = "DCPSSubscription";
		Rparam.topic.topicKind = WITH_KEY;
		Rparam.topic.topicDataType = "DiscoveredReaderData";
		Rparam.unicastLocatorList = this->mp_PDP->mp_localDPData->m_metatrafficUnicastLocatorList;
				Rparam.multicastLocatorList = this->mp_PDP->mp_localDPData->m_metatrafficMulticastLocatorList;
		Rparam.userDefinedId = -1;
		//FIXME:Rparam.unicastLocatorList = this->mp_PDP->mp_localDPData->m_metatrafficUnicastLocatorList;
		Rparam.multicastLocatorList = this->mp_PDP->mp_localDPData->m_metatrafficMulticastLocatorList;
		created &=this->mp_PDP->mp_participant->createReader(&raux,Rparam,DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE,true,STATEFUL,NULL,NULL,c_EntityId_SEDPSubReader);
		if(created)
		{
			mp_SubReader = dynamic_cast<StatefulReader*>(raux);
			mp_SubReader->setListener((SubscriberListener*)&m_listeners.m_SubListener);
			pInfo(CYAN<<"SEDP Subscription Reader created"<<DEF<<endl);
		}
	}
	pInfo(CYAN<<"SimpleEDP Endpoints creation finished"<<DEF<<endl);
	return created;
}

void SimpleEDP::assignRemoteEndpoints(DiscoveredParticipantData* pdata)
{
	pInfo(CYAN<<"SimpleEDP:assignRemoteEndpoints: new DPD received, adding remote endpoints to our SimpleEDP endpoints"<<DEF<<endl);
	uint32_t endp = pdata->m_availableBuiltinEndpoints;
	uint32_t auxendp = endp;
	auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER;
	if(auxendp!=0 && mp_PubReader!=NULL) //Exist Pub Writer and i have pub reader
	{
		pDebugInfo(CYAN<<"Adding SEDP Pub Writer to my Pub Reader"<<DEF<<endl);
		WriterProxy_t wp;
		wp.remoteWriterGuid.guidPrefix = pdata->m_guidPrefix;
		wp.remoteWriterGuid.entityId = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER;
		wp.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		wp.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		mp_PubReader->matched_writer_add(wp);
	}
	auxendp = endp;
	auxendp &=DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR;
	if(auxendp!=0 && mp_PubWriter!=NULL) //Exist Pub Detector
	{
		pDebugInfo(CYAN<<"Adding SEDP Pub Reader to my Pub Writer"<<DEF<<endl);
		ReaderProxy_t rp;
		rp.expectsInlineQos = false;
		rp.m_reliability = RELIABLE;
		rp.remoteReaderGuid.guidPrefix = pdata->m_guidPrefix;
		rp.remoteReaderGuid.entityId = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER;
		rp.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		rp.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		mp_PubWriter->matched_reader_add(rp);
	}
	auxendp = endp;
	auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER;
	if(auxendp!=0 && mp_SubReader!=NULL) //Exist Pub Announcer
	{
		pDebugInfo(CYAN<<"Adding SEDP Sub Writer to my Sub Reader"<<DEF<<endl);
		WriterProxy_t wp;
		wp.remoteWriterGuid.guidPrefix = pdata->m_guidPrefix;
		wp.remoteWriterGuid.entityId = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER;
		wp.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		wp.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		mp_SubReader->matched_writer_add(wp);
	}
	auxendp = endp;
	auxendp &= DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR;
	if(auxendp!=0 && mp_SubWriter!=NULL) //Exist Pub Announcer
	{
		pDebugInfo(CYAN<<"Adding SEDP Sub Reader to my Sub Writer"<<DEF<<endl);
		ReaderProxy_t rp;
		rp.expectsInlineQos = false;
		rp.m_reliability = RELIABLE;
		rp.remoteReaderGuid.guidPrefix = pdata->m_guidPrefix;
		rp.remoteReaderGuid.entityId = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER;
		rp.unicastLocatorList = pdata->m_metatrafficUnicastLocatorList;
		rp.multicastLocatorList = pdata->m_metatrafficMulticastLocatorList;
		mp_SubWriter->matched_reader_add(rp);
	}
}

bool SimpleEDP::addNewLocalWriter(RTPSWriter* W)
{
	if(mp_PubWriter!=NULL)
	{
		DiscoveredWriterData* wdata = new DiscoveredWriterData();
		wdata->m_writerProxy.unicastLocatorList = W->unicastLocatorList;
		repareDiscoveredDataLocatorList(&wdata->m_writerProxy.unicastLocatorList);
		wdata->m_writerProxy.multicastLocatorList = W->multicastLocatorList;
		wdata->m_writerProxy.remoteWriterGuid = W->getGuid();
		wdata->m_key = W->getGuid();
		wdata->m_participantKey = this->mp_PDP->mp_participant->getGuid();
		wdata->m_topicName = W->getTopic().getTopicName();
		wdata->m_typeName = W->getTopic().getTopicDataType();
		wdata->topicKind = W->getTopic().getTopicKind();
		wdata->m_qos = W->getQos();
		this->mp_PDP->mp_localDPData->m_writers.push_back(wdata);
		//Create a new change in History:
		CacheChange_t* change = NULL;
		if(mp_PubWriter->new_change(ALIVE,NULL,&change))
		{
			change->instanceHandle = wdata->m_key;
			ParameterList_t param;
			DiscoveredData::DiscoveredWriterData2ParameterList(*wdata,&param);
			ParameterList::updateCDRMsg(&param,EPROSIMA_ENDIAN);
			change->serializedPayload.encapsulation = EPROSIMA_ENDIAN == BIGEND ? PL_CDR_BE: PL_CDR_LE;
			change->serializedPayload.length = param.m_cdrmsg.length;
			memcpy(change->serializedPayload.data,param.m_cdrmsg.buffer,change->serializedPayload.length);
			mp_PubWriter->add_change(change);
			mp_PubWriter->unsent_change_add(change);
		}
	}
	return true;
}

bool SimpleEDP::localWriterMatching(RTPSWriter* W, bool first_time)
{
	pDebugInfo(CYAN<<"SimpleEDP: localWriterMatching "<<DEF<<endl);
	if(first_time)
	{
		pDebugInfo(CYAN << "SimpleEDP: adding New Local Writer Info"<<DEF<<endl);
		addNewLocalWriter(W);
	}
	bool matched = false;
	for(std::vector<DiscoveredParticipantData*>::iterator pit = this->mp_PDP->m_discoveredParticipants.begin();
			pit!=this->mp_PDP->m_discoveredParticipants.end();++pit)
	{
		for(std::vector<DiscoveredReaderData*>::iterator rit = (*pit)->m_readers.begin();
				rit!=(*pit)->m_readers.end();++rit)
		{
			matched |= localWriterMatching(W,(*rit));
		}
	}
	return matched;
}



bool SimpleEDP::localReaderMatching(RTPSReader* R, bool first_time)
{
	pDebugInfo(CYAN<<"SimpleEDP: localReaderMatching "<<DEF<<endl);
	if(first_time)
	{
		addNewLocalReader(R);
	}
	bool matched = false;
	for(std::vector<DiscoveredParticipantData*>::iterator pit = this->mp_PDP->m_discoveredParticipants.begin();
			pit!=this->mp_PDP->m_discoveredParticipants.end();++pit)
	{
//		cout << "partname " << (*pit)->m_participantName << endl;
//		cout << "partic Pref" << (*pit)->m_guidPrefix << endl;
//		cout << "partic Key" << (*pit)->m_key << endl;
		for(std::vector<DiscoveredWriterData*>::iterator wit = (*pit)->m_writers.begin();
				wit!=(*pit)->m_writers.end();++wit)
		{
			matched |= localReaderMatching(R,*wit);
		}
	}
	return matched;
}

bool SimpleEDP::updateWriterMatching(RTPSWriter* writer,DiscoveredReaderData* rdata)
{
	pError("updateWriterMatching Not YET implemented "<<endl);
	return true;
}

bool SimpleEDP::updateReaderMatching(RTPSReader* reader,DiscoveredWriterData* wdata)
{
	pError("updateReaderMatching Not YET implemented "<<endl);
	return true;
}

bool SimpleEDP::addNewLocalReader(RTPSReader* R)
{
	if(mp_SubWriter!=NULL)
	{
		DiscoveredReaderData* rdata = new DiscoveredReaderData();
		rdata->m_readerProxy.unicastLocatorList = R->unicastLocatorList;
		repareDiscoveredDataLocatorList(&rdata->m_readerProxy.unicastLocatorList);
		rdata->m_readerProxy.multicastLocatorList = R->multicastLocatorList;
		rdata->m_readerProxy.remoteReaderGuid = R->getGuid();
		rdata->m_readerProxy.expectsInlineQos = R->expectsInlineQos();
		rdata->m_key = R->getGuid();
		rdata->m_participantKey = this->mp_PDP->mp_participant->getGuid();
		rdata->m_topicName = R->getTopic().getTopicName();
		rdata->m_typeName = R->getTopic().getTopicDataType();
		rdata->topicKind = R->getTopic().getTopicKind();
		rdata->m_qos = R->getQos();
		this->mp_PDP->mp_localDPData->m_readers.push_back(rdata);
		//Create a new change in History:
		CacheChange_t* change = NULL;
		if(mp_SubWriter->new_change(ALIVE,NULL,&change))
		{
			change->instanceHandle = rdata->m_key;
			ParameterList_t param;
			DiscoveredData::DiscoveredReaderData2ParameterList(*rdata,&param);
			ParameterList::updateCDRMsg(&param,EPROSIMA_ENDIAN);
			change->serializedPayload.encapsulation = EPROSIMA_ENDIAN == BIGEND ? PL_CDR_BE: PL_CDR_LE;
			change->serializedPayload.length = param.m_cdrmsg.length;
			memcpy(change->serializedPayload.data,param.m_cdrmsg.buffer,change->serializedPayload.length);
			mp_SubWriter->add_change(change);
			mp_SubWriter->unsent_change_add(change);
		}
	}
	return true;
}

void SimpleEDP::repareDiscoveredDataLocatorList(LocatorList_t* loclist)
{
	bool iszero = false;
	LocatorList_t newLocList;
	LocatorList_t myIPs;
	IPFinder::getIPAddress(&myIPs);
	Locator_t auxLoc;
	for(LocatorListIterator lit = loclist->begin();lit!=loclist->end();++lit)
	{
		iszero = true;
		for(uint8_t i = 0;i<16;++i)
		{
			if(lit->address[i]!=0)
			{
				iszero= false;
				break;
			}
		}
		if(iszero)
		{
			for(LocatorListIterator myit = myIPs.begin();myit!=myIPs.end();++myit)
			{
				auxLoc = *myit;
				auxLoc.port = lit->port;
				newLocList.push_back(auxLoc);
			}
		}
		else
		{
			newLocList.push_back(*lit);
		}
	}
	*loclist = newLocList;
}

bool SimpleEDP::localWriterMatching(RTPSWriter* W,DiscoveredReaderData* rdata)
{
	pInfo("SimpleEDP:localWriterMatching W-DRD"<<endl);
	bool matched = false;
	if(W->getTopic().getTopicName() == rdata->m_topicName && W->getTopic().getTopicDataType() == rdata->m_typeName &&
			W->getTopic().getTopicKind() == rdata->topicKind && rdata->isAlive)
	{
		if(W->getStateType() == STATELESS && rdata->m_qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS)
		{
			StatelessWriter* p_SLW = (StatelessWriter*)W;
			ReaderLocator RL;
			RL.expectsInlineQos = rdata->m_readerProxy.expectsInlineQos;
			for(std::vector<Locator_t>::iterator lit = rdata->m_readerProxy.unicastLocatorList.begin();
					lit != rdata->m_readerProxy.unicastLocatorList.end();++lit)
			{
				//cout << "added unicast RL to my STATELESSWRITER"<<endl;
				RL.locator = *lit;
				if(p_SLW->reader_locator_add(RL))
					matched =true;
			}
			for(std::vector<Locator_t>::iterator lit = rdata->m_readerProxy.multicastLocatorList.begin();
					lit != rdata->m_readerProxy.multicastLocatorList.end();++lit)
			{
				RL.locator = *lit;
				if(p_SLW->reader_locator_add(RL))
					matched = true;
			}
		}
		else if(W->getStateType() == STATEFUL)
		{
			StatefulWriter* p_SFW = (StatefulWriter*)W;
			if(p_SFW->matched_reader_add(rdata->m_readerProxy))
				matched = true;
		}
		if(matched && W->getListener()!=NULL)
			W->getListener()->onPublicationMatched();
	}
	return matched;
}

bool SimpleEDP::localReaderMatching(RTPSReader* R,DiscoveredWriterData* wdata)
{
	pInfo("SimpleEDP: localReader with Discovered Writer Matching"<<endl);
	bool matched = false;
//	cout << R->getTopic().getTopicName() << " " <<wdata->m_topicName<<endl;
//	cout << R->getTopic().getTopicKind() << " " <<wdata->topicKind<<endl;
//	cout << R->getTopic().getTopicDataType() << " " <<wdata->m_typeName<<endl;
//	cout << "wdata alive. "<<wdata->isAlive<<endl;
	if(		R->getTopic().getTopicName() == wdata->m_topicName &&
			R->getTopic().getTopicKind() == wdata->topicKind &&
			R->getTopic().getTopicDataType() == wdata->m_typeName &&
			wdata->isAlive) //Matching
	{
		if(R->getStateType() == STATELESS)
		{
			pInfo("BEST_EFFORT Readers don't need matching"<<endl);
			matched = true;
		}
		else if(R->getStateType() == STATEFUL && wdata->m_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS)
		{
			StatefulReader* p_SFR = (StatefulReader*)R;
			if(p_SFR->matched_writer_add(wdata->m_writerProxy))
				matched = true;
		}
		if(matched && R->getListener()!=NULL)
		{
			R->getListener()->onSubscriptionMatched();
		}
	}
	return matched;
}



} /* namespace rtps */
} /* namespace eprosima */
