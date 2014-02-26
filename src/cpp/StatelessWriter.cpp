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
 */

#include "eprosimartps/StatelessWriter.h"
#include "eprosimartps/ReaderLocator.h"

namespace eprosima {
namespace rtps {

StatelessWriter::StatelessWriter() {
	// TODO Auto-generated constructor stub

}

void StatelessWriter::init(WriterParams_t param) {
	// TODO Auto-generated constructor stub
	pushMode = param.pushMode;
	heartbeatPeriod = param.heartbeatPeriod;
	nackResponseDelay = param.nackResponseDelay;
	nackSupressionDuration = param.nackSupressionDuration;
	resendDataPeriod = param.resendDataPeriod;
	writer_cache.changes.reserve(param.HistorySize);
	lastChangeSequenceNumber = 0;
	stateType = STATELESS;
	writer_cache.rtpswriter = (RTPSWriter*)this;
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
	std::vector<CacheChange_t>::iterator it;
	for(it = writer_cache.changes.begin();it!=writer_cache.changes.end();it++){
		a_locator.unsent_changes.push_back(&(*it));
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
	std::vector<CacheChange_t>::iterator cit;
	for(rit=reader_locator.begin();rit!=reader_locator.end();rit++){
		rit->unsent_changes.clear();
		for(cit=writer_cache.changes.begin();cit!=writer_cache.changes.end();cit++){
			rit->unsent_changes.push_back(&(*cit));
		}
	}
	unsent_changes_not_empty();
}

void StatelessWriter::unsent_change_add(SequenceNumber_t sn) {
	std::vector<ReaderLocator>::iterator rit;
	CacheChange_t* cpoin;
	writer_cache.get_change(sn,cpoin);
	for(rit=reader_locator.begin();rit!=reader_locator.end();rit++){
		rit->unsent_changes.push_back(cpoin);
	}
	unsent_changes_not_empty();
}

void StatelessWriter::unsent_changes_not_empty(){
	std::vector<ReaderLocator>::iterator rit;
	CacheChange_t* change = NULL;
	participant->threadSend.sendMutex.lock();
	for(rit=reader_locator.begin();rit!=reader_locator.end();rit++)
	{
		while(rit->next_unsent_change(change))
		{
			SubmsgData_t DataSubM;
			DataSubM.dataFlag = true;
			DataSubM.endiannessFlag = true;
			DataSubM.inlineQosFlag = false;
			DataSubM.keyFlag = false;
			DataSubM.readerId = ENTITYID_UNKNOWN;
			DataSubM.writerId = this->guid.entityId;
			DataSubM.writerSN = change->sequenceNumber;
			DataSubM.serializedPayload = change->serializedPayload;
			CDRMessage_t msg;
			MC.createMessageData(&msg,participant->guid.guidPrefix,&DataSubM);
			participant->threadSend.sendSync(msg,rit->locator);
		}
	}
	participant->threadSend.sendMutex.unlock();
}


} /* namespace rtps */
} /* namespace eprosima */


