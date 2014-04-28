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


WriterProxy::~WriterProxy()
{
	pDebugInfo("WriterProxy destructor"<<endl;);
}

WriterProxy::WriterProxy(WriterProxy_t* WPparam,StatefulReader* SR) :
		mp_SFR(SR),
		param(*WPparam),
		m_acknackCount(0),
		m_lastHeartbeatCount(0),
		m_isMissingChangesEmpty(true),
		m_heartbeatResponse(this,boost::posix_time::milliseconds(SR->reliability.heartbeatResponseDelay.to64time()*1000)),
		m_heartbeatFinalFlag(false)

{

}

bool WriterProxy::missing_changes_update(SequenceNumber_t* seqNum)
{
	pDebugInfo("WriterProxy:MISSING_changes_update: up to seqNum: "<<seqNum->to64long()<<endl);
	boost::lock_guard<WriterProxy> guard(*this);
	SequenceNumber_t seq = (*seqNum)+1;
	add_unknown_changes(seq);

	for(std::vector<ChangeFromWriter_t>::iterator cit=m_changesFromW.begin();cit!=m_changesFromW.end();++cit)
	{
		if(cit->status == MISSING)
			m_isMissingChangesEmpty = false;
		if(cit->status == UNKNOWN)
		{
			if(cit->change->sequenceNumber.to64long() <= seqNum->to64long())
			{
				cit->status = MISSING;
				m_isMissingChangesEmpty = false;
			}
		}

	}
	return true;
}

bool WriterProxy::lost_changes_update(SequenceNumber_t* seqNum)
{
	pDebugInfo("WriterProxy:LOST_changes_update: up to seqNum: "<<seqNum->to64long()<<endl);
	boost::lock_guard<WriterProxy> guard(*this);
	SequenceNumber_t seq = (*seqNum)+1;
	add_unknown_changes(seq);

	for(std::vector<ChangeFromWriter_t>::iterator cit=m_changesFromW.begin();cit!=m_changesFromW.end();++cit)
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
	pDebugInfo("WriterProxy:RECEIVED_changes_set: change with seqNum: "<<change->sequenceNumber.to64long()<<endl);
	boost::lock_guard<WriterProxy> guard(*this);
	for(std::vector<ChangeFromWriter_t>::iterator cit=m_changesFromW.begin();cit!=m_changesFromW.end();++cit)
	{
		if(cit->change->sequenceNumber.to64long() == change->sequenceNumber.to64long())
		{
			mp_SFR->m_reader_cache.release_Cache(cit->change);
			cit->change = change;
			cit->status = RECEIVED;
			return true;
		}
	}
	ChangeFromWriter_t chfw;
	chfw.change = change;
	chfw.is_relevant = true;
	chfw.status = RECEIVED;
	add_unknown_changes(change->sequenceNumber);
	m_changesFromW.push_back(chfw);

	return true;
}

bool WriterProxy::irrelevant_change_set(SequenceNumber_t* seqNum)
{
	boost::lock_guard<WriterProxy> guard(*this);
	for(std::vector<ChangeFromWriter_t>::iterator cit=m_changesFromW.begin();cit!=m_changesFromW.end();++cit)
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
	CacheChange_t* ch = mp_SFR->m_reader_cache.reserve_Cache();
	ch->sequenceNumber = *seqNum;
	chfw.change = ch;
	add_unknown_changes(*seqNum);
	m_changesFromW.push_back(chfw);

	return true;
}

bool sort_chFW (ChangeFromWriter_t c1,ChangeFromWriter_t c2)
{
	return(c1.change->sequenceNumber.to64long() < c2.change->sequenceNumber.to64long());
}


bool WriterProxy::missing_changes(std::vector<ChangeFromWriter_t*>* missing)
{
	if(!m_changesFromW.empty())
	{
		boost::lock_guard<WriterProxy> guard(*this);
		missing->clear();
		for(std::vector<ChangeFromWriter_t>::iterator it=m_changesFromW.begin();it!=m_changesFromW.end();++it)
		{
			if(it->status == MISSING)
				missing->push_back(&(*it));
		}
		if(missing->empty())
			m_isMissingChangesEmpty = true;
		return true;
	}
	else
		return false;
}



