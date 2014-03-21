/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * RTPSWriter.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *      		grcanosa@gmail.com
 */

#include "eprosimartps/writer/RTPSWriter.h"
#include "eprosimartps/HistoryCache.h"
#include "eprosimartps/CDRMessage.h"
#include "eprosimartps/dds/Publisher.h"

#include "eprosimartps/dds/ParameterList.h"

namespace eprosima {
namespace rtps {

RTPSWriter::RTPSWriter() {
	// TODO Auto-generated constructor stub

}

RTPSWriter::~RTPSWriter() {
	// TODO Auto-generated destructor stub
}

bool RTPSWriter::new_change(ChangeKind_t changeKind,
		SerializedPayload_t* data, InstanceHandle_t handle,CacheChange_t* change)
{
	change->kind = changeKind;
	//change->sequenceNumber = lastChangeSequenceNumber;
	change->writerGUID = guid;
	change->instanceHandle = handle;
	if(change->kind == ALIVE)
		change->serializedPayload.copy(data);
	return true;
}

void RTPSWriter::DataSubM(CDRMessage_t* submsg,bool expectsInlineQos,CacheChange_t* change,EntityId_t ReaderId)
{
	ParameterList_t* inlineQos;
	if(expectsInlineQos)
		inlineQos = &Pub->ParamList;
	else
		inlineQos = NULL;
	RTPSMessageCreator::createSubmessageData(submsg,change,topicKind,ReaderId,inlineQos);
}


void RTPSWriter::sendChangesList(std::vector<CacheChange_t*> changes,
		std::vector<Locator_t>* unicast,std::vector<Locator_t>* multicast,
		bool expectsInlineQos,EntityId_t ReaderId)
{
	boost::lock_guard<ThreadSend> guard(participant->threadSend);
	pDebugInfo("Sending relevant changes as data messages" << endl);
	std::vector<Locator_t>::iterator lit;

	std::vector<CacheChange_t*>::iterator cit;
	cit = changes.begin();
	CDRMessage_t header(RTPSMESSAGE_HEADER_SIZE);
	RTPSMessageCreator::createHeader(&header,participant->guid.guidPrefix);
	uint16_t data_msg_size = 0;
	uint16_t change_n = 1;
	//FIRST SUBMESSAGE
	CDRMessage_t submessage;
	DataSubM(&submessage, expectsInlineQos,*cit,ReaderId);
	data_msg_size = submessage.length;
	if(data_msg_size+RTPSMESSAGE_HEADER_SIZE > RTPSMESSAGE_MAX_SIZE)
	{
		pError("The Data messages are larger than max size, fragmentation needed" << endl);
	}
	bool first = true;
	do
	{
		CDRMessage_t fullmsg;
		CDRMessage::appendMsg(&fullmsg,&header);
		RTPSMessageCreator::createSubmessageInfoTS_Now(&fullmsg,false);
		if(first)
		{
			CDRMessage::appendMsg(&fullmsg,&submessage);
			first = false;
		}
		while(fullmsg.length + data_msg_size < fullmsg.max_size
				&& (change_n + 1) <= changes.size()) //another one fits in the full message
		{
			change_n++;
			cit++;
			DataSubM(&submessage, expectsInlineQos,*cit,ReaderId);
			CDRMessage::appendMsg(&fullmsg,&submessage);
		}
		if(unicast!=NULL)
		{
			for(lit = unicast->begin();lit!=unicast->end();lit++)
				participant->threadSend.sendSync(&fullmsg,*lit);
		}

		if(multicast!=NULL)
		{
			for(lit = multicast->begin();lit!=multicast->end();lit++)
				participant->threadSend.sendSync(&fullmsg,*lit);
		}

	}while(change_n < changes.size()); //There is still a message to add
}














} /* namespace rtps */
} /* namespace eprosima */

