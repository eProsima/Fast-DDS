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


WriterProxy_t::WriterProxy_t():
					remoteWriterGuid(c_Guid_Unknown),
					leaseDuration(c_TimeInfinite),
					livelinessKind(AUTOMATIC_LIVELINESS_QOS),
					ownershipStrength(0)

{

}

WriterProxy_t::~WriterProxy_t()
{
}

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
		m_livelinessAsserted(true),
		m_firstReceived(true)

{
	m_changesFromW.clear();
	Time_t aux;
	TIME_INFINITE(aux);
	if(WPparam.leaseDuration < aux)
		m_writerProxyLiveliness.restart_timer();
}

bool WriterProxy::missing_changes_update(SequenceNumber_t& seqNum)
{
	pDebugInfo("WriterProxy "<<param.remoteWriterGuid.entityId<<": MISSING_changes_update: up to seqNum: "<<seqNum.to64long()<<endl);
	boost::lock_guard<WriterProxy> guard(*this);
//	SequenceNumber_t seq = (seqNum)+1;
//	add_unknown_changes(seq);
	add_changes_from_writer_up_to(seqNum);

	for(std::vector<ChangeFromWriter_t>::iterator cit=m_changesFromW.begin();cit!=m_changesFromW.end();++cit)
	{
		if(cit->status == MISSING)
			m_isMissingChangesEmpty = false;
		if(cit->status == UNKNOWN)
		{
			if(cit->seqNum <= seqNum)
			{
				cit->status = MISSING;
				m_isMissingChangesEmpty = false;
			}
		}

	}
	m_hasMaxAvailableSeqNumChanged = true;
	m_hasMinAvailableSeqNumChanged = true;
	print_changes_fromWriter_test2();
	return true;
}

bool WriterProxy::add_changes_from_writer_up_to(SequenceNumber_t seq)
{
	SequenceNumber_t firstSN;
	if(m_changesFromW.size()==0)
		firstSN = this->m_lastRemovedSeqNum;
	else
		firstSN = m_changesFromW.back().seqNum;
	while(1)
	{
		++firstSN;
		if(firstSN>seq)
		{
			break;
		}
		ChangeFromWriter_t chw;
		chw.seqNum = firstSN;
		chw.status = UNKNOWN;
		chw.is_relevant = true;
		pDebugInfo("WriterProxy:add_unknown_changes: "<<chw.seqNum.to64long()<<endl);
		m_changesFromW.push_back(chw);

	}
	return true;
}

bool WriterProxy::lost_changes_update(SequenceNumber_t& seqNum)
{
	pDebugInfo("WriterProxy "<<param.remoteWriterGuid.entityId<<": LOST_changes_update: up to seqNum: "<<seqNum.to64long()<<endl);
	boost::lock_guard<WriterProxy> guard(*this);
//	SequenceNumber_t seq = (seqNum)+1;
//	add_unknown_changes(seq);
	add_changes_from_writer_up_to(seqNum);

	for(std::vector<ChangeFromWriter_t>::iterator cit=m_changesFromW.begin();cit!=m_changesFromW.end();++cit)
	{
		if(cit->status == UNKNOWN || cit->status == MISSING)
		{
			if(cit->seqNum < seqNum)
				cit->status = LOST;
		}
	}
	m_hasMaxAvailableSeqNumChanged = true;
	m_hasMinAvailableSeqNumChanged = true;
	print_changes_fromWriter_test2();
	return true;
}

bool WriterProxy::received_change_set(CacheChange_t* change)
{
	pDebugInfo("WriterProxy "<<param.remoteWriterGuid.entityId<<": RECEIVED_changes_set: change with seqNum: "<<change->sequenceNumber.to64long()<<endl);
	boost::lock_guard<WriterProxy> guard(*this);
	m_hasMaxAvailableSeqNumChanged = true;
	m_hasMinAvailableSeqNumChanged = true;
	if(!this->m_firstReceived || m_changesFromW.size()>0)
	{
		add_changes_from_writer_up_to(change->sequenceNumber);
		m_firstReceived = false;
	}
	else
	{
		ChangeFromWriter_t chfw;
		chfw.setChange(change);
		chfw.status = RECEIVED;
		chfw.is_relevant = true;
		m_changesFromW.push_back(chfw);
		m_firstReceived = false;
		print_changes_fromWriter_test2();
		return true;
	}
	for(std::vector<ChangeFromWriter_t>::reverse_iterator cit=m_changesFromW.rbegin();cit!=m_changesFromW.rend();++cit)
	{
		if(cit->seqNum == change->sequenceNumber)
		{
			cit->setChange(change);
			cit->status = RECEIVED;
			print_changes_fromWriter_test2();
			return true;
		}
	}
	pError("WriterProxy received change set: something has gone wrong"<<endl;);
	return false;
//	ChangeFromWriter_t chfw;
//	chfw.setChange(change);
//	chfw.is_relevant = true;
//	chfw.status = RECEIVED;
//
//	add_unknown_changes(change->sequenceNumber);
//
//	m_changesFromW.push_back(chfw);
//
//	return true;
}

