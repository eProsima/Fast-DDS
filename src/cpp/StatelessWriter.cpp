/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * StatelessWriter.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com
 */

#include "eprosimartps/StatelessWriter.h"
#include "eprosimartps/ReaderLocator.h"
#include "eprosimartps/ParameterListCreator.h"

namespace eprosima {
namespace rtps {

StatelessWriter::StatelessWriter() {
	// TODO Auto-generated constructor stub


}

void StatelessWriter::init(WriterParams_t param) {
	pushMode = param.pushMode;
	heartbeatPeriod = param.heartbeatPeriod;
	nackResponseDelay = param.nackResponseDelay;
	nackSupressionDuration = param.nackSupressionDuration;
	resendDataPeriod = param.resendDataPeriod;
	//writer_cache.changes.reserve(param.historySize);
	writer_cache.historySize = param.historySize;
	writer_cache.historyKind = WRITER;
	lastChangeSequenceNumber.high= 0;
	lastChangeSequenceNumber.low = 0;
	heartbeatCount = 0;
	stateType = STATELESS;
	writer_cache.rtpswriter = (RTPSWriter*)this;
	//locator lists:
	unicastLocatorList = param.unicastLocatorList;
	multicastLocatorList = param.multicastLocatorList;
	reliabilityKind = param.reliabilityKind;
	topicKind = param.topicKind;
}


StatelessWriter::~StatelessWriter() {
	// TODO Auto-generated destructor stub
}

bool StatelessWriter::reader_locator_add(ReaderLocator a_locator) {
	std::vector<ReaderLocator>::iterator rit;
	for(rit=reader_locator.begin();rit!=reader_locator.end();rit++){
		if(rit->locator == a_locator.locator)
			return false;
	}
	std::vector<CacheChange_t*>::iterator it;
	for(it = writer_cache.changes.begin();it!=writer_cache.changes.end();it++){
		a_locator.unsent_changes.push_back((*it));
	}
	reader_locator.push_back(a_locator);
	return true;
}

bool StatelessWriter::reader_locator_remove(Locator_t locator) {
	std::vector<ReaderLocator>::iterator it;
	for(it=reader_locator.begin();it!=reader_locator.end();it++){
		if(it->locator == locator){
			reader_locator.erase(it);
			return true;
		}
	}
	return false;
}

void StatelessWriter::unsent_changes_reset() {
	std::vector<ReaderLocator>::iterator rit;
	std::vector<CacheChange_t*>::iterator cit;
	for(rit=reader_locator.begin();rit!=reader_locator.end();rit++){
		rit->unsent_changes.clear();
		for(cit=writer_cache.changes.begin();cit!=writer_cache.changes.end();cit++){
			rit->unsent_changes.push_back((*cit));
		}
	}
	unsent_changes_not_empty();
}

bool sort_cacheChanges (CacheChange_t* c1,CacheChange_t* c2)
{
	return(c1->sequenceNumber.to64long() < c2->sequenceNumber.to64long());
}

void StatelessWriter::unsent_change_add(CacheChange_t* cptr)
{
	if(!reader_locator.empty())
	{
		std::vector<ReaderLocator>::iterator rit;
		for(rit=reader_locator.begin();rit!=reader_locator.end();rit++)
		{
			rit->unsent_changes.push_back(cptr);
		}
		//Order vector.
		//std::sort(rit->unsent_changes.begin(),rit->unsent_changes.end(),sort_cacheChanges);
		unsent_changes_not_empty();
	}
	else
	{
		RTPSLog::Warning << "No reader locator to add change" << std::endl;
		RTPSLog::printWarning();
	}
}

void StatelessWriter::unsent_changes_not_empty()
{
	std::vector<ReaderLocator>::iterator rit;
	CacheChange_t** change = (CacheChange_t**)malloc(sizeof(CacheChange_t*));
	boost::lock_guard<ThreadSend> guard(participant->threadSend);
	for(rit=reader_locator.begin();rit!=reader_locator.end();rit++)
	{
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


