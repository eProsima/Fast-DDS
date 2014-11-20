/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDP.cpp
 *
 */

#include "eprosimartps/builtin/discovery/endpoint/EDP.h"

#include "eprosimartps/builtin/discovery/RTPSParticipant/PDPSimple.h"

#include "eprosimartps/RTPSParticipant.h"
#include "eprosimartps/RTPSParticipantProxyData.h"

#include "eprosimartps/writer/StatefulWriter.h"
#include "eprosimartps/reader/StatefulReader.h"
#include "eprosimartps/writer/StatelessWriter.h"
#include "eprosimartps/reader/StatelessReader.h"

#include "eprosimartps/reader/WriterProxyData.h"
#include "eprosimartps/writer/ReaderProxyData.h"

#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/utils/IPFinder.h"
#include "eprosimartps/utils/StringMatching.h"


#include "eprosimartps/pubsub/PublisherListener.h"
#include "eprosimartps/pubsub/SubscriberListener.h"

using namespace eprosima::pubsub;

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "EDP";

EDP::EDP(PDPSimple* p,RTPSParticipantImpl* part):
	mp_PDP(p),
	mp_RTPSParticipant(part)
{
	// TODO Auto-generated constructor stub

}

EDP::~EDP()
{
	// TODO Auto-generated destructor stub
}

bool EDP::newLocalReaderProxyData(RTPSReader* reader)
{
	const char* const METHOD_NAME = "newLocalReaderProxyData";
	logInfo(RTPS_EDP,"Adding " << reader->getGuid().entityId << " in topic "<<reader->getTopic().topicName,EPRO_CYAN);
	ReaderProxyData* rpd = new ReaderProxyData();
	rpd->m_isAlive = true;
	rpd->m_expectsInlineQos = reader->expectsInlineQos();
	rpd->m_guid = reader->getGuid();
	rpd->m_key = rpd->m_guid;
	rpd->m_multicastLocatorList = reader->multicastLocatorList;
	rpd->m_unicastLocatorList = reader->unicastLocatorList;
	rpd->m_RTPSParticipantKey = mp_RTPSParticipant->getGuid();
	rpd->m_topicName = reader->getTopic().getTopicName();
	rpd->m_typeName = reader->getTopic().getTopicDataType();
	rpd->m_topicKind = reader->getTopic().getTopicKind();
	rpd->m_qos = reader->getQos();
	rpd->m_userDefinedId = reader->getUserDefinedId();
	//ADD IT TO THE LIST OF READERPROXYDATA
	if(!this->mp_PDP->addReaderProxyData(rpd))
	{
		delete(rpd);
		return false;
	}
	//DO SOME PROCESSING DEPENDING ON THE IMPLEMENTATION (SIMPLE OR STATIC)
	processLocalReaderProxyData(rpd);
	//PAIRING
	pairingReader(reader);
	pairingReaderProxy(rpd);
	return true;
}

bool EDP::newLocalWriterProxyData(RTPSWriter* writer)
{
	const char* const METHOD_NAME = "newLocalWriterProxyData";
	logInfo(RTPS_EDP,"Adding " << writer->getGuid().entityId << " in topic "<<writer->getTopic().topicName,EPRO_CYAN);
	WriterProxyData* wpd = new WriterProxyData();
	wpd->m_isAlive = true;
	wpd->m_guid = writer->getGuid();
	wpd->m_key = wpd->m_guid;
	wpd->m_multicastLocatorList = writer->multicastLocatorList;
	wpd->m_unicastLocatorList = writer->unicastLocatorList;
	wpd->m_RTPSParticipantKey = mp_RTPSParticipant->getGuid();
	wpd->m_topicName = writer->getTopic().getTopicName();
	wpd->m_typeName = writer->getTopic().getTopicDataType();
	wpd->m_topicKind = writer->getTopic().getTopicKind();
	wpd->m_typeMaxSerialized = writer->mp_type->m_typeSize;
	wpd->m_qos = writer->getQos();
	wpd->m_userDefinedId = writer->getUserDefinedId();
	//ADD IT TO THE LIST OF READERPROXYDATA
	if(!this->mp_PDP->addWriterProxyData(wpd))
	{
		delete(wpd);
		return false;
	}
	//DO SOME PROCESSING DEPENDING ON THE IMPLEMENTATION (SIMPLE OR STATIC)
	processLocalWriterProxyData(wpd);
	//PAIRING
	pairingWriterProxy(wpd);
	pairingWriter(writer);
	return true;
}

