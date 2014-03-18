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
	boost::lock_guard<ThreadSend> guard(participant->threadSend);
	for(rit=reader_locator.begin();rit!=reader_locator.end();rit++)
	{
		if(pushMode)
		{
			if(!rit->unsent_changes.empty())
			{
				std::sort(rit->unsent_changes.begin(),rit->unsent_changes.end(),sort_cacheChanges);
				std::vector<Locator_t> loc;
				loc.push_back(rit->locator);
				sendChangesList(rit->unsent_changes,&loc,NULL,rit->expectsInlineQos,ENTITYID_UNKNOWN);
				rit->unsent_changes.clear();
			}
		}
		else
		{
			CDRMessage_t msg;
			SequenceNumber_t first,last;
			writer_cache.get_seq_num_min(&first,NULL);
			writer_cache.get_seq_num_max(&last,NULL);
			heartbeatCount++;
			CDRMessageCreator::createMessageHeartbeat(&msg,participant->guid.guidPrefix,ENTITYID_UNKNOWN,this->guid.entityId,
					first,last,heartbeatCount,true,false);
			participant->threadSend.sendSync(&msg,rit->locator);
			rit->unsent_changes.clear();
		}

	}
	RTPSLog::DebugInfo << "Finish sending unsent changes" << endl;RTPSLog::printDebugInfo();
}


} /* namespace rtps */
} /* namespace eprosima */


