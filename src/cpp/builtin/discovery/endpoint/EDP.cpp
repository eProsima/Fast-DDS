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
#include "eprosimartps/utils/IPFinder.h"
#include "eprosimartps/utils/StringMatching.h"


#include "eprosimartps/dds/PublisherListener.h"
#include "eprosimartps/dds/SubscriberListener.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

EDP::EDP(PDPSimple* p,ParticipantImpl* part):
													mp_PDP(p),
													mp_participant(part)
{
	// TODO Auto-generated constructor stub

}

EDP::~EDP()
{
	// TODO Auto-generated destructor stub
}

bool EDP::newLocalReaderProxyData(RTPSReader* reader)
{
	pDebugInfo(RTPS_CYAN<<"EDP new LocalReaderProxyData: "<<reader->getGuid().entityId<<RTPS_DEF<<endl);
	ReaderProxyData* rpd = new ReaderProxyData();
	rpd->m_isAlive = true;
	rpd->m_expectsInlineQos = reader->expectsInlineQos();
	rpd->m_guid = reader->getGuid();
	rpd->m_key = rpd->m_guid;
	rpd->m_multicastLocatorList = reader->multicastLocatorList;
	rpd->m_unicastLocatorList = reader->unicastLocatorList;
	rpd->m_participantKey = mp_participant->getGuid();
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
	pairReader(reader);
	pairReaderProxy(rpd);
	return true;
}

bool EDP::updatedLocalReader(RTPSReader* R)
{
	ReaderProxyData* rdata = NULL;
	if(this->mp_PDP->lookupReaderProxyData(R->getGuid(),&rdata))
	{
		rdata->m_qos.setQos(R->getQos(),false);
		rdata->m_expectsInlineQos = R->expectsInlineQos();
		processLocalReaderProxyData(rdata);
		this->updatedReaderProxy(rdata);
		return true;
	}
	return false;
}

bool EDP::updatedLocalWriter(RTPSWriter* W)
{
	WriterProxyData* wdata = NULL;
	if(this->mp_PDP->lookupWriterProxyData(W->getGuid(),&wdata))
	{
		wdata->m_qos.setQos(W->getQos(),false);
		processLocalWriterProxyData(wdata);
		this->updatedWriterProxy(wdata);
		return true;
	}
	return false;
}



bool EDP::newLocalWriterProxyData(RTPSWriter* writer)
{
	pDebugInfo(RTPS_CYAN<<"EDP new LocalWriterProxyData: "<<writer->getGuid().entityId<<RTPS_DEF<<endl);
	WriterProxyData* wpd = new WriterProxyData();
	wpd->m_isAlive = true;
	wpd->m_guid = writer->getGuid();
	wpd->m_key = wpd->m_guid;
	wpd->m_multicastLocatorList = writer->multicastLocatorList;
	wpd->m_unicastLocatorList = writer->unicastLocatorList;
	wpd->m_participantKey = mp_participant->getGuid();
	wpd->m_topicName = writer->getTopic().getTopicName();
	wpd->m_typeName = writer->getTopic().getTopicDataType();
	wpd->m_topicKind = writer->getTopic().getTopicKind();
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
	pairWriterProxy(wpd);
	pairWriter(writer);
	return true;
}





void EDP::pairReaderProxy(ReaderProxyData* rdata)
{
	pDebugInfo(RTPS_CYAN<<"EDP pairing readerProxy: "<<rdata->m_guid<< " in topic: " << rdata->m_topicName<<RTPS_DEF<<endl);
	for(std::vector<RTPSWriter*>::iterator wit = mp_participant->userWritersListBegin();
			wit!=mp_participant->userWritersListEnd();++wit)
	{
		if(validMatching(*wit,rdata))
		{
			pDebugInfo("Valid Matching to local writer: "<<(*wit)->getGuid().entityId<<endl);
			if((*wit)->matched_reader_add(rdata))
			{
				//MATCHED AND ADDED CORRECTLY:
				if((*wit)->getListener()!=NULL)
				{
					MatchingInfo info;
					info.status = MATCHED_MATCHING;
					info.remoteEndpointGuid = rdata->m_guid;
					(*wit)->getListener()->onPublicationMatched(info);
				}
			}
		}
	}
}