bool EDP::updatedLocalReader(RTPSReader* R)
{
	ReaderProxyData* rdata = nullptr;
	if(this->mp_PDP->lookupReaderProxyData(R->getGuid(),&rdata))
	{
		rdata->m_qos.setQos(R->getQos(),false);
		rdata->m_expectsInlineQos = R->expectsInlineQos();
		processLocalReaderProxyData(rdata);
		//this->updatedReaderProxy(rdata);
		pairingReaderProxy(rdata);
		pairingReader(R);
		return true;
	}
	return false;
}

bool EDP::updatedLocalWriter(RTPSWriter* W)
{
	WriterProxyData* wdata = nullptr;
	if(this->mp_PDP->lookupWriterProxyData(W->getGuid(),&wdata))
	{
		wdata->m_qos.setQos(W->getQos(),false);
		processLocalWriterProxyData(wdata);
		//this->updatedWriterProxy(wdata);
		pairingWriterProxy(wdata);
		pairingWriter(W);
		return true;
	}
	return false;
}


bool EDP::removeWriterProxy(const GUID_t& writer)
{
	const char* const METHOD_NAME = "removeWriterProxy";
	logInfo(RTPS_EDP,writer,EPRO_CYAN);
	WriterProxyData* wdata = nullptr;
	if(this->mp_PDP->lookupWriterProxyData(writer,&wdata))
	{
		unpairWriterProxy(wdata);
		this->mp_PDP->removeWriterProxyData(wdata);
		return true;
	}
	return false;
}

bool EDP::removeReaderProxy(const GUID_t& reader)
{
	const char* const METHOD_NAME = "removeReaderProxy";
	logInfo(RTPS_EDP,reader,EPRO_CYAN);
	ReaderProxyData* rdata = nullptr;
	if(this->mp_PDP->lookupReaderProxyData(reader,&rdata))
	{
		unpairReaderProxy(rdata);
		this->mp_PDP->removeReaderProxyData(rdata);
		return true;
	}
	return false;
}

bool EDP::unpairWriterProxy(WriterProxyData* wdata)
{
	const char* const METHOD_NAME = "unpairWriterProxy";
	logInfo(RTPS_EDP,wdata->m_guid << " in topic: "<< wdata->m_topicName,EPRO_CYAN);
	for(std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
			rit!=mp_RTPSParticipant->userReadersListEnd();++rit)
	{
		if((*rit)->matched_writer_remove(wdata))
		{
			//MATCHED AND ADDED CORRECTLY:
			if((*rit)->getListener()!=nullptr)
			{
				MatchingInfo info;
				info.status = REMOVED_MATCHING;
				info.remoteEndpointGuid = wdata->m_guid;
				(*rit)->getListener()->onSubscriptionMatched(info);
			}
		}
	}
	return true;
}

bool EDP::unpairReaderProxy(ReaderProxyData* rdata)
{
	const char* const METHOD_NAME = "unpairReaderProxy";
		logInfo(RTPS_EDP,rdata->m_guid << " in topic: "<< rdata->m_topicName,EPRO_CYAN);
	for(std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
			wit!=mp_RTPSParticipant->userWritersListEnd();++wit)
	{
		if((*wit)->matched_reader_remove(rdata))
		{
			//MATCHED AND ADDED CORRECTLY:
			if((*wit)->getListener()!=nullptr)
			{
				MatchingInfo info;
				info.status = REMOVED_MATCHING;
				info.remoteEndpointGuid = rdata->m_guid;
				(*wit)->getListener()->onPublicationMatched(info);
			}
		}
	}
	return true;
}


