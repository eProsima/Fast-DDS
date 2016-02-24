/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDP.cpp
 *
 */

#include <fastrtps/rtps/builtin/discovery/endpoint/EDP.h>

#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>

#include "../../../participant/RTPSParticipantImpl.h"


#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/writer/WriterListener.h>
#include <fastrtps/rtps/reader/ReaderListener.h>

#include <fastrtps/rtps/builtin/data/WriterProxyData.h>
#include <fastrtps/rtps/builtin/data/ReaderProxyData.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>

#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/rtps/common/MatchingInfo.h>

#include <fastrtps/utils/StringMatching.h>
#include <fastrtps/utils/RTPSLog.h>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "EDP";

EDP::EDP(PDPSimple* p,RTPSParticipantImpl* part):	mp_PDP(p),
		mp_RTPSParticipant(part)
{
	// TODO Auto-generated constructor stub

}

EDP::~EDP()
{
	// TODO Auto-generated destructor stub
}

bool EDP::newLocalReaderProxyData(RTPSReader* reader,TopicAttributes& att, ReaderQos& rqos)
{
	const char* const METHOD_NAME = "newLocalReaderProxyData";
	logInfo(RTPS_EDP,"Adding " << reader->getGuid().entityId << " in topic "<<att.topicName,C_CYAN);
	ReaderProxyData* rpd = new ReaderProxyData();
	rpd->m_isAlive = true;
	rpd->m_expectsInlineQos = reader->expectsInlineQos();
	rpd->m_guid = reader->getGuid();
	rpd->m_key = rpd->m_guid;
	rpd->m_multicastLocatorList = reader->getAttributes()->multicastLocatorList;
	rpd->m_unicastLocatorList = reader->getAttributes()->unicastLocatorList;
	rpd->m_RTPSParticipantKey = mp_RTPSParticipant->getGuid();
	rpd->m_topicName = att.getTopicName();
	rpd->m_typeName = att.getTopicDataType();
	rpd->m_topicKind = att.getTopicKind();
	rpd->m_qos = rqos;
	rpd->m_userDefinedId = reader->getAttributes()->getUserDefinedID();
	reader->m_acceptMessagesFromUnkownWriters = false;
	//ADD IT TO THE LIST OF READERPROXYDATA
    ParticipantProxyData* pdata = nullptr;
	if(!this->mp_PDP->addReaderProxyData(rpd, false, nullptr, &pdata))
	{
		delete(rpd);
		return false;
	}
	//DO SOME PROCESSING DEPENDING ON THE IMPLEMENTATION (SIMPLE OR STATIC)
	processLocalReaderProxyData(rpd);
	//PAIRING
    pairingReaderProxy(pdata, rpd);
	pairingReader(reader);
	return true;
}

bool EDP::newLocalWriterProxyData(RTPSWriter* writer,TopicAttributes& att, WriterQos& wqos)
{
	const char* const METHOD_NAME = "newLocalWriterProxyData";
	logInfo(RTPS_EDP,"Adding " << writer->getGuid().entityId << " in topic "<<att.topicName,C_CYAN);
	WriterProxyData* wpd = new WriterProxyData();
	wpd->m_isAlive = true;
	wpd->m_guid = writer->getGuid();
	wpd->m_key = wpd->m_guid;
	wpd->m_multicastLocatorList = writer->getAttributes()->multicastLocatorList;
	wpd->m_unicastLocatorList = writer->getAttributes()->unicastLocatorList;
	wpd->m_RTPSParticipantKey = mp_RTPSParticipant->getGuid();
	wpd->m_topicName = att.getTopicName();
	wpd->m_typeName = att.getTopicDataType();
	wpd->m_topicKind = att.getTopicKind();
	wpd->m_typeMaxSerialized = writer->getTypeMaxSerialized();
	wpd->m_qos = wqos;
	wpd->m_userDefinedId = writer->getAttributes()->getUserDefinedID();
	//ADD IT TO THE LIST OF READERPROXYDATA
    ParticipantProxyData* pdata = nullptr;
    if(!this->mp_PDP->addWriterProxyData(wpd, false, nullptr, &pdata))
	{
		delete(wpd);
		return false;
	}
	//DO SOME PROCESSING DEPENDING ON THE IMPLEMENTATION (SIMPLE OR STATIC)
	processLocalWriterProxyData(wpd);
	//PAIRING
    pairingWriterProxy(pdata, wpd);
	pairingWriter(writer);
	return true;
}

