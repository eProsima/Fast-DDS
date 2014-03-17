/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file StatefulWriter.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/StatefulWriter.h"
#include "eprosimartps/ReaderProxy.h"



namespace eprosima {
namespace rtps {

StatefulWriter::StatefulWriter() {
	// TODO Auto-generated constructor stub

}

StatefulWriter::~StatefulWriter() {
	// TODO Auto-generated destructor stub
}


void StatefulWriter::init(WriterParams_t param)
{
	pushMode = param.pushMode;
	heartbeatPeriod = param.heartbeatPeriod;
	nackResponseDelay = param.nackResponseDelay;
	nackSupressionDuration = param.nackSupressionDuration;

	//writer_cache.changes.reserve(param.historySize);
	writer_cache.historySize = param.historySize;
	writer_cache.historyKind = WRITER;
	lastChangeSequenceNumber.high= 0;
	lastChangeSequenceNumber.low = 0;
	heartbeatCount = 0;
	stateType = STATEFUL;
	writer_cache.rtpswriter = (RTPSWriter*)this;
	//locator lists:
	unicastLocatorList = param.unicastLocatorList;
	multicastLocatorList = param.multicastLocatorList;
	reliabilityKind = param.reliabilityKind;
	topicKind = param.topicKind;
}

bool StatefulWriter::matched_reader_add(ReaderProxy_t RPparam)
{
	std::vector<ReaderProxy*>::iterator it;
	for(it=matched_readers.begin();it!=matched_readers.end();it++)
	{
		if((*it)->param.remoteReaderGuid == RPparam.remoteReaderGuid)
		{
			RTPSLog::Warning << "Attempting to add existing reader" << endl;pW
			return false;
		}
	}
	ReaderProxy* rp = new ReaderProxy();
	rp->param = RPparam;

	std::vector<CacheChange_t*>::iterator cit;
	for(cit=writer_cache.changes.begin();cit!=writer_cache.changes.end();it++)
	{
		ChangeForReader_t changeForReader;
		changeForReader.change = (*cit);
		changeForReader.is_relevant = rp->dds_is_relevant(*cit);

		if(pushMode)
			changeForReader.status = UNSENT;
		else
			changeForReader.status = UNACKNOWLEDGED;
		rp->changes.push_back(changeForReader);
	}
	matched_readers.push_back(rp);
	RTPSLog::DebugInfo << "Reader Proxy added" << endl;pDI
	return true;
}

bool StatefulWriter::matched_reader_remove(ReaderProxy_t Rp)
{
	return matched_reader_remove(Rp.remoteReaderGuid);
}

bool StatefulWriter::matched_reader_remove(GUID_t readerGuid)
{
	std::vector<ReaderProxy*>::iterator it;
	for(it=matched_readers.begin();it!=matched_readers.end();it++)
	{
		if((*it)->param.remoteReaderGuid == readerGuid)
		{
			delete(*it);
			matched_readers.erase(it);
			RTPSLog::DebugInfo << "Reader Proxy removed" << endl;pDI
			return true;
		}
	}
	RTPSLog::Info << "Reader Proxy doesn't exist in this writer" << endl;pI
	return false;
}

bool StatefulWriter::matched_reader_lookup(GUID_t readerGuid,ReaderProxy** RP)
{
	std::vector<ReaderProxy*>::iterator it;
	for(it=matched_readers.begin();it!=matched_readers.end();it++)
	{
		if((*it)->param.remoteReaderGuid == readerGuid)
		{
			*RP = *it;
			return true;
		}
	}
	return false;
}

bool StatefulWriter::is_acked_by_all(CacheChange_t* change)
{
	std::vector<ReaderProxy*>::iterator it;
	for(it=matched_readers.begin();it!=matched_readers.end();it++)
	{
		ChangeForReader_t changeForReader;
		if((*it)->getChangeForReader(change,&changeForReader))
		{
			if(!changeForReader.is_relevant
					|| !(changeForReader.status == ACKNOWLEDGED))
			{
				RTPSLog::DebugInfo << "Change not acked. Relevant: " << changeForReader.is_relevant;
				RTPSLog::DebugInfo << " status: " << changeForReader.status << endl;pDI
				return false;
			}
		}
	}
	return true;
}

void StatefulWriter::unsent_change_add(CacheChange_t* change)
{
	if(!matched_readers.empty())
	{
		std::vector<ReaderProxy*>::iterator it;
		for(it=matched_readers.begin();it!=matched_readers.end();it++)
		{
			ChangeForReader_t changeForReader;
			changeForReader.change = change;
			changeForReader.status = UNSENT;
			changeForReader.is_relevant = (*it)->dds_is_relevant(change);
			(*it)->changes.push_back(changeForReader);
		}
		unsent_changes_not_empty();
	}
	else
	{
		RTPSLog::Warning << "No reader proxy to add change." << endl;pW
	}
}

bool sort_changeForReader (ChangeForReader_t* c1,ChangeForReader_t* c2)
{
	return(c1->change->sequenceNumber.to64long() < c2->change->sequenceNumber.to64long());
}

void StatefulWriter::unsent_changes_not_empty()
{
	std::vector<ReaderProxy*>::iterator rit;
	boost::lock_guard<ThreadSend> guard(participant->threadSend);
	for(rit=matched_readers.begin();rit!=matched_readers.end();rit++)
	{
		std::vector<ChangeForReader_t*> ch_vec;
		if((*rit)->unsent_changes(&ch_vec))
		{
			std::sort(ch_vec.begin(),ch_vec.end(),sort_cacheChanges);
			std::vector<ChangeForReader_t*>::iterator cit;
			for(cit=ch_vec.begin();cit!=ch_vec.end();cit++)
			{
				if((*cit)->is_relevant) //DATA Message
				{
					SubmsgData_t DataSubM;
					DataSubM.expectsInlineQos = (*rit)->param.expectsInlineQos;
					if(DataSubM.expectsInlineQos)
					{
						DataSubM.inlineQos.params = Pub->ParamList.inlineqos_params;
						if(topicKind == WITH_KEY)
						{
							ParameterListCreator::addParameterKey(&DataSubM.inlineQos,PID_KEY_HASH,(*cit)->change->instanceHandle);
							if((*cit)->change->kind !=ALIVE)
							{
								octet status = (*cit)->change->kind == NOT_ALIVE_DISPOSED ? 1:0;
								status = (*cit)->change->kind == NOT_ALIVE_UNREGISTERED ? 2:status;
								ParameterListCreator::addParameterStatus(&DataSubM.inlineQos,PID_STATUS_INFO,status);
							}
						}
					}
					DataSubM.instanceHandle = (*cit)->change->instanceHandle;
					DataSubM.changeKind = (*cit)->change->kind;
					DataSubM.readerId = ENTITYID_UNKNOWN;
					DataSubM.writerId = this->guid.entityId;
					DataSubM.writerSN = (*cit)->change->sequenceNumber;
					DataSubM.serializedPayload.copy(&(*cit)->change->serializedPayload);
					RTPSLog::DebugInfo << "Sending message with seqNum: " << (*cit)->change->sequenceNumber.to64long() << endl;
					RTPSLog::printDebugInfo();
					CDRMessage_t msg;
					MC.createMessageData(&msg,participant->guid.guidPrefix,&DataSubM,(RTPSWriter*)this);

				}
				else //GAP message
				{

				}
				std::vector<Locator_t>::iterator lit;
				for(lit=(*rit)->param.unicastLocatorList.begin();lit!=(*rit)->param.unicastLocatorList.end();lit++)
				{
					participant->threadSend.sendSync(&msg,*lit);
				}
				for(lit=(*rit)->param.multicastLocatorList.begin();lit!=(*rit)->param.multicastLocatorList.end();lit++)
				{
					participant->threadSend.sendSync(&msg,*lit);
				}
			}
		}
		while(rit->next_unsent_change(change))
		{
			if(pushMode)
			{
				SubmsgData_t DataSubM;
				DataSubM.expectsInlineQos = rit->expectsInlineQos;
				if(DataSubM.expectsInlineQos)
				{
					DataSubM.inlineQos.params = Pub->ParamList.inlineqos_params;
					if(topicKind == WITH_KEY)
					{
						ParameterListCreator::addParameterKey(&DataSubM.inlineQos,PID_KEY_HASH,(*change)->instanceHandle);
						if((*change)->kind !=ALIVE)
						{
							octet status = (*change)->kind == NOT_ALIVE_DISPOSED ? 1:0;
							status = (*change)->kind == NOT_ALIVE_UNREGISTERED ? 2:status;
							ParameterListCreator::addParameterStatus(&DataSubM.inlineQos,PID_STATUS_INFO,status);
						}
					}
				}

				DataSubM.instanceHandle = (*change)->instanceHandle;
				DataSubM.changeKind = (*change)->kind;
				DataSubM.readerId = ENTITYID_UNKNOWN;
				DataSubM.writerId = this->guid.entityId;
				DataSubM.writerSN = (*change)->sequenceNumber;
				DataSubM.serializedPayload.copy(&(*change)->serializedPayload);
				RTPSLog::DebugInfo << "Sending message with seqNum: " << (*change)->sequenceNumber.to64long() << endl;
				RTPSLog::printDebugInfo();
				CDRMessage_t msg;
				MC.createMessageData(&msg,participant->guid.guidPrefix,&DataSubM,(RTPSWriter*)this);
				participant->threadSend.sendSync(&msg,rit->locator);
				rit->remove_unsent_change((*change));
			}
			else
			{
				//FIXME: Send Heartbeats indicating new data
				SubmsgHeartbeat_t HBSubM;
				HBSubM.finalFlag = true;
				HBSubM.livelinessFlag = false; //TODOG: esto es asi?
				HBSubM.readerId = ENTITYID_UNKNOWN;
				HBSubM.writerId = this->guid.entityId;
				writer_cache.get_seq_num_min(&HBSubM.firstSN,NULL);
				writer_cache.get_seq_num_max(&HBSubM.lastSN,NULL);
				heartbeatCount++;
				HBSubM.count = heartbeatCount;
				CDRMessage_t msg;
				MC.createMessageHeartbeat(&msg,participant->guid.guidPrefix,&HBSubM);
				participant->threadSend.sendSync(&msg,rit->locator);
				rit->remove_unsent_change((*change));
			}
		}
	}
	RTPSLog::DebugInfo << "Finish sending unsent changes" << endl;RTPSLog::printDebugInfo();
}


} /* namespace rtps */
} /* namespace eprosima */