bool EDP::validMatching(RTPSWriter* W,ReaderProxyData* rdata)
{
	const char* const METHOD_NAME = "validMatching(W2RP)";
	if(W->getTopic().getTopicName() != rdata->m_topicName)
		return false;
	if(W->getTopic().getTopicDataType() != rdata->m_typeName)
		return false;
	if(W->getTopic().getTopicKind() != rdata->m_topicKind)
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS:Remote Reader "<<rdata->m_guid << " is publishing in topic "
				<< rdata->m_topicName << "(keyed:"<<rdata->m_topicKind<<
				"), local writer publishes as keyed: "<<W->getTopic().getTopicKind(),EPRO_CYAN)
																																														return false;
	}
	if(!rdata->m_isAlive) //Matching
	{
		logWarning(RTPS_EDP,"ReaderProxyData object is NOT alive",EPRO_CYAN);
		return false;
	}
	if(W->getStateType() == STATELESS && rdata->m_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS) //Means our writer is BE but the reader wants RE
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< rdata->m_topicName<<"):Remote Reader "
				<<rdata->m_guid << " is Reliable and local writer is BE ",EPRO_CYAN);
		return false;
	}
	if(W->getQos().m_durability.kind == VOLATILE_DURABILITY_QOS && rdata->m_qos.m_durability.kind == TRANSIENT_LOCAL_DURABILITY_QOS)
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< rdata->m_topicName<<"):RemoteReader "
				<<rdata->m_guid << " has TRANSIENT_LOCAL DURABILITY and we offer VOLATILE",EPRO_CYAN);
		return false;
	}
	if(W->getQos().m_ownership.kind != rdata->m_qos.m_ownership.kind)
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< rdata->m_topicName<<"):Remote reader "
				<<rdata->m_guid << " has different Ownership Kind",EPRO_CYAN);
		return false;
	}
	//Partition check:
	bool matched = false;
	if(W->getQos().m_partition.names.empty() && rdata->m_qos.m_partition.names.empty())
	{
		matched = true;
	}
	else if(W->getQos().m_partition.names.empty() && rdata->m_qos.m_partition.names.size()>0)
	{
		for(std::vector<std::string>::const_iterator rnameit = rdata->m_qos.m_partition.names.begin();
					rnameit!=rdata->m_qos.m_partition.names.end();++rnameit)
		{
			if(rnameit->size()==0)
			{
				matched = true;
				break;
			}
		}
	}
	else if(W->getQos().m_partition.names.size()>0 && rdata->m_qos.m_partition.names.empty() )
	{
		for(std::vector<std::string>::const_iterator wnameit = W->getQos().m_partition.names.begin();
				wnameit !=  W->getQos().m_partition.names.end();++wnameit)
		{
			if(wnameit->size()==0)
			{
				matched = true;
				break;
			}
		}
	}
	else
	{
		for(std::vector<std::string>::const_iterator wnameit = W->getQos().m_partition.names.begin();
				wnameit !=  W->getQos().m_partition.names.end();++wnameit)
		{
			for(std::vector<std::string>::const_iterator rnameit = rdata->m_qos.m_partition.names.begin();
					rnameit!=rdata->m_qos.m_partition.names.end();++rnameit)
			{
				if(StringMatching::matchString(wnameit->c_str(),rnameit->c_str()))
				{
					matched = true;
					break;
				}
			}
			if(matched)
				break;
		}
	}
	if(!matched) //Different partitions
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< rdata->m_topicName<<"): Different Partitions",EPRO_CYAN);
	return matched;
}
bool EDP::validMatching(RTPSReader* R,WriterProxyData* wdata)
{
	const char* const METHOD_NAME = "validMatching(R2WP)";
	if(R->getTopic().getTopicName() != wdata->m_topicName)
		return false;
	if(R->getTopic().getTopicDataType() != wdata->m_typeName)
		return false;
	if(R->getTopic().getTopicKind() != wdata->m_topicKind)
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS:Remote Writer "<<wdata->m_guid << " is publishing in topic " << wdata->m_topicName << "(keyed:"<<wdata->m_topicKind<<
				"), local reader subscribes as keyed: "<<R->getTopic().getTopicKind(),EPRO_CYAN)
																																																	return false;
	}
	if(!wdata->m_isAlive) //Matching
	{
		logWarning(RTPS_EDP,"WriterProxyData " << wdata->m_guid << " is NOT alive",EPRO_CYAN);
		return false;
	}
	if(R->getStateType() == STATEFUL && wdata->m_qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS) //Means our reader is reliable but hte writer is not
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< wdata->m_topicName<<"): Remote Writer "<<wdata->m_guid << " is Best Effort and local reader is RELIABLE "<<endl;);
		return false;
	}
	if(R->getQos().m_durability.kind == TRANSIENT_LOCAL_DURABILITY_QOS && wdata->m_qos.m_durability.kind == VOLATILE_DURABILITY_QOS)
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< wdata->m_topicName<<"):RemoteWriter "<<wdata->m_guid << " has VOLATILE DURABILITY and we want TRANSIENT_LOCAL"<<endl;);
		return false;
	}
	if(R->getQos().m_ownership.kind != wdata->m_qos.m_ownership.kind)
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< wdata->m_topicName<<"):Remote Writer "<<wdata->m_guid << " has different Ownership Kind"<<endl;);
		return false;
	}
	//Partition check:
	bool matched = false;
	if(R->getQos().m_partition.names.empty() && wdata->m_qos.m_partition.names.empty())
	{
		matched = true;
	}
	else if(R->getQos().m_partition.names.empty() && wdata->m_qos.m_partition.names.size()>0)
	{
		for(std::vector<std::string>::const_iterator rnameit = wdata->m_qos.m_partition.names.begin();
					rnameit!=wdata->m_qos.m_partition.names.end();++rnameit)
		{
			if(rnameit->size()==0)
			{
				matched = true;
				break;
			}
		}
	}
	else if(R->getQos().m_partition.names.size()>0 && wdata->m_qos.m_partition.names.empty() )
	{
		for(std::vector<std::string>::const_iterator wnameit = R->getQos().m_partition.names.begin();
				wnameit !=  R->getQos().m_partition.names.end();++wnameit)
		{
			if(wnameit->size()==0)
			{
				matched = true;
				break;
			}
		}
	}
	else
	{
		for(std::vector<std::string>::const_iterator wnameit = R->getQos().m_partition.names.begin();
				wnameit !=  R->getQos().m_partition.names.end();++wnameit)
		{
			for(std::vector<std::string>::const_iterator rnameit = wdata->m_qos.m_partition.names.begin();
					rnameit!=wdata->m_qos.m_partition.names.end();++rnameit)
			{
				if(StringMatching::matchString(wnameit->c_str(),rnameit->c_str()))
				{
					matched = true;
					break;
				}
			}
			if(matched)
				break;
		}
	}
	if(!matched) //Different partitions
			logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< wdata->m_topicName<<"): Different Partitions",EPRO_CYAN);
	return matched;
}