bool WriterProxy::available_changes_max(SequenceNumber_t* seqNum)
{
	if(!m_changesFromW.empty())
	{
		boost::lock_guard<WriterProxy> guard(*this);
		//Order changesFromWriter
		std::sort(m_changesFromW.begin(),m_changesFromW.end(),sort_chFW);
		seqNum->high = 0;
		seqNum->low = 0;
		//We check the rest for the largest one with Status Received or lost
		for(std::vector<ChangeFromWriter_t>::iterator it=m_changesFromW.begin();it!=m_changesFromW.end();++it)
		{
			if(it->status == RECEIVED || it->status == LOST)
				*seqNum = it->change->sequenceNumber;
			else
				break;
		}
		return true;

	}
	pDebugInfo("WriterProxy:available_changes_max:no changesFromW"<<endl);
	return false;
}


bool WriterProxy::available_changes_min(SequenceNumber_t* seqNum)
{
	if(!m_changesFromW.empty())
	{
		boost::lock_guard<WriterProxy> guard(*this);
		//Order changesFromWriter
		std::sort(m_changesFromW.begin(),m_changesFromW.end(),sort_chFW);
		seqNum->high = 0;
		seqNum->low = 0;
		bool cont = false;

		for(std::vector<ChangeFromWriter_t>::iterator it=m_changesFromW.begin();it!=m_changesFromW.end();++it)
		{
			if(it->status == RECEIVED)
			{
				*seqNum = it->change->sequenceNumber;
				return true;
			}
			else if(it->status == LOST)
			{

			}
			else
			{
				return false;
			}
		}
	}
	pDebugInfo("WriterProxy:available_changes_max:no changesFromW"<<endl);
	return false;
}


SequenceNumber_t WriterProxy::max_seq_num()
{
	SequenceNumber_t seq;
	if(!m_changesFromW.empty())
	{
		boost::lock_guard<WriterProxy> guard(*this);
		std::sort(m_changesFromW.begin(),m_changesFromW.end(),sort_chFW);
		std::vector<ChangeFromWriter_t>::iterator it = m_changesFromW.end()-1;
		seq = it->change->sequenceNumber;
	}
	else
	{
		seq.high = 0;
		seq.low = 0;
	}
	return seq;
}


bool WriterProxy::add_unknown_changes(SequenceNumber_t& seq)
{

	boost::lock_guard<WriterProxy> guard(*this);
	uint64_t n_to_add;
	SequenceNumber_t maxseqNum = max_seq_num();
	if((maxseqNum+1) < (seq)) //if the maximum plus one is less than our seqNum we need to add
	{
		n_to_add = (seq.to64long() - maxseqNum.to64long() -1);
		pDebugInfo("WriterProxy:add_unknown_changes: up to: "<<seq.to64long() << " || adding " << n_to_add << " changes."<<endl;);
		while(n_to_add>0)
		{
			CacheChange_t* ch = mp_SFR->m_reader_cache.reserve_Cache();
			ch->sequenceNumber = seq-n_to_add;
			ChangeFromWriter_t chfw;
			chfw.change = ch;
			chfw.status = UNKNOWN;
			chfw.is_relevant = false;
			m_changesFromW.push_back(chfw);
			n_to_add--;
		}
	}
	return true;

}

bool WriterProxy::removeChangeFromWriter(SequenceNumber_t& seq)
{
	for(std::vector<ChangeFromWriter_t>::iterator it=m_changesFromW.begin();it!=m_changesFromW.end();++it)
	{
		if(it->change !=NULL)
		{
			if(it->change->sequenceNumber.to64long() == seq.to64long())
			{
				if(it->status == RECEIVED || it->status == LOST)
				{
					m_lastRemovedSeqNum = it->change->sequenceNumber;
					m_changesFromW.erase(it);
					return true;
				}
				else
					return false;
			}
		}
	}
	return false;
}


} /* namespace rtps */
} /* namespace eprosima */
