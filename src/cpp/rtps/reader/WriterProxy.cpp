/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterProxy.cpp
 *
 */

#include <fastrtps/rtps/reader/WriterProxy.h>
#include <fastrtps/rtps/reader/StatefulReader.h>

#include <fastrtps/utils/RTPSLog.h>
#include <fastrtps/utils/TimeConversion.h>

#include <boost/thread/lock_guard.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <fastrtps/rtps/reader/timedevent/HeartbeatResponseDelay.h>
#include <fastrtps/rtps/reader/timedevent/WriterProxyLiveliness.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const int WRITERPROXY_LIVELINESS_PERIOD_MULTIPLIER = 1;

static const char* const CLASS_NAME = "WriterProxy";

WriterProxy::~WriterProxy()
{
	//pDebugInfo("WriterProxy destructor"<<endl;);
	if(mp_writerProxyLiveliness!=nullptr)
		delete(mp_writerProxyLiveliness);

	delete(mp_heartbeatResponse);
	delete(mp_mutex);
}

WriterProxy::WriterProxy(RemoteWriterAttributes& watt,
		Duration_t /*heartbeatResponse*/,
		StatefulReader* SR) :
												mp_SFR(SR),
												m_att(watt),
												m_acknackCount(0),
												m_nackfragCount(0),
												m_lastHeartbeatCount(0),
												m_isMissingChangesEmpty(true),
												mp_heartbeatResponse(nullptr),
												mp_writerProxyLiveliness(nullptr),
												m_heartbeatFinalFlag(false),
												m_hasMinAvailableSeqNumChanged(false),
												m_hasMaxAvailableSeqNumChanged(false),
												m_isAlive(true),
												m_firstReceived(true),
												mp_mutex(new boost::recursive_mutex())

{
	const char* const METHOD_NAME = "WriterProxy";
	m_changesFromW.clear();
	//Create Events
	mp_writerProxyLiveliness = new WriterProxyLiveliness(this,TimeConv::Time_t2MilliSecondsDouble(m_att.livelinessLeaseDuration)*WRITERPROXY_LIVELINESS_PERIOD_MULTIPLIER);
	mp_heartbeatResponse = new HeartbeatResponseDelay(this,TimeConv::Time_t2MilliSecondsDouble(mp_SFR->getTimes().heartbeatResponseDelay));
	if(m_att.livelinessLeaseDuration < c_TimeInfinite)
		mp_writerProxyLiveliness->restart_timer();
	logInfo(RTPS_READER,"Writer Proxy created in reader: "<<mp_SFR->getGuid().entityId);
}

bool WriterProxy::missing_changes_update(SequenceNumber_t& seqNum)
{
	const char* const METHOD_NAME = "missing_changes_update";
	logInfo(RTPS_READER,m_att.guid.entityId<<": changes up to seqNum: " << seqNum <<" missing.");
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
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
	const char* const METHOD_NAME = "add_changes_from_writer_up_to";
	SequenceNumber_t firstSN;
	if(m_changesFromW.size()==0)
	{
		if(this->m_lastRemovedSeqNum > SequenceNumber_t(0,0))
			firstSN = this->m_lastRemovedSeqNum;
		else
			firstSN = seq-1;
	}
	else
		firstSN = m_changesFromW.back().seqNum;

    ++firstSN;
    while(firstSN <= seq)
	{
		ChangeFromWriter_t chw;
		chw.seqNum = firstSN;
		chw.status = UNKNOWN;
		chw.is_relevant = true;
		logInfo(RTPS_READER,"WP "<<this->m_att.guid << " adding unknown changes up to: " << chw.seqNum);
		m_changesFromW.push_back(chw);
        ++firstSN;
	}

	return true;
}

bool WriterProxy::lost_changes_update(SequenceNumber_t& seqNum)
{
	const char* const METHOD_NAME = "lost_changes_update";
	logInfo(RTPS_READER,m_att.guid.entityId<<": up to seqNum: "<<seqNum);
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
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
	const char* const METHOD_NAME = "received_change_set";
	logInfo(RTPS_READER,m_att.guid.entityId<<": seqNum: " << change->sequenceNumber);
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
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
	logError(RTPS_READER," Something has gone wrong";);
	return false;

}

