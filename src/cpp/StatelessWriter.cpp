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
	lastChangeSequenceNumber = 0;
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
	if(!reader_locator.empty())
	{
		std::vector<ReaderLocator>::iterator rit;
		//Pointer to pointer;
		CacheChange_t** cpoin = (CacheChange_t**)malloc(sizeof(CacheChange_t*));
		if(writer_cache.get_change(sn,cpoin))
		{
			for(rit=reader_locator.begin();rit!=reader_locator.end();rit++)
			{
				rit->unsent_changes.push_back(*cpoin);
			}
			unsent_changes_not_empty();
		}
		else
			cout << "Failed to get change" << endl;
	}
}

void StatelessWriter::unsent_changes_not_empty(){
	std::vector<ReaderLocator>::iterator rit;
	CacheChange_t** change = (CacheChange_t**)malloc(sizeof(CacheChange_t*));
	participant->threadSend.sendMutex.lock();
	for(rit=reader_locator.begin();rit!=reader_locator.end();rit++)
	{
		while(rit->next_unsent_change(change))
		{
			cout << B_BLUE << "Creating message " << DEF << endl;
			SubmsgData_t DataSubM;
			DataSubM.dataFlag = true;
			DataSubM.endiannessFlag = true;
			DataSubM.inlineQosFlag = true;
			DataSubM.keyFlag = false;
			DataSubM.readerId = ENTITYID_UNKNOWN;
			DataSubM.writerId = this->guid.entityId;
			DataSubM.writerSN = (*change)->sequenceNumber;
			DataSubM.serializedPayload.copy(&(*change)->serializedPayload);
			Parameter_t p1,p2;
//			p1.create(PID_TOPIC_NAME,std::string("Nombre de topico largoxx"));
//			DataSubM.inlineQos.push_back(p1);
			p2.createParameterLocator(PID_UNICAST_LOCATOR,participant->defaultUnicastLocatorList[0]);
			DataSubM.inlineQos.push_back(p2);
			CDRMessage_t msg;
			MC.createMessageData(&msg,participant->guid.guidPrefix,&DataSubM);
			participant->threadSend.sendSync(&msg,rit->locator);
			rit->remove_unsent_change((*change));
		}
	}
	participant->threadSend.sendMutex.unlock();
	cout << "Finish sending unsent changes" << endl;
}


} /* namespace rtps */
} /* namespace eprosima */