bool EDP::updatedLocalReader(RTPSReader* R,ReaderQos& rqos)
{
    ParticipantProxyData* pdata = nullptr;
	ReaderProxyData* rdata = nullptr;
	if(this->mp_PDP->lookupReaderProxyData(R->getGuid(),&rdata, &pdata))
	{
		rdata->m_qos.setQos(rqos,false);
		rdata->m_expectsInlineQos = R->expectsInlineQos();
		processLocalReaderProxyData(rdata);
		//this->updatedReaderProxy(rdata);
		pairingReaderProxy(pdata, rdata);
		pairingReader(R);
		return true;
	}
	return false;
}

bool EDP::updatedLocalWriter(RTPSWriter* W,WriterQos& wqos)
{
    ParticipantProxyData* pdata = nullptr;
	WriterProxyData* wdata = nullptr;
	if(this->mp_PDP->lookupWriterProxyData(W->getGuid(),&wdata, &pdata))
	{
		wdata->m_qos.setQos(wqos,false);
		processLocalWriterProxyData(wdata);
		//this->updatedWriterProxy(wdata);
		pairingWriterProxy(pdata, wdata);
		pairingWriter(W);
		return true;
	}
	return false;
}


bool EDP::removeWriterProxy(const GUID_t& writer)
{
	const char* const METHOD_NAME = "removeWriterProxy";
	logInfo(RTPS_EDP,writer,C_CYAN);
    ParticipantProxyData* pdata = nullptr;
	WriterProxyData* wdata = nullptr;
    // Block because other thread can be removing the participant.
    boost::lock_guard<boost::recursive_mutex> pguard(*mp_PDP->getMutex());
	if(this->mp_PDP->lookupWriterProxyData(writer,&wdata, &pdata))
	{
		logInfo(RTPS_EDP," in topic: "<<wdata->m_topicName,C_CYAN);
		unpairWriterProxy(pdata, wdata);
		this->mp_PDP->removeWriterProxyData(pdata, wdata);
		return true;
	}
	return false;
}

bool EDP::removeReaderProxy(const GUID_t& reader)
{
	const char* const METHOD_NAME = "removeReaderProxy";
	logInfo(RTPS_EDP,reader,C_CYAN);
    ParticipantProxyData* pdata = nullptr;
	ReaderProxyData* rdata = nullptr;
    // Block because other thread can be removing the participant.
    boost::lock_guard<boost::recursive_mutex> pguard(*mp_PDP->getMutex());
	if(this->mp_PDP->lookupReaderProxyData(reader,&rdata, &pdata))
	{
		logInfo(RTPS_EDP," in topic: "<<rdata->m_topicName,C_CYAN);
		unpairReaderProxy(pdata, rdata);
		this->mp_PDP->removeReaderProxyData(pdata, rdata);
		return true;
	}
	return false;
}

bool EDP::unpairWriterProxy(ParticipantProxyData *pdata, WriterProxyData* wdata)
{
	const char* const METHOD_NAME = "unpairWriterProxy";
	logInfo(RTPS_EDP,wdata->m_guid << " in topic: "<< wdata->m_topicName,C_CYAN);
	boost::lock_guard<boost::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
	for(std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
			rit!=mp_RTPSParticipant->userReadersListEnd();++rit)
	{
		RemoteWriterAttributes watt;
        boost::unique_lock<boost::recursive_mutex> plock(*pdata->mp_mutex);
		watt.guid = wdata->m_guid;
		if((*rit)->matched_writer_remove(watt))
		{
			//MATCHED AND ADDED CORRECTLY:
			if((*rit)->getListener()!=nullptr)
			{
				MatchingInfo info;
				info.status = REMOVED_MATCHING;
				info.remoteEndpointGuid = wdata->m_guid;
				(*rit)->getListener()->onReaderMatched((*rit),info);
			}
		}
	}
	return true;
}

