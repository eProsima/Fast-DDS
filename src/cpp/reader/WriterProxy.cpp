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
 */

#include "eprosimartps/reader/WriterProxy.h"
#include "eprosimartps/reader/StatefulReader.h"

#include "eprosimartps/utils/RTPSLog.h"


namespace eprosima {
namespace rtps {


WriterProxy::~WriterProxy()
{
	pDebugInfo("WriterProxy destructor"<<endl;);
	m_writerProxyLiveliness.stop_timer();
}

WriterProxy::WriterProxy(const WriterProxy_t& WPparam,
		Duration_t heartbeatResponse,
		StatefulReader* SR) :
		mp_SFR(SR),
		param(WPparam),
		m_acknackCount(0),
		m_lastHeartbeatCount(0),
		m_isMissingChangesEmpty(true),
		m_heartbeatResponse(this,boost::posix_time::milliseconds(Time_t2MilliSec(heartbeatResponse))),
		m_writerProxyLiveliness(this,boost::posix_time::milliseconds(Time_t2MilliSec(WPparam.leaseDuration))),
		m_heartbeatFinalFlag(false),
		m_hasMaxAvailableSeqNumChanged(false),
		m_hasMinAvailableSeqNumChanged(false),
		m_livelinessAsserted(true)

{
	m_changesFromW.clear();
	//cout << "WriterProxy CREATED with lease Duration: "<< Time_t2MilliSec(WPparam.leaseDuration)<<endl;
	Time_t aux;
	TIME_INFINITE(aux);
	if(WPparam.leaseDuration < aux)
		m_writerProxyLiveliness.restart_timer();
}

bool WriterProxy::missing_changes_update(SequenceNumber_t& seqNum)
{
	pDebugInfo("WriterProxy:MISSING_changes_update: up to seqNum: "<<seqNum.to64long()<<endl);
	boost::lock_guard<WriterProxy> guard(*this);
	SequenceNumber_t seq = (seqNum)+1;
	add_unknown_changes(seq);

	for(std::vector<ChangeFromWriter_t>::iterator cit=m_changesFromW.begin();cit!=m_changesFromW.end();++cit)
	{
		if(cit->status == MISSING)
			m_isMissingChangesEmpty = false;
		if(cit->status == UNKNOWN)
		{
			if(cit->change->sequenceNumber.to64long() <= seqNum.to64long())
			{
				cit->status = MISSING;
				m_isMissingChangesEmpty = false;
			}
		}

	}
	m_hasMaxAvailableSeqNumChanged = true;
		m_hasMinAvailableSeqNumChanged = true;
	return true;
}

bool WriterProxy::lost_changes_update(SequenceNumber_t& seqNum)
{
	pDebugInfo("WriterProxy:LOST_changes_update: up to seqNum: "<<seqNum.to64long()<<endl);
	boost::lock_guard<WriterProxy> guard(*this);
	SequenceNumber_t seq = (seqNum)+1;
	add_unknown_changes(seq);

	for(std::vector<ChangeFromWriter_t>::iterator cit=m_changesFromW.begin();cit!=m_changesFromW.end();++cit)
	{
		if(cit->status == UNKNOWN || cit->status == MISSING)
		{
			if(cit->change->sequenceNumber.to64long() < seqNum.to64long())
				cit->status = LOST;
		}
	}
	m_hasMaxAvailableSeqNumChanged = true;
		m_hasMinAvailableSeqNumChanged = true;
	return true;
}

bool WriterProxy::received_change_set(CacheChange_t* change)
{
	pDebugInfo("WriterProxy:RECEIVED_changes_set: change with seqNum: "<<change->sequenceNumber.to64long()<<endl);
	boost::lock_guard<WriterProxy> guard(*this);
	m_hasMaxAvailableSeqNumChanged = true;
		m_hasMinAvailableSeqNumChanged = true;
	for(std::vector<ChangeFromWriter_t>::iterator cit=m_changesFromW.begin();cit!=m_changesFromW.end();++cit)
	{
		if(cit->change->sequenceNumber.to64long() == change->sequenceNumber.to64long())
		{
			mp_SFR->release_Cache(cit->change);
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

bool WriterProxy::irrelevant_change_set(SequenceNumber_t& seqNum)
{
	boost::lock_guard<WriterProxy> guard(*this);
	m_hasMaxAvailableSeqNumChanged = true;
	m_hasMinAvailableSeqNumChanged = true;
	for(std::vector<ChangeFromWriter_t>::iterator cit=m_changesFromW.begin();cit!=m_changesFromW.end();++cit)
	{
		if(cit->change->sequenceNumber.to64long() == seqNum.to64long())
		{
			cit->status = RECEIVED;
			cit->is_relevant = false;
			return true;
		}
	}
	ChangeFromWriter_t chfw;
	chfw.is_relevant = false;
	chfw.status = RECEIVED;
	CacheChange_t* ch = mp_SFR->reserve_Cache();
	ch->sequenceNumber = seqNum;
	chfw.change = ch;
	add_unknown_changes(seqNum);
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
		if(m_hasMaxAvailableSeqNumChanged)
		{
			//Order changesFromWriter
			std::sort(m_changesFromW.begin(),m_changesFromW.end(),sort_chFW);
			seqNum->high = 0;
			seqNum->low = 0;
			//We check the rest for the largest one with Status Received or lost
			for(std::vector<ChangeFromWriter_t>::iterator it=m_changesFromW.begin();it!=m_changesFromW.end();++it)
			{
				if(it->status == RECEIVED || it->status == LOST)
				{
					*seqNum = it->change->sequenceNumber;
					m_max_available_seqNum = it->change->sequenceNumber;
					m_hasMaxAvailableSeqNumChanged = false;
				}
				else
					break;
			}
		}
		else
			*seqNum = this->m_max_available_seqNum;
	}
	if(*seqNum<this->m_lastRemovedSeqNum)
	{
		*seqNum = this->m_lastRemovedSeqNum;
		m_max_available_seqNum = this->m_lastRemovedSeqNum;
		m_hasMaxAvailableSeqNumChanged = false;
	}
	return true;
}


bool WriterProxy::available_changes_min(SequenceNumber_t* seqNum)
{
	if(!m_changesFromW.empty())
	{
		boost::lock_guard<WriterProxy> guard(*this);
		if(m_hasMinAvailableSeqNumChanged)
		{
			//Order changesFromWriter
			std::sort(m_changesFromW.begin(),m_changesFromW.end(),sort_chFW);
			seqNum->high = 0;
			seqNum->low = 0;
			for(std::vector<ChangeFromWriter_t>::iterator it=m_changesFromW.begin();it!=m_changesFromW.end();++it)
			{
				if(it->status == RECEIVED)
				{
					*seqNum = it->change->sequenceNumber;
					this->m_min_available_seqNum = it->change->sequenceNumber;
					m_hasMinAvailableSeqNumChanged = false;
					return true;
				}
				else if(it->status == LOST)
				{
					continue;
				}
				else
				{
					return false;
				}
			}
		}
		else
		{
			*seqNum = this->m_min_available_seqNum;
			return true;
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
	//	cout << "Calculating max of: ";
//		for(std::vector<ChangeFromWriter_t>::iterator it=m_changesFromW.begin();it!=m_changesFromW.end();++it)
//			cout << it->change->sequenceNumber.low << " ";
//		cout << endl;
		std::vector<ChangeFromWriter_t>::iterator it = m_changesFromW.end()-1;
		seq = it->change->sequenceNumber;
	}
	else
	{
		seq = this->m_lastRemovedSeqNum;
	}
	return seq;
}


bool WriterProxy::add_unknown_changes(SequenceNumber_t& seqin)
{
	SequenceNumber_t seq(seqin);
	boost::lock_guard<WriterProxy> guard(*this);
	uint32_t n_to_add;
	SequenceNumber_t maxseqNum(max_seq_num());

	if((maxseqNum+1).to64long() < (seq).to64long()) //if the maximum plus one is less than our seqNum we need to add
	{
		n_to_add = (uint32_t)(seq.to64long() - maxseqNum.to64long() -1);
		while(n_to_add>0)
		{
			CacheChange_t* ch = mp_SFR->reserve_Cache();
			ch->sequenceNumber = seq-n_to_add;
			ChangeFromWriter_t chfw;
			chfw.change = ch;
			chfw.status = UNKNOWN;
			chfw.is_relevant = false;
			pDebugInfo("WriterProxy:add_unknown_changes: "<<chfw.change->sequenceNumber.to64long()<<endl);
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
					m_hasMinAvailableSeqNumChanged = true;
					pDebugInfo("WriterProxy: removeChangeFromWriter: "<<m_lastRemovedSeqNum.to64long()<<endl);
					return true;
				}
				else
				{
					pDebugInfo("WriterProxy: removeChangeFromWriter: "<<it->change->sequenceNumber.to64long()<< " FALSE " <<endl);
					return false;
				}
			}
		}
	}
	return false;
}


} /* namespace rtps */
} /* namespace eprosima */
