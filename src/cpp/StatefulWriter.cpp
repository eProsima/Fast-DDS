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
#include "eprosimartps/CDRMessage.h"
#include "eprosimartps/ParameterListCreator.h"



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
			pWarning("Attempting to add existing reader" << endl);
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
	pDebugInfo("Reader Proxy added" << endl);
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
			pDebugInfo("Reader Proxy removed" << endl);
			return true;
		}
	}
	pInfo("Reader Proxy doesn't exist in this writer" << endl)
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
				pDebugInfo("Change not acked. Relevant: " << changeForReader.is_relevant);
				pDebugInfo(" status: " << changeForReader.status << endl);
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
		pWarning("No reader proxy to add change." << endl);
	}
}

bool sort_changeForReader (ChangeForReader_t* c1,ChangeForReader_t* c2)
{
	return(c1->change->sequenceNumber.to64long() < c2->change->sequenceNumber.to64long());
}

bool sort_changes (CacheChange_t* c1,CacheChange_t* c2)
{
	return(c1->sequenceNumber.to64long() < c2->sequenceNumber.to64long());
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
			std::sort(ch_vec.begin(),ch_vec.end(),sort_changeForReader);
			//Get relevant data cache changes
			std::vector<CacheChange_t*> relevant_changes;
			std::vector<CacheChange_t*> not_relevant_changes;
			std::vector<ChangeForReader_t*>::iterator cit;
			for(cit = ch_vec.begin();cit!=ch_vec.end();cit++)
			{
				(*cit)->status = UNDERWAY;
				if((*cit)->is_relevant)
				{
					relevant_changes.push_back((*cit)->change);
				}
				else
				{
					not_relevant_changes.push_back((*cit)->change);
				}
			}
			if(!relevant_changes.empty())
				sendChangesList(relevant_changes,&(*rit)->param.unicastLocatorList,
					&(*rit)->param.multicastLocatorList,
					(*rit)->param.expectsInlineQos,
					(*rit)->param.remoteReaderGuid.entityId);
			if(!not_relevant_changes.empty())
				sendChangesListAsGap(&not_relevant_changes,
					(*rit)->param.remoteReaderGuid.entityId,
					&(*rit)->param.unicastLocatorList,
					&(*rit)->param.multicastLocatorList);



		}
	}
	pDebugInfo("Finish sending unsent changes" << endl);
}


void StatefulWriter::sendChangesListAsGap(std::vector<CacheChange_t*>* changes,
				EntityId_t readerId,std::vector<Locator_t>* unicast,std::vector<Locator_t>* multicast)
{
	//First compute the number of GAP messages we need:
	std::vector<CacheChange_t*>::iterator it;
	std::sort(changes->begin(),changes->end(),sort_changes);
	std::pair<SequenceNumber_t,SequenceNumberSet_t> pair;
	std::vector<std::pair<SequenceNumber_t,SequenceNumberSet_t>> Sequences;

	SequenceNumber_t start;
	SequenceNumberSet_t set;
	start = (*changes->begin())->sequenceNumber;
	uint32_t count = 1;
	bool set_first = true;
	bool new_pair = false;
	for(it=changes->begin()+1;it!=changes->end();it++)
	{
		if(new_pair)
		{
			start = (*it)->sequenceNumber;
			count = 1;
			new_pair = false;
			continue;
		}
		if(((*it)->sequenceNumber.to64long() - start.to64long()) == count) //continuous seqNum from start to base
		{
			count++;
			continue;
		}
		else
		{
			if(set_first)
			{
				set.base = (*it-1)->sequenceNumber; //add the last one as the base
				set.add((*it-1)->sequenceNumber); //also add it to the set
				set_first = false;
			}
			if(set.add((*it)->sequenceNumber)) //try to add the current one to the base
				continue;
			else //if we fail to add the element to the set is because they are to far away.
			{
				new_pair = true;
				set_first = true;
				it--;
				pair.first = start;
				pair.second = set;
				Sequences.push_back(pair);
				continue;
			}
		}
	}
	//Prepare the send operation
	CDRMessage_t header(RTPSMESSAGE_HEADER_SIZE);
	CDRMessageCreator::createHeader(&header,participant->guid.guidPrefix);
	std::vector<std::pair<SequenceNumber_t,SequenceNumberSet_t>>::iterator seqit;
	seqit = Sequences.begin();
	std::vector<Locator_t>::iterator lit;
	uint16_t gap_msg_size = 0;
	uint16_t gap_n = 1;
	//FIRST SUBMESSAGE
	CDRMessage_t submessage;
	CDRMessageCreator::createSubmessageGap(&submessage,seqit->first,seqit->second,
			readerId,this->guid.entityId);

	gap_msg_size = submessage.length;
	if(gap_msg_size+RTPSMESSAGE_HEADER_SIZE > RTPSMESSAGE_MAX_SIZE)
	{
		pError("The Gap messages are larger than max size, fragmentation needed" << endl);
	}
	bool first = true;
	do
	{
		CDRMessage_t fullmsg;
		CDRMessage::appendMsg(&fullmsg,&header);
		if(first)
		{
			CDRMessage::appendMsg(&fullmsg,&submessage);
			first = false;
		}
		while(fullmsg.length + gap_msg_size < fullmsg.max_size
				&& (gap_n + 1) <=Sequences.size()) //another one fits in the full message
		{
			gap_n++;
			seqit++;
			CDRMessage::initCDRMsg(&submessage);
			CDRMessageCreator::createSubmessageGap(&submessage,seqit->first,seqit->second,
						readerId,this->guid.entityId);
			CDRMessage::appendMsg(&fullmsg,&submessage);
		}
		for(lit = unicast->begin();lit!=unicast->end();lit++)
			participant->threadSend.sendSync(&fullmsg,*lit);
		for(lit = multicast->begin();lit!=multicast->end();lit++)
			participant->threadSend.sendSync(&fullmsg,*lit);

	}while(gap_n < Sequences.size()); //There is still a message to add
}



} /* namespace rtps */
} /* namespace eprosima */