bool EDP::unpairReaderProxy(ParticipantProxyData *pdata, ReaderProxyData* rdata)
{
	const char* const METHOD_NAME = "unpairReaderProxy";
	logInfo(RTPS_EDP,rdata->m_guid << " in topic: "<< rdata->m_topicName,C_CYAN);
	boost::lock_guard<boost::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
	for(std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
			wit!=mp_RTPSParticipant->userWritersListEnd();++wit)
	{
		RemoteReaderAttributes ratt;
        boost::unique_lock<boost::recursive_mutex> plock(*pdata->mp_mutex);
		ratt.guid = rdata->m_guid;
		if((*wit)->matched_reader_remove(ratt))
		{
			//MATCHED AND ADDED CORRECTLY:
			if((*wit)->getListener()!=nullptr)
			{
				MatchingInfo info;
				info.status = REMOVED_MATCHING;
				info.remoteEndpointGuid = rdata->m_guid;
				(*wit)->getListener()->onWriterMatched((*wit),info);
			}
		}
	}
	return true;
}


bool EDP::validMatching(WriterProxyData* wdata,ReaderProxyData* rdata)
{
	const char* const METHOD_NAME = "validMatching(W2RP)";

	if(wdata->m_topicName != rdata->m_topicName)
		return false;
	if(wdata->m_typeName != rdata->m_typeName)
		return false;
	if(wdata->m_topicKind != rdata->m_topicKind)
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS:Remote Reader "<<rdata->m_guid << " is publishing in topic "
				<< rdata->m_topicName << "(keyed:"<<rdata->m_topicKind<<
				"), local writer publishes as keyed: "<<wdata->m_topicKind,C_CYAN)
																																																														return false;
	}
	if(!rdata->m_isAlive) //Matching
	{
		logWarning(RTPS_EDP,"ReaderProxyData object is NOT alive",C_CYAN);
		return false;
	}
	if( wdata->m_qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS
			&& rdata->m_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS) //Means our writer is BE but the reader wants RE
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< rdata->m_topicName<<"):Remote Reader "
				<<rdata->m_guid << " is Reliable and local writer is BE ",C_CYAN);
		return false;
	}
	if(wdata->m_qos.m_durability.kind == VOLATILE_DURABILITY_QOS
			&& rdata->m_qos.m_durability.kind == TRANSIENT_LOCAL_DURABILITY_QOS)
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< rdata->m_topicName<<"):RemoteReader "
				<<rdata->m_guid << " has TRANSIENT_LOCAL DURABILITY and we offer VOLATILE",C_CYAN);
		return false;
	}
	if(wdata->m_qos.m_ownership.kind != rdata->m_qos.m_ownership.kind)
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< rdata->m_topicName<<"):Remote reader "
				<<rdata->m_guid << " has different Ownership Kind",C_CYAN);
		return false;
	}
	//Partition check:
	bool matched = false;
	if(wdata->m_qos.m_partition.names.empty() && rdata->m_qos.m_partition.names.empty())
	{
		matched = true;
	}
	else if(wdata->m_qos.m_partition.names.empty() && rdata->m_qos.m_partition.names.size()>0)
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
	else if(wdata->m_qos.m_partition.names.size()>0 && rdata->m_qos.m_partition.names.empty() )
	{
		for(std::vector<std::string>::const_iterator wnameit = wdata->m_qos.m_partition.names.begin();
				wnameit !=  wdata->m_qos.m_partition.names.end();++wnameit)
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
		for(std::vector<std::string>::const_iterator wnameit = wdata->m_qos.m_partition.names.begin();
				wnameit !=  wdata->m_qos.m_partition.names.end();++wnameit)
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
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< rdata->m_topicName<<"): Different Partitions",C_CYAN);
	return matched;
}
bool EDP::validMatching(ReaderProxyData* rdata,WriterProxyData* wdata)
{
	const char* const METHOD_NAME = "validMatching(R2WP)";

	if(rdata->m_topicName != wdata->m_topicName)
		return false;
	if( rdata->m_typeName != wdata->m_typeName)
		return false;
	if(rdata->m_topicKind != wdata->m_topicKind)
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS:Remote Writer "<<wdata->m_guid << " is publishing in topic " << wdata->m_topicName << "(keyed:"<<wdata->m_topicKind<<
				"), local reader subscribes as keyed: "<<rdata->m_topicKind,C_CYAN)
																																																																	return false;
	}
	if(!wdata->m_isAlive) //Matching
	{
		logWarning(RTPS_EDP,"WriterProxyData " << wdata->m_guid << " is NOT alive",C_CYAN);
		return false;
	}
	if(rdata->m_qos.m_reliability.kind == RELIABLE_RELIABILITY_QOS
			&& wdata->m_qos.m_reliability.kind == BEST_EFFORT_RELIABILITY_QOS) //Means our reader is reliable but hte writer is not
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< wdata->m_topicName<<"): Remote Writer "<<wdata->m_guid << " is Best Effort and local reader is RELIABLE "<<endl;);
		return false;
	}
	if(rdata->m_qos.m_durability.kind == TRANSIENT_LOCAL_DURABILITY_QOS
			&& wdata->m_qos.m_durability.kind == VOLATILE_DURABILITY_QOS)
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< wdata->m_topicName<<"):RemoteWriter "<<wdata->m_guid << " has VOLATILE DURABILITY and we want TRANSIENT_LOCAL"<<endl;);
		return false;
	}
	if(rdata->m_qos.m_ownership.kind != wdata->m_qos.m_ownership.kind)
	{
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< wdata->m_topicName<<"):Remote Writer "<<wdata->m_guid << " has different Ownership Kind"<<endl;);
		return false;
	}
	//Partition check:
	bool matched = false;
	if(rdata->m_qos.m_partition.names.empty() && wdata->m_qos.m_partition.names.empty())
	{
		matched = true;
	}
	else if(rdata->m_qos.m_partition.names.empty() && wdata->m_qos.m_partition.names.size()>0)
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
	else if(rdata->m_qos.m_partition.names.size()>0 && wdata->m_qos.m_partition.names.empty() )
	{
		for(std::vector<std::string>::const_iterator wnameit = rdata->m_qos.m_partition.names.begin();
				wnameit !=  rdata->m_qos.m_partition.names.end();++wnameit)
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
		for(std::vector<std::string>::const_iterator wnameit = rdata->m_qos.m_partition.names.begin();
				wnameit !=  rdata->m_qos.m_partition.names.end();++wnameit)
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
		logWarning(RTPS_EDP,"INCOMPATIBLE QOS (topic: "<< wdata->m_topicName<<"): Different Partitions",C_CYAN);
	return matched;

}