bool EDP::pairingReader(RTPSReader* R)
{
	const char* const METHOD_NAME = "pairingReader";
	logInfo(RTPS_EDP,R->getGuid()<<" in topic: \"" << R->getTopic().getTopicName()<<"\"",EPRO_CYAN);
	for(std::vector<RTPSParticipantProxyData*>::const_iterator pit = mp_PDP->RTPSParticipantProxiesBegin();
			pit!=mp_PDP->RTPSParticipantProxiesEnd();++pit)
	{
		for(std::vector<WriterProxyData*>::iterator wdatait = (*pit)->m_writers.begin();
				wdatait!=(*pit)->m_writers.end();++wdatait)
		{
			if(validMatching(R,*wdatait))
			{
				logInfo(RTPS_EDP,"Valid Matching to writerProxy: "<<(*wdatait)->m_guid,EPRO_CYAN);
				if(R->matched_writer_add(*wdatait))
				{
					//MATCHED AND ADDED CORRECTLY:
					if(R->getListener()!=nullptr)
					{
						MatchingInfo info;
						info.status = MATCHED_MATCHING;
						info.remoteEndpointGuid = (*wdatait)->m_guid;
						R->getListener()->onSubscriptionMatched(info);
					}
				}
			}
			else
			{
				//logInfo(RTPS_EDP,RTPS_CYAN<<"Valid Matching to writerProxy: "<<(*wdatait)->m_guid<<RTPS_DEF<<endl);
				if(R->matched_writer_remove(*wdatait))
				{
					//MATCHED AND ADDED CORRECTLY:
					if(R->getListener()!=nullptr)
					{
						MatchingInfo info;
						info.status = REMOVED_MATCHING;
						info.remoteEndpointGuid = (*wdatait)->m_guid;
						R->getListener()->onSubscriptionMatched(info);
					}
				}
			}
		}
	}
	return true;
}