void EDP::pairReader(RTPSReader* R)
{
	pDebugInfo(RTPS_CYAN<<"EDP pairing Local Reader: "<<R->getGuid()<<" in topic: " << R->getTopic().getTopicName()<<RTPS_DEF<<endl);
	for(std::vector<ParticipantProxyData*>::const_iterator pit = mp_PDP->participantProxiesBegin();
			pit!=mp_PDP->participantProxiesEnd();++pit)
	{
		for(std::vector<WriterProxyData*>::iterator wdatait = (*pit)->m_writers.begin();
				wdatait!=(*pit)->m_writers.end();++wdatait)
		{
			if(validMatching(R,*wdatait))
			{
				pDebugInfo("Valid Matching to writerProxy: "<<(*wdatait)->m_guid<<endl);
				if(R->matched_writer_add(*wdatait))
				{
					//MATCHED AND ADDED CORRECTLY:
					if(R->getListener()!=NULL)
					{
						MatchingInfo info;
						info.status = MATCHED_MATCHING;
						info.remoteEndpointGuid = (*wdatait)->m_guid;
						R->getListener()->onSubscriptionMatched(info);
					}
				}
			}
		}
	}
}


void EDP::pairWriterProxy(WriterProxyData* wdata)
{
	pDebugInfo(RTPS_CYAN<<"EDP pairing writerPoxy: "<<wdata->m_guid<<" in topic: " << wdata->m_topicName<<RTPS_DEF<<endl);
	for(std::vector<RTPSReader*>::iterator rit = mp_participant->userReadersListBegin();
			rit!=mp_participant->userReadersListEnd();++rit)
	{
		if(validMatching(*rit,wdata))
		{
			pDebugInfo("Valid Matching to local Reader "<<(*rit)->getGuid().entityId<<endl);
			if((*rit)->matched_writer_add(wdata))
			{
				//MATCHED AND ADDED CORRECTLY:
				if((*rit)->getListener()!=NULL)
				{
					MatchingInfo info;
					info.status = MATCHED_MATCHING;
					info.remoteEndpointGuid = wdata->m_guid;
					if((*rit)->getListener()!=NULL)
						(*rit)->getListener()->onSubscriptionMatched(info);
				}
			}
		}
	}
}


void EDP::pairWriter(RTPSWriter* W)
{
	pDebugInfo(RTPS_CYAN<<"EDP pairing local Writer: "<<W->getGuid()<< " in topic: " << W->getTopic().getTopicName()<<RTPS_DEF<<endl);
	for(std::vector<ParticipantProxyData*>::const_iterator pit = mp_PDP->participantProxiesBegin();
			pit!=mp_PDP->participantProxiesEnd();++pit)
	{
		for(std::vector<ReaderProxyData*>::iterator rdatait = (*pit)->m_readers.begin();
				rdatait!=(*pit)->m_readers.end();++rdatait)
		{
			if(validMatching(W,*rdatait))
			{
				pDebugInfo("Valid Matching to ReaderProxy "<<(*rdatait)->m_guid<<endl);
				if(W->matched_reader_add(*rdatait))
				{
					//MATCHED AND ADDED CORRECTLY:
					if(W->getListener()!=NULL)
					{
						MatchingInfo info;
						info.status = MATCHED_MATCHING;
						info.remoteEndpointGuid = (*rdatait)->m_guid;
						W->getListener()->onPublicationMatched(info);
					}
				}
			}
		}
	}
}