//TODO Estas cuatro funciones comparten codigo comun (2 a 2) y se podrían seguramente combinar.

bool EDP::pairingReader(RTPSReader* R)
{
    const char* const METHOD_NAME = "pairingReader";
    ParticipantProxyData* pdata = nullptr;
	ReaderProxyData* rdata = nullptr;
	if(this->mp_PDP->lookupReaderProxyData(R->getGuid(),&rdata, &pdata))
	{
		logInfo(RTPS_EDP,R->getGuid()<<" in topic: \"" << rdata->m_topicName<<"\"",C_CYAN);
		boost::lock_guard<boost::recursive_mutex> pguard(*mp_PDP->getMutex());
		for(std::vector<ParticipantProxyData*>::const_iterator pit = mp_PDP->ParticipantProxiesBegin();
				pit!=mp_PDP->ParticipantProxiesEnd();++pit)
		{
			boost::lock_guard<boost::recursive_mutex> guard(*(*pit)->mp_mutex);
			for(std::vector<WriterProxyData*>::iterator wdatait = (*pit)->m_writers.begin();
					wdatait!=(*pit)->m_writers.end();++wdatait)
			{
                pdata->mp_mutex->lock();
                bool valid = validMatching(rdata, *wdatait);
                pdata->mp_mutex->unlock();

				if(valid)
				{
					logInfo(RTPS_EDP,"Valid Matching to writerProxy: "<<(*wdatait)->m_guid,C_CYAN);
					if(R->matched_writer_add((*wdatait)->toRemoteWriterAttributes()))
					{
						//MATCHED AND ADDED CORRECTLY:
						if(R->getListener()!=nullptr)
						{
							MatchingInfo info;
							info.status = MATCHED_MATCHING;
							info.remoteEndpointGuid = (*wdatait)->m_guid;
							R->getListener()->onReaderMatched(R,info);
						}
					}
				}
				else
				{
					//logInfo(RTPS_EDP,RTPS_CYAN<<"Valid Matching to writerProxy: "<<(*wdatait)->m_guid<<RTPS_DEF<<endl);
					if(R->matched_writer_is_matched((*wdatait)->toRemoteWriterAttributes())
							&& R->matched_writer_remove((*wdatait)->toRemoteWriterAttributes()))
					{
						//MATCHED AND ADDED CORRECTLY:
						if(R->getListener()!=nullptr)
						{
							MatchingInfo info;
							info.status = REMOVED_MATCHING;
							info.remoteEndpointGuid = (*wdatait)->m_guid;
							R->getListener()->onReaderMatched(R,info);
						}
					}
				}
			}
		}
		return true;
	}
	return false;
}

