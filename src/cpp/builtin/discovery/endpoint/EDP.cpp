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

#include "eprosimartps/writer/ReaderProxyData.h"
#include "eprosimartps/utils/RTPSLog.h"

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
	ReaderProxyData* rpd = new ReaderProxyData();
	rpd->m_isAlive = true;
	rpd->m_expectsInlineQos = reader->expectsInlineQos();
	rpd->m_guid = reader->getGuid();
	rpd->m_key = rpd->m_guid;
	rpd->m_multicastLocatorList = reader->multicastLocatorList;
	rpd->m_unicastLocatorList = reader->unicastLocatorList;
	rpd->m_participantKey = mp_PDP->mp_participant->getGuid();
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
	pairReaderProxy(rpd);
	return true;
}

bool EDP::newLocalWriterProxyData(RTPSWriter* writer)
{
	WriterProxyData* wpd = new WriterProxyData();
	wpd->m_isAlive = true;
	wpd->m_guid = writer->getGuid();
	wpd->m_key = wpd->m_guid;
	wpd->m_multicastLocatorList = writer->multicastLocatorList;
	wpd->m_unicastLocatorList = writer->unicastLocatorList;
	wpd->m_participantKey = mp_PDP->mp_participant->getGuid();
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
	return true;
}


void EDP::pairReaderProxy(ReaderProxyData* rdata)
{
	for(std::vector<RTPSWriter*>::iterator wit = mp_participant->userWritersListBegin();
			wit!=mp_participant->userWritersListEnd();++wit)
	{
		if(validMatching(*wit,rdata))
		{
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


void EDP::pairWriterProxy(WriterProxyData* wdata)
{
	for(std::vector<RTPSReader*>::iterator rit = mp_participant->userReadersListBegin();
			rit!=mp_participant->userReadersListEnd();++rit)
	{
		if(validMatching(*rit,wdata))
		{
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
	}
}

bool EDP::unpairWriterProxy(WriterProxyData* wdata)
{
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
	for(std::vector<RTPSWriter*>::iterator rit = mp_participant->userReadersListBegin();
			rit!=mp_participant->userReadersListEnd();++rit)
	{
		if((*rit)->matched_writer_remove(rdata))
		{
			//MATCHED AND ADDED CORRECTLY:
			if((*rit)->getListener()!=NULL)
			{
				MatchingInfo info;
				info.status = REMOVED_MATCHING;
				info.remoteEndpointGuid = rdata->m_guid;
				(*rit)->getListener()->onPublicationMatched(info);
			}
		}
	}
	return true;
}




bool validMatching(RTPSWriter* W,ReaderProxyData* rdata)
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
		return false;
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
bool validMatching(RTPSReader* R,WriterProxyData* wdata)
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
		return false;
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







} /* namespace rtps */
} /* namespace eprosima */