bool EDP::removeWriterProxy(const GUID_t& writer)
{
	pDebugInfo(RTPS_CYAN<<"EDP removing writer: "<<writer<<RTPS_DEF<<endl);
	WriterProxyData* wdata = NULL;
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
	pDebugInfo(RTPS_CYAN<<"EDP removing reader: "<<reader<<RTPS_DEF<<endl);
	ReaderProxyData* rdata = NULL;
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
	pDebugInfo(RTPS_CYAN<<"EDP unpairing writer: "<<wdata->m_guid<<RTPS_DEF<<endl);
	for(std::vector<RTPSReader*>::iterator rit = mp_participant->userReadersListBegin();
			rit!=mp_participant->userReadersListEnd();++rit)
	{
		if((*rit)->matched_writer_remove(wdata))
		{
			//MATCHED AND ADDED CORRECTLY:
			if((*rit)->getListener()!=NULL)
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
	pDebugInfo(RTPS_CYAN<<"EDP unpairing reader: "<<rdata->m_guid<<RTPS_DEF<<endl);
	for(std::vector<RTPSWriter*>::iterator wit = mp_participant->userWritersListBegin();
			wit!=mp_participant->userWritersListEnd();++wit)
	{
		if((*wit)->matched_reader_remove(rdata))
		{
			//MATCHED AND ADDED CORRECTLY:
			if((*wit)->getListener()!=NULL)
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
	if(W->getTopic().getTopicName() != rdata->m_topicName)
		return false;
	if(W->getTopic().getTopicDataType() != rdata->m_typeName)
		return false;
	if(W->getTopic().getTopicKind() != rdata->m_topicKind)
	{
		pWarning("INCOMPATIBLE QOS:Remote Reader "<<rdata->m_guid << " is publishing in topic " << rdata->m_topicName << "(keyed:"<<rdata->m_topicKind<<
				"), local writer publishes as keyed: "<<W->getTopic().getTopicKind()<<endl;)
																																				return false;
	}
	if(!rdata->m_isAlive) //Matching
	{
		pWarning("ReaderProxyData object is NOT alive"<<endl);
		return false;
	}
	if(W->getStateType() == STATELESS && rdata->m_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS) //Means our writer is BE but the reader wants RE
	{
		pWarning("INCOMPATIBLE QOS:Remote Reader is Reliable and local writer is BE "<<endl;);
		return false;
	}
	if(W->getQos().m_durability.kind == VOLATILE_DURABILITY_QOS && rdata->m_qos.m_durability.kind == TRANSIENT_LOCAL_DURABILITY_QOS)
	{
		pWarning("INCOMPATIBLE QOS:RemoteReader has TRANSIENT_LOCAL DURABILITY and we offer VOLATILE"<<endl;);
		return false;
	}
	if(W->getQos().m_ownership.kind != rdata->m_qos.m_ownership.kind)
	{
		pWarning("INCOMPATIBLE QOS:Different Ownership Kind"<<endl;);
		return false;
	}
	//Partition check:
	bool matched = false;
	if(W->getQos().m_partition.names.empty() && rdata->m_qos.m_partition.names.empty())
	{
		matched = true;
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

	return matched;
}
bool EDP::validMatching(RTPSReader* R,WriterProxyData* wdata)
{
	if(R->getTopic().getTopicName() != wdata->m_topicName)
		return false;
	if(R->getTopic().getTopicDataType() != wdata->m_typeName)
		return false;
	if(R->getTopic().getTopicKind() != wdata->m_topicKind)
	{
		pWarning("INCOMPATIBLE QOS:Remote Writer "<<wdata->m_guid << " is publishing in topic " << wdata->m_topicName << "(keyed:"<<wdata->m_topicKind<<
				"), local reader subscribes as keyed: "<<R->getTopic().getTopicKind()<<endl;)
																																							return false;
	}
	if(!wdata->m_isAlive) //Matching
	{
		pWarning("WriterProxyData " << wdata->m_guid << " is NOT alive"<<endl);
		return false;
	}
	if(R->getStateType() == STATEFUL && wdata->m_qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS) //Means our reader is reliable but hte writer is not
	{
		pWarning("INCOMPATIBLE QOS:Remote Writer is Best Effort and local reader is RELIABLE "<<endl;);
		return false;
	}
	if(R->getQos().m_durability.kind == TRANSIENT_LOCAL_DURABILITY_QOS && wdata->m_qos.m_durability.kind == VOLATILE_DURABILITY_QOS)
	{
		pWarning("INCOMPATIBLE QOS:RemoteWriter has VOLATILE DURABILITY and we want TRANSIENT_LOCAL"<<endl;);
		return false;
	}
	if(R->getQos().m_ownership.kind != wdata->m_qos.m_ownership.kind)
	{
		pWarning("INCOMPATIBLE QOS:Different Ownership Kind"<<endl;);
		return false;
	}
	//Partition check:
	bool matched = false;
	if(R->getQos().m_partition.names.empty() && wdata->m_qos.m_partition.names.empty())
	{
		matched = true;
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
	return matched;
}


bool EDP::updatedReaderProxy(ReaderProxyData* rdata)
{
	pDebugInfo(RTPS_CYAN<<"EDP updated readerProxy: "<<rdata->m_guid<< " in topic: " << rdata->m_topicName<<RTPS_DEF<<endl);
	for(std::vector<RTPSWriter*>::iterator wit = mp_participant->userWritersListBegin();
			wit!=mp_participant->userWritersListEnd();++wit)
	{
		if(validMatching(*wit,rdata))
		{
			pDebugInfo("Valid Matching"<<endl);
			if((*wit)->matched_reader_add(rdata))
			{
				//MATCHED AND ADDED CORRECTLY:
				if((*wit)->getListener()!=NULL)
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
				if((*wit)->getListener()!=NULL)
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

bool EDP::updatedWriterProxy(WriterProxyData* wdata)
{
	pDebugInfo(RTPS_CYAN<<"EDP updated readerProxy: "<<wdata->m_guid<< " in topic: " << wdata->m_topicName<<RTPS_DEF<<endl);
	for(std::vector<RTPSReader*>::iterator rit = mp_participant->userReadersListBegin();
			rit!=mp_participant->userReadersListEnd();++rit)
	{
		if(validMatching(*rit,wdata))
		{
			pDebugInfo("Valid Matching"<<endl);
			if((*rit)->matched_writer_add(wdata))
			{
				//MATCHED AND ADDED CORRECTLY:
				if((*rit)->getListener()!=NULL)
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
				if((*rit)->getListener()!=NULL)
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