//TODO Añadir WriterProxyData como argumento con nullptr como valor por defecto.
bool EDP::pairingWriter(RTPSWriter* W)
{
	const char* const METHOD_NAME = "pairingWriter";
    ParticipantProxyData* pdata = nullptr;
	WriterProxyData* wdata = nullptr;
	if(this->mp_PDP->lookupWriterProxyData(W->getGuid(),&wdata, &pdata))
	{
		logInfo(RTPS_EDP,W->getGuid()<<" in topic: \"" << wdata->m_topicName<<"\"",C_CYAN);
		boost::lock_guard<boost::recursive_mutex> pguard(*mp_PDP->getMutex());
		for(std::vector<ParticipantProxyData*>::const_iterator pit = mp_PDP->ParticipantProxiesBegin();
				pit!=mp_PDP->ParticipantProxiesEnd();++pit)
		{
			boost::lock_guard<boost::recursive_mutex> guard(*(*pit)->mp_mutex);
			for(std::vector<ReaderProxyData*>::iterator rdatait = (*pit)->m_readers.begin();
					rdatait!=(*pit)->m_readers.end(); ++rdatait)
			{
                pdata->mp_mutex->lock();
                bool valid = validMatching(wdata, *rdatait);
                pdata->mp_mutex->unlock();

				if(valid)
				{
					//std::cout << "VALID MATCHING to " <<(*rdatait)->m_guid<< std::endl;
					logInfo(RTPS_EDP,"Valid Matching to readerProxy: "<<(*rdatait)->m_guid,C_CYAN);
					if(W->matched_reader_add((*rdatait)->toRemoteReaderAttributes()))
					{
						//MATCHED AND ADDED CORRECTLY:
						if(W->getListener()!=nullptr)
						{
							MatchingInfo info;
							info.status = MATCHED_MATCHING;
							info.remoteEndpointGuid = (*rdatait)->m_guid;
							W->getListener()->onWriterMatched(W,info);
						}
					}
				}
				else
				{
					//logInfo(RTPS_EDP,RTPS_CYAN<<"Valid Matching to writerProxy: "<<(*wdatait)->m_guid<<RTPS_DEF<<endl);
					if(W->matched_reader_is_matched((*rdatait)->toRemoteReaderAttributes()) &&
							W->matched_reader_remove((*rdatait)->toRemoteReaderAttributes()))
					{
						//MATCHED AND ADDED CORRECTLY:
						if(W->getListener()!=nullptr)
						{
							MatchingInfo info;
							info.status = REMOVED_MATCHING;
							info.remoteEndpointGuid = (*rdatait)->m_guid;
							W->getListener()->onWriterMatched(W,info);
						}
					}
				}
			}
		}
		return true;
	}
	return false;
}

