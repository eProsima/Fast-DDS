/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterProxy.cpp
 *
 *  Created on: Mar 24, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/reader/WriterProxy.h"
#include "eprosimartps/reader/StatefulReader.h"


namespace eprosima {
namespace rtps {


WriterProxy::~WriterProxy() {
	// TODO Auto-generated destructor stub
}

WriterProxy::WriterProxy(WriterProxy_t* WPparam,StatefulReader* SR) :
		param(*WPparam),
		acknackCount(0),lastHeartbeatCount(0),isMissingChangesEmpty(true),
		heartbeatResponse(SR,boost::posix_time::milliseconds(SR->reliability.heartbeatResponseDelay.to64time()*1000)),
		SFR(SR)
{


}

bool WriterProxy::missing_changes_update(SequenceNumber_t* seqNum)
{
	boost::lock_guard<WriterProxy> guard(*this);
	SequenceNumber_t maxSeqNum;
	this->max_seq_num(&maxSeqNum);
	if(maxSeqNum < *seqNum)
		add_unknown_changes((seqNum++));

	for(std::vector<ChangeFromWriter_t>::iterator cit=changes.begin();cit!=changes.end();++cit)
	{
		if(cit->status == UNKNOWN)
		{
			if(cit->change->sequenceNumber.to64long() <= seqNum->to64long())
			{
				cit->status = MISSING;
				isMissingChangesEmpty = false;
			}
		}
	}
	return true;
}

bool WriterProxy::lost_changes_update(SequenceNumber_t* seqNum)
{
	boost::lock_guard<WriterProxy> guard(*this);
	SequenceNumber_t maxSeqNum;
	this->max_seq_num(&maxSeqNum);
	if(maxSeqNum < *seqNum)
		add_unknown_changes((seqNum++));

	for(std::vector<ChangeFromWriter_t>::iterator cit=changes.begin();cit!=changes.end();++cit)
	{
		if(cit->status == UNKNOWN || cit->status == MISSING)
		{
			if(cit->change->sequenceNumber.to64long() < seqNum->to64long())
				cit->status = LOST;
		}
	}
	return true;
}

bool WriterProxy::received_change_set(CacheChange_t* change)
{
	boost::lock_guard<WriterProxy> guard(*this);
	for(std::vector<ChangeFromWriter_t>::iterator cit=changes.begin();cit!=changes.end();++cit)
	{
		if(cit->change->sequenceNumber.to64long() == change->sequenceNumber.to64long())
		{
			SFR->reader_cache.release_Cache(cit->change);
			cit->change = change;
			cit->status = RECEIVED;
			return true;
		}
	}
	ChangeFromWriter_t chfw;
	chfw.change = change;
	chfw.is_relevant = true;
	chfw.status = RECEIVED;
	SequenceNumber_t maxSeqNum;
	max_seq_num(&maxSeqNum);
	maxSeqNum++;
	if(maxSeqNum < change->sequenceNumber)
		add_unknown_changes(&change->sequenceNumber);
	changes.push_back(chfw);

	return true;
}

bool WriterProxy::irrelevant_change_set(SequenceNumber_t* seqNum)
{
	boost::lock_guard<WriterProxy> guard(*this);
	for(std::vector<ChangeFromWriter_t>::iterator cit=changes.begin();cit!=changes.end();++cit)
	{
		if(cit->change->sequenceNumber.to64long() == seqNum->to64long())
		{
			cit->status = RECEIVED;
			cit->is_relevant = false;
			return true;
		}
	}
	ChangeFromWriter_t chfw;
	chfw.is_relevant = false;
	chfw.status = RECEIVED;
	CacheChange_t* ch = SFR->reader_cache.reserve_Cache();
	ch->sequenceNumber = *seqNum;
	chfw.change = ch;
	SequenceNumber_t maxSeqNum;
	max_seq_num(&maxSeqNum);
	maxSeqNum++;
	if(maxSeqNum < *seqNum)
		add_unknown_changes(seqNum);
	changes.push_back(chfw);

	return true;
}

bool sort_chFW (ChangeFromWriter_t c1,ChangeFromWriter_t c2)
{
	return(c1.change->sequenceNumber.to64long() < c2.change->sequenceNumber.to64long());
}


bool WriterProxy::max_seq_num(SequenceNumber_t* sn)
{
	if(!changes.empty())
	{
		boost::lock_guard<WriterProxy> guard(*this);
		std::sort(changes.begin(),changes.end(),sort_chFW);
		std::vector<ChangeFromWriter_t>::iterator it = changes.end();
		*sn = it->change->sequenceNumber;
	}
	else
	{
		sn->high = 0;
		sn->low = 0;
	}
	return true;
}

bool WriterProxy::missing_changes(std::vector<ChangeFromWriter_t*>* missing)
{
	boost::lock_guard<WriterProxy> guard(*this);
	missing->clear();
	for(std::vector<ChangeFromWriter_t>::iterator it=changes.begin();it!=changes.end();++it)
	{
		if(it->status == MISSING)
			missing->push_back(&(*it));
	}
	if(missing->empty())
		isMissingChangesEmpty = true;
	return true;
}



bool WriterProxy::available_changes_max(SequenceNumber_t* seqNum)
{
	if(!changes.empty())
	{
		boost::lock_guard<WriterProxy> guard(*this);
		std::sort(changes.begin(),changes.end(),sort_chFW);
		if(changes.begin()->status == RECEIVED || changes.begin()->status == LOST)
			*seqNum = changes.begin()->change->sequenceNumber;
		else
			return false;

		for(std::vector<ChangeFromWriter_t>::iterator it=changes.begin()+1;it!=changes.end();++it)
		{
			if(it->status == RECEIVED || it->status == LOST)
				*seqNum = it->change->sequenceNumber;
			else
				return true;
		}

	}
	pDebugInfo("No changes in writer proxy");
	return false;
}

bool WriterProxy::add_unknown_changes(SequenceNumber_t* sn)
{
	boost::lock_guard<WriterProxy> guard(*this);
	uint32_t n_to_add;
	if(!changes.empty())
	{
		std::sort(changes.begin(),changes.end(),sort_chFW);
		n_to_add = sn->to64long() - (*changes.end()).change->sequenceNumber.to64long()-1;
	}
	else
	{
		n_to_add = sn->to64long()-1;
	}
	while(n_to_add>0)
	{
		CacheChange_t* ch = SFR->reader_cache.reserve_Cache();
		ch->sequenceNumber = (*sn)-n_to_add;
		ChangeFromWriter_t chfw;
		chfw.change = ch;
		chfw.status = UNKNOWN;
		changes.push_back(chfw);
		n_to_add--;
	}
	return true;

}


} /* namespace rtps */
} /* namespace eprosima */