bool EDP::pairingWriter(RTPSWriter* W)
{
	const char* const METHOD_NAME = "pairingWriter";
		logInfo(RTPS_EDP,W->getGuid()<<" in topic: \"" << W->getTopic().getTopicName()<<"\"",EPRO_CYAN);
	for(std::vector<RTPSParticipantProxyData*>::const_iterator pit = mp_PDP->RTPSParticipantProxiesBegin();
			pit!=mp_PDP->RTPSParticipantProxiesEnd();++pit)
	{
		for(std::vector<ReaderProxyData*>::iterator rdatait = (*pit)->m_readers.begin();
				rdatait!=(*pit)->m_readers.end();++rdatait)
		{
			if(validMatching(W,*rdatait))
			{
				//std::cout << "VALID MATCHING to " <<(*rdatait)->m_guid<< std::endl;
				logInfo(RTPS_EDP,"Valid Matching to readerProxy: "<<(*rdatait)->m_guid,EPRO_CYAN);
				if(W->matched_reader_add(*rdatait))
				{
					//MATCHED AND ADDED CORRECTLY:
					if(W->getListener()!=nullptr)
					{
						MatchingInfo info;
						info.status = MATCHED_MATCHING;
						info.remoteEndpointGuid = (*rdatait)->m_guid;
						W->getListener()->onPublicationMatched(info);
					}
				}
			}
			else
			{
				//logInfo(RTPS_EDP,RTPS_CYAN<<"Valid Matching to writerProxy: "<<(*wdatait)->m_guid<<RTPS_DEF<<endl);
				if(W->matched_reader_remove(*rdatait))
				{
					//MATCHED AND ADDED CORRECTLY:
					if(W->getListener()!=nullptr)
					{
						MatchingInfo info;
						info.status = REMOVED_MATCHING;
						info.remoteEndpointGuid = (*rdatait)->m_guid;
						W->getListener()->onPublicationMatched(info);
					}
				}
			}
		}
	}
	return true;
}

bool EDP::pairingReaderProxy(ReaderProxyData* rdata)
{
	const char* const METHOD_NAME = "pairingReaderProxy";
	logInfo(RTPS_EDP,rdata->m_guid<<" in topic: \"" << rdata->m_topicName <<"\"",EPRO_CYAN);
	for(std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
			wit!=mp_RTPSParticipant->userWritersListEnd();++wit)
	{
		if(validMatching(*wit,rdata))
		{
			logInfo(RTPS_EDP,"Valid Matching to local writer: "<<(*wit)->getGuid().entityId,EPRO_CYAN);
			if((*wit)->matched_reader_add(rdata))
			{
				//MATCHED AND ADDED CORRECTLY:
				if((*wit)->getListener()!=nullptr)
				{
					MatchingInfo info;
					info.status = MATCHED_MATCHING;
					info.remoteEndpointGuid = rdata->m_guid;
					(*wit)->getListener()->onPublicationMatched(info);
				}
			}
		}
		else
		{
			if((*wit)->matched_reader_remove(rdata))
			{
				//MATCHED AND ADDED CORRECTLY:
				if((*wit)->getListener()!=nullptr)
				{
					MatchingInfo info;
					info.status = REMOVED_MATCHING;
					info.remoteEndpointGuid = rdata->m_guid;
					(*wit)->getListener()->onPublicationMatched(info);
				}
			}
		}
	}
	return true;
}

bool EDP::pairingWriterProxy(WriterProxyData* wdata)
{
	const char* const METHOD_NAME = "pairingWriterProxy";
	logInfo(RTPS_EDP,wdata->m_guid<<" in topic: \"" << wdata->m_topicName <<"\"",EPRO_CYAN);
	for(std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
			rit!=mp_RTPSParticipant->userReadersListEnd();++rit)
	{
		if(validMatching(*rit,wdata))
		{
			logInfo(RTPS_EDP,"Valid Matching to local reader: "<<(*rit)->getGuid().entityId,EPRO_CYAN);
			if((*rit)->matched_writer_add(wdata))
			{
				//MATCHED AND ADDED CORRECTLY:
				if((*rit)->getListener()!=nullptr)
				{
					MatchingInfo info;
					info.status = MATCHED_MATCHING;
					info.remoteEndpointGuid = wdata->m_guid;
					(*rit)->getListener()->onSubscriptionMatched(info);
				}
			}
		}
		else
		{
			if((*rit)->matched_writer_remove(wdata))
			{
				//MATCHED AND ADDED CORRECTLY:
				if((*rit)->getListener()!=nullptr)
				{
					MatchingInfo info;
					info.status = REMOVED_MATCHING;
					info.remoteEndpointGuid = wdata->m_guid;
					(*rit)->getListener()->onSubscriptionMatched(info);
				}
			}
		}
	}
	return true;
}





} /* namespace rtps */
} /* namespace eprosima */