bool EDP::pairingReaderProxy(ParticipantProxyData* pdata, ReaderProxyData* rdata)
{
	const char* const METHOD_NAME = "pairingReaderProxy";
	logInfo(RTPS_EDP,rdata->m_guid<<" in topic: \"" << rdata->m_topicName <<"\"",C_CYAN);
	boost::lock_guard<boost::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
	for(std::vector<RTPSWriter*>::iterator wit = mp_RTPSParticipant->userWritersListBegin();
			wit!=mp_RTPSParticipant->userWritersListEnd();++wit)
	{
        GUID_t writerGUID;
		boost::unique_lock<boost::recursive_mutex> lock(*(*wit)->getMutex());
        writerGUID = (*wit)->getGuid();
        lock.unlock();
        ParticipantProxyData* wpdata = nullptr;
		WriterProxyData* wdata = nullptr;
		if(mp_PDP->lookupWriterProxyData(writerGUID,&wdata, &wpdata))
		{
            boost::unique_lock<boost::recursive_mutex> plock(*pdata->mp_mutex);

            wpdata->mp_mutex->lock();
            bool valid = validMatching(wdata, rdata);
            wpdata->mp_mutex->unlock();

			if(valid)
			{
                logInfo(RTPS_EDP, "Valid Matching to local writer: " << writerGUID.entityId, C_CYAN);
				if((*wit)->matched_reader_add(rdata->toRemoteReaderAttributes()))
				{
					//MATCHED AND ADDED CORRECTLY:
					if((*wit)->getListener()!=nullptr)
					{
						MatchingInfo info;
						info.status = MATCHED_MATCHING;
						info.remoteEndpointGuid = rdata->m_guid;
						(*wit)->getListener()->onWriterMatched((*wit),info);
					}
				}
			}
			else
			{
				if((*wit)->matched_reader_is_matched(rdata->toRemoteReaderAttributes())
						&& (*wit)->matched_reader_remove(rdata->toRemoteReaderAttributes()))
				{
					//MATCHED AND ADDED CORRECTLY:
					if((*wit)->getListener()!=nullptr)
					{
						MatchingInfo info;
						info.status = REMOVED_MATCHING;
						info.remoteEndpointGuid = rdata->m_guid;
						(*wit)->getListener()->onWriterMatched((*wit),info);
					}
				}
			}
		}
	}
	return true;
}

bool EDP::pairingWriterProxy(ParticipantProxyData *pdata, WriterProxyData* wdata)
{
	const char* const METHOD_NAME = "pairingWriterProxy";
	logInfo(RTPS_EDP,wdata->m_guid<<" in topic: \"" << wdata->m_topicName <<"\"",C_CYAN);
	boost::lock_guard<boost::recursive_mutex> guard(*mp_RTPSParticipant->getParticipantMutex());
	for(std::vector<RTPSReader*>::iterator rit = mp_RTPSParticipant->userReadersListBegin();
			rit!=mp_RTPSParticipant->userReadersListEnd();++rit)
	{
        GUID_t readerGUID;
		boost::unique_lock<boost::recursive_mutex> lock(*(*rit)->getMutex());
        readerGUID = (*rit)->getGuid();
        lock.unlock();
        ParticipantProxyData* rpdata = nullptr;
		ReaderProxyData* rdata = nullptr;
        if(mp_PDP->lookupReaderProxyData(readerGUID, &rdata, &rpdata))
		{
            boost::unique_lock<boost::recursive_mutex> plock(*pdata->mp_mutex);

            rpdata->mp_mutex->lock();
            bool valid = validMatching(rdata, wdata);
            rpdata->mp_mutex->unlock();

			if(valid)
			{
                logInfo(RTPS_EDP, "Valid Matching to local reader: " << readerGUID.entityId, C_CYAN);
				if((*rit)->matched_writer_add(wdata->toRemoteWriterAttributes()))
				{
					//MATCHED AND ADDED CORRECTLY:
					if((*rit)->getListener()!=nullptr)
					{
						MatchingInfo info;
						info.status = MATCHED_MATCHING;
						info.remoteEndpointGuid = wdata->m_guid;
						(*rit)->getListener()->onReaderMatched((*rit),info);
					}
				}
			}
			else
			{
				if((*rit)->matched_writer_is_matched(wdata->toRemoteWriterAttributes())
						&& (*rit)->matched_writer_remove(wdata->toRemoteWriterAttributes()))
				{
					//MATCHED AND ADDED CORRECTLY:
					if((*rit)->getListener()!=nullptr)
					{
						MatchingInfo info;
						info.status = REMOVED_MATCHING;
						info.remoteEndpointGuid = wdata->m_guid;
						(*rit)->getListener()->onReaderMatched((*rit),info);
					}
				}
			}
		}
	}
	return true;
}




}
} /* namespace rtps */
} /* namespace eprosima */