bool WriterProxy::irrelevant_change_set(SequenceNumber_t& seqNum)
{
	boost::lock_guard<WriterProxy> guard(*this);
	m_hasMaxAvailableSeqNumChanged = true;
	m_hasMinAvailableSeqNumChanged = true;
	add_changes_from_writer_up_to(seqNum);
	for(std::vector<ChangeFromWriter_t>::reverse_iterator cit=m_changesFromW.rbegin();cit!=m_changesFromW.rend();++cit)
	{
		if(cit->seqNum == seqNum)
		{
			cit->status = RECEIVED;
			cit->is_relevant = false;
			print_changes_fromWriter_test2();
			return true;
		}
	}
	pError("WriterProxy irrelevant change set: something has gone wrong"<<endl;);
	return false;
//	ChangeFromWriter_t chfw;
//	chfw.is_relevant = false;
//	chfw.status = RECEIVED;
////	CacheChange_t* ch = mp_SFR->reserve_Cache();
//	//ch->sequenceNumber = seqNum;
//	chfw.seqNum = seqNum;
//	add_unknown_changes(seqNum);
//	m_changesFromW.push_back(chfw);

	return true;
}

//bool sort_chFW (ChangeFromWriter_t c1,ChangeFromWriter_t c2)
//{
//	return(c1.change->sequenceNumber.to64long() < c2.change->sequenceNumber.to64long());
//}


bool WriterProxy::missing_changes(std::vector<ChangeFromWriter_t*>* missing)
{
	if(!m_changesFromW.empty())
	{
		boost::lock_guard<WriterProxy> guard(*this);
		missing->clear();
		for(std::vector<ChangeFromWriter_t>::iterator it=m_changesFromW.begin();it!=m_changesFromW.end();++it)
		{
			if(it->status == MISSING && it->is_relevant)
				missing->push_back(&(*it));
		}
		if(missing->empty())
			m_isMissingChangesEmpty = true;
		print_changes_fromWriter_test2();
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
		//print_changes_fromWriter_test();
		if(m_hasMaxAvailableSeqNumChanged)
		{
			//Order changesFromWriter
			//std::sort(m_changesFromW.begin(),m_changesFromW.end(),sort_chFW);
			seqNum->high = 0;
			seqNum->low = 0;
			//We check the rest for the largest one with Status Received or lost
			//ignoring the first one that are not valid.
	//		bool first_ones = true;
			for(std::vector<ChangeFromWriter_t>::iterator it=m_changesFromW.begin();it!=m_changesFromW.end();++it)
			{
//				if(!it->isValid() && first_ones)
//				{
//					continue;
//				}
				if((it->status == RECEIVED || it->status == LOST))
				{
				//	first_ones = false;
					*seqNum = it->seqNum;
					m_max_available_seqNum = it->seqNum;
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
		//	std::sort(m_changesFromW.begin(),m_changesFromW.end(),sort_chFW);
			seqNum->high = 0;
			seqNum->low = 0;
			for(std::vector<ChangeFromWriter_t>::iterator it=m_changesFromW.begin();it!=m_changesFromW.end();++it)
			{
				if(it->status == RECEIVED)
				{
					*seqNum = it->seqNum;
					this->m_min_available_seqNum = it->seqNum;
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


void WriterProxy::print_changes_fromWriter_test2()
{
	std::stringstream ss;
	ss << "WP "<<this->param.remoteWriterGuid.entityId<<": ";
	for(std::vector<ChangeFromWriter_t>::iterator it=m_changesFromW.begin();it!=m_changesFromW.end();++it)
	{
		ss << it->seqNum.to64long()<<"("<<it->isValid()<<","<<it->status<<")-";
	}
	std::string auxstr = ss.str();
	pDebugInfo(auxstr<<endl;);
}

bool WriterProxy::removeChangesFromWriterUpTo(SequenceNumber_t& seq)
{
	for(std::vector<ChangeFromWriter_t>::iterator it=m_changesFromW.begin();it!=m_changesFromW.end();++it)
	{
		if(it->seqNum < seq)
		{
			if(it->status == RECEIVED || it->status == LOST)
			{
				m_lastRemovedSeqNum = it->seqNum;
				m_changesFromW.erase(it);
				m_hasMinAvailableSeqNumChanged = true;
			}
		}
		else if(it->seqNum == seq)
		{

			if(it->status == RECEIVED || it->status == LOST)
			{
				m_lastRemovedSeqNum = it->seqNum;
				m_changesFromW.erase(it);
				m_hasMinAvailableSeqNumChanged = true;
				pDebugInfo("WriterProxy: removeChangeFromWriter: "<<m_lastRemovedSeqNum.to64long()<<endl);
				return true;
			}
			else
			{
				pDebugInfo("WriterProxy: removeChangeFromWriterUpTo: "<<it->seqNum.to64long()<< " FALSE " <<endl);
				return false;
			}

		}
	}
	return false;
}


} /* namespace rtps */
} /* namespace eprosima */