bool WriterProxy::irrelevant_change_set(SequenceNumber_t& seqNum)
{
	const char* const METHOD_NAME = "irrelevant_change_set";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
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
	logError(RTPS_READER,"Something has gone wrong"<<endl;);
	return false;
}


bool WriterProxy::missing_changes(std::vector<ChangeFromWriter_t*>* missing)
{
	//const char* const METHOD_NAME = "missing_changes";
	if(!m_changesFromW.empty())
	{
		boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
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
	//const char* const METHOD_NAME = "available_changes_max";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	//print_changes_fromWriter_test();
	if(this->m_lastRemovedSeqNum <= SequenceNumber_t(0,0) && m_changesFromW.size() == 0) //NOT RECEIVED ANYTHING
    {
		return false;
    }
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
	//const char* const METHOD_NAME = "available_changes_min";
	if(this->m_lastRemovedSeqNum <= SequenceNumber_t(0, 0) && m_changesFromW.size() == 0) //NOT RECEIVED ANYTHING
		return false;
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
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
                if(it->isValid())
                {
                    *seqNum = it->seqNum;
                    this->m_min_available_seqNum = it->seqNum;
                    m_hasMinAvailableSeqNumChanged = false;
                    return true;
                }
                else
                    continue;
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
	}
	if(*seqNum<=this->m_lastRemovedSeqNum)
	{
		*seqNum = this->m_lastRemovedSeqNum;
		m_min_available_seqNum = this->m_lastRemovedSeqNum;
		m_hasMinAvailableSeqNumChanged = false;
		return false;
	}
	return true;
}


void WriterProxy::print_changes_fromWriter_test2()
{
	const char* const METHOD_NAME = "status";
	std::stringstream ss;
	ss << this->m_att.guid.entityId<<": ";
	for(std::vector<ChangeFromWriter_t>::iterator it=m_changesFromW.begin();it!=m_changesFromW.end();++it)
	{
		ss << it->seqNum <<"("<<it->isValid()<<","<<it->status<<")-";
	}
	std::string auxstr = ss.str();
	logInfo(RTPS_READER,auxstr;);
}

//bool WriterProxy::removeChangesFromWriterUpTo(SequenceNumber_t& seq)
//{
//	const char* const METHOD_NAME = "removeChangesFromWriterUpTo";
//	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
//	for(std::vector<ChangeFromWriter_t>::iterator it=m_changesFromW.begin();it!=m_changesFromW.end();++it)
//	{
//		if(it->seqNum < seq)
//		{
//			if(it->status == RECEIVED || it->status == LOST)
//			{
//				m_lastRemovedSeqNum = it->seqNum;
//				m_changesFromW.erase(it);
//				m_hasMinAvailableSeqNumChanged = true;
//			}
//		}
//		else if(it->seqNum == seq)
//		{
//
//			if(it->status == RECEIVED || it->status == LOST)
//			{
//				m_lastRemovedSeqNum = it->seqNum;
//				m_changesFromW.erase(it);
//				m_hasMinAvailableSeqNumChanged = true;
//				logInfo(RTPS_READER,m_lastRemovedSeqNum.to64long()<< " OK";);
//				return true;
//			}
//			else
//			{
//				logInfo(RTPS_READER,it->seqNum.to64long()<< " NOT OK";);
//				return false;
//			}
//
//		}
//	}
//	return false;
//}

bool WriterProxy::get_change(SequenceNumber_t& seq,CacheChange_t** change)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	for(std::vector<ChangeFromWriter_t>::iterator it = this->m_changesFromW.begin();
			it!= this->m_changesFromW.end();++it)
	{
		if(it->seqNum == seq && it->isValid())
		{
			*change = it->getChange();
			return true;
		}
	}
	return false;
}

void WriterProxy::assertLiveliness()
{
	const char* const METHOD_NAME = "assertLiveliness";

	logInfo(RTPS_READER,this->m_att.guid.entityId << " Liveliness asserted");

	//boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

	m_isAlive=true;

	this->mp_writerProxyLiveliness->cancel_timer();
	this->mp_writerProxyLiveliness->restart_timer();
}



}
} /* namespace rtps */
} /* namespace eprosima */
