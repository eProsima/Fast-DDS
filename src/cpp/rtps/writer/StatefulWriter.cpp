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
 */

#include "eprosimartps/rtps/writer/StatefulWriter.h"
#include "eprosimartps/rtps/writer/ReaderProxy.h"

#include "eprosimartps/rtps/participant/RTPSParticipantImpl.h"

#include "eprosimartps/rtps/messages/RTPSMessageCreator.h"

#include "eprosimartps/rtps/resources/ResourceSend.h"

#include "eprosimartps/utils/TimeConversion.h"

#include "eprosimartps/rtps/writer/timedevent/UnsentChangesNotEmptyEvent.h"
#include "eprosimartps/rtps/writer/timedevent/PeriodicHeartbeat.h"
#include "eprosimartps/rtps/writer/timedevent/NackSupressionDuration.h"
#include "eprosimartps/rtps/writer/timedevent/NackResponseDelay.h"

#include "eprosimartps/rtps/history/WriterHistory.h"

#include "eprosimartps/utils/RTPSLog.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "StatefulWriter";

StatefulWriter::~StatefulWriter()
{
	const char* const METHOD_NAME = "~StatefulWriter";
	logInfo(RTPS_HISTORY,"StatefulWriter destructor");
	if(mp_periodicHB !=nullptr)
		delete(mp_periodicHB);
	for(std::vector<ReaderProxy*>::iterator it = matched_readers.begin();
			it!=matched_readers.end();++it)
	{
		delete(*it);
	}
}

StatefulWriter::StatefulWriter(RTPSParticipantImpl* pimpl,GUID_t& guid,
		WriterAttributes& att,WriterHistory* hist,WriterListener* listen):
		RTPSWriter(pimpl,guid,att,hist,listen),
		mp_periodicHB(nullptr),
		m_times(att.times)
{
	m_heartbeatCount = 0;
	if(guid.entityId == c_EntityId_SEDPPubWriter)
		m_HBReaderEntityId = c_EntityId_SEDPPubReader;
	else if(guid.entityId == c_EntityId_SEDPSubWriter)
		m_HBReaderEntityId = c_EntityId_SEDPSubReader;
	else if(guid.entityId == c_EntityId_WriterLiveliness)
		m_HBReaderEntityId= c_EntityId_ReaderLiveliness;
	else
		m_HBReaderEntityId = c_EntityId_Unknown;
}

/*
 *	CHANGE-RELATED METHODS
 */

void StatefulWriter::unsent_change_add(CacheChange_t* change)
{
	const char* const METHOD_NAME = "unsent_change_add";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	if(!matched_readers.empty())
	{
		for(auto it=matched_readers.begin();it!=matched_readers.end();++it)
		{
			ChangeForReader_t changeForReader;
			changeForReader.setChange(change);
			if(m_pushMode)
				changeForReader.status = UNSENT;
			else
				changeForReader.status = UNACKNOWLEDGED;
			changeForReader.is_relevant = (*it)->pubsub_is_relevant(change);
			(*it)->m_changesForReader.push_back(changeForReader);
		}
		unsent_changes_not_empty();
	}
	else
	{
		logInfo(RTPS_HISTORY,"No reader proxy to add change.");
	}
}


bool StatefulWriter::change_removed_by_history(CacheChange_t* a_change)
{
	const char* const METHOD_NAME = "change_removed_by_history";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	logInfo(RTPS_HISTORY,"Change "<<a_change->sequenceNumber.to64long()<< " to be removed.");
	for(std::vector<ReaderProxy*>::iterator it = this->matched_readers.begin();
			it!=this->matched_readers.end();++it)
	{
		for(std::vector<ChangeForReader_t>::iterator chit = (*it)->m_changesForReader.begin();
				chit!=(*it)->m_changesForReader.end();++chit)
		{
			if(chit->seqNum == a_change->sequenceNumber)
			{
				chit->notValid();
			}
		}
	}
	return true;
}



void StatefulWriter::unsent_changes_not_empty()
{
	const char* const METHOD_NAME = "unsent_changes_not_empty";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	std::vector<ReaderProxy*>::iterator rit;
	boost::lock_guard<boost::recursive_mutex> guard2(*this->getRTPSParticipant()->getSendMutex());
	for(rit=matched_readers.begin();rit!=matched_readers.end();++rit)
	{
		boost::lock_guard<boost::recursive_mutex> guard(*(*rit)->mp_mutex);
		std::vector<ChangeForReader_t*> ch_vec;
		if((*rit)->unsent_changes(&ch_vec))
		{
			std::vector<CacheChange_t*> relevant_changes;
			std::vector<SequenceNumber_t> not_relevant_changes;
			std::vector<ChangeForReader_t*>::iterator cit;
			for(cit = ch_vec.begin();cit!=ch_vec.end();++cit)
			{
				(*cit)->status = UNDERWAY;
				if((*cit)->is_relevant && (*cit)->isValid())
				{
					relevant_changes.push_back((*cit)->getChange());
				}
				else
				{
					not_relevant_changes.push_back((*cit)->seqNum);
				}
			}
			if(m_pushMode)
			{
				if(!relevant_changes.empty())
					RTPSMessageGroup::send_Changes_AsData(&m_cdrmessages,(RTPSWriter*)this,
							&relevant_changes,
							(*rit)->m_att.endpoint.unicastLocatorList,
							(*rit)->m_att.endpoint.multicastLocatorList,
							(*rit)->m_att.expectsInlineQos,
							(*rit)->m_att.guid.entityId);
				if(!not_relevant_changes.empty())
					RTPSMessageGroup::send_Changes_AsGap(&m_cdrmessages,(RTPSWriter*)this,
							&not_relevant_changes,
							(*rit)->m_att.guid.entityId,
							&(*rit)->m_att.endpoint.unicastLocatorList,
							&(*rit)->m_att.endpoint.multicastLocatorList);
				if((*rit)->m_att.endpoint.reliabilityKind == RELIABLE)
				{
					this->mp_periodicHB->restart_timer();
				}
				(*rit)->mp_nackSupression->restart_timer();
			}
			else
			{
				CacheChange_t* first;
				CacheChange_t* last;
				mp_history->get_min_change(&first);
				mp_history->get_max_change(&last);
				if(first->sequenceNumber.to64long()>0 && last->sequenceNumber.to64long() >= first->sequenceNumber.to64long()  )
				{
				incrementHBCount();
				CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_fullmsg);
				RTPSMessageCreator::addMessageHeartbeat(&m_cdrmessages.m_rtpsmsg_fullmsg,m_guid.guidPrefix,
						c_EntityId_Unknown,m_guid.entityId,first->sequenceNumber,last->sequenceNumber,m_heartbeatCount,true,false);
				std::vector<Locator_t>::iterator lit;
				for(lit = (*rit)->m_att.endpoint.unicastLocatorList.begin();lit!=(*rit)->m_att.endpoint.unicastLocatorList.end();++lit)
					getRTPSParticipant()->sendSync(&m_cdrmessages.m_rtpsmsg_fullmsg,(*lit));
				for(lit = (*rit)->m_att.endpoint.multicastLocatorList.begin();lit!=(*rit)->m_att.endpoint.multicastLocatorList.end();++lit)
					getRTPSParticipant()->sendSync(&m_cdrmessages.m_rtpsmsg_fullmsg,(*lit));
				}
			}
		}
	}
	logInfo(RTPS_HISTORY,"Finish sending unsent changes");
}

/*
 *	MATCHED_READER-RELATED METHODS
 */

bool StatefulWriter::matched_reader_add(RemoteReaderAttributes& rdata)
{
	const char* const METHOD_NAME = "matched_reader_add";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	if(rdata.guid == c_Guid_Unknown)
	{
		logError(RTPS_WRITER,"Reliable Writer need GUID_t of matched readers");
		return false;
	}
	for(std::vector<ReaderProxy*>::iterator it=matched_readers.begin();it!=matched_readers.end();++it)
	{
		if((*it)->m_att.guid == rdata.guid)
		{
			logWarning(RTPS_HISTORY,"Attempting to add existing reader" << endl);
			return false;
		}
	}
	ReaderProxy* rp = new ReaderProxy(rdata,m_times,this);
	if(mp_periodicHB==nullptr)
		mp_periodicHB = new PeriodicHeartbeat(this,TimeConv::Time_t2MilliSecondsDouble(m_times.heartbeatPeriod));
	if(rp->m_att.endpoint.durabilityKind >= TRANSIENT_LOCAL)
	{
		for(std::vector<CacheChange_t*>::iterator cit=mp_history->changesBegin();
				cit!=mp_history->changesEnd();++cit)
		{
			ChangeForReader_t changeForReader;
			changeForReader.setChange(*cit);
			changeForReader.is_relevant = rp->pubsub_is_relevant(*cit);

			if(m_pushMode)
				changeForReader.status = UNSENT;
			else
				changeForReader.status = UNACKNOWLEDGED;
			rp->m_changesForReader.push_back(changeForReader);
		}
	}
	matched_readers.push_back(rp);
	logInfo(RTPS_HISTORY,"Reader Proxy added to StatefulWriter with "
			<<rp->m_att.endpoint.unicastLocatorList.size()<<"(u)-"
			<<rp->m_att.endpoint.multicastLocatorList.size()<<"(m) locators: "<<rp->m_att.guid);
	if(rp->m_changesForReader.size()>0)
	{
		//unsent_changes_not_empty();
		this->mp_unsetChangesNotEmpty = new UnsentChangesNotEmptyEvent(this,1.0);
		this->mp_unsetChangesNotEmpty->restart_timer();
		this->mp_unsetChangesNotEmpty = nullptr;
	}
	return true;
}

bool StatefulWriter::matched_reader_remove(RemoteReaderAttributes& rdata)
{
	const char* const METHOD_NAME = "matched_reader_remove";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	for(std::vector<ReaderProxy*>::iterator it=matched_readers.begin();it!=matched_readers.end();++it)
	{
		if((*it)->m_att.guid == rdata.guid)
		{
			logInfo(RTPS_HISTORY,"Reader Proxy removed: " <<(*it)->m_att.guid);
			delete(*it);
			matched_readers.erase(it);
			if(matched_readers.size()==0)
				this->mp_periodicHB->stop_timer();
			return true;
		}
	}
	logInfo(RTPS_HISTORY,"Reader Proxy doesn't exist in this writer")
	return false;
}

bool StatefulWriter::matched_reader_is_matched(RemoteReaderAttributes& rdata)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	for(std::vector<ReaderProxy*>::iterator it=matched_readers.begin();it!=matched_readers.end();++it)
	{
		if((*it)->m_att.guid == rdata.guid)
		{
			return true;
		}
	}
	return false;
}

bool StatefulWriter::matched_reader_lookup(GUID_t& readerGuid,ReaderProxy** RP)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	std::vector<ReaderProxy*>::iterator it;
	for(it=matched_readers.begin();it!=matched_readers.end();++it)
	{
		if((*it)->m_att.guid == readerGuid)
		{
			*RP = *it;
			return true;
		}
	}
	return false;
}

bool StatefulWriter::is_acked_by_all(CacheChange_t* change)
{
	const char* const METHOD_NAME = "is_acked_by_all";
	if(change->writerGUID != this->getGuid())
	{
		logWarning(RTPS_WRITER,"The given change is not from this Writer");
		return false;
	}
	std::vector<ReaderProxy*>::iterator it;
	for(it=matched_readers.begin();it!=matched_readers.end();++it)
	{
		ChangeForReader_t changeForReader;
		if((*it)->getChangeForReader(change,&changeForReader))
		{
			if(changeForReader.is_relevant)
			{
				if(changeForReader.status != ACKNOWLEDGED)
				{
					logInfo(RTPS_HISTORY,"Change not acked. Relevant: " << changeForReader.is_relevant<<" status: " << changeForReader.status << endl);
					return false;
				}
			}
		}
	}
	return true;
}


//
//bool sort_changeForReader_ptr (ChangeForReader_t* c1,ChangeForReader_t* c2)
//{
//	return(c1->seqNum < c2->seqNum);
//}
//
//bool sort_changeForReader(ChangeForReader_t c1,ChangeForReader_t c2)
//{
//	return(c1.seqNum < c2.seqNum);
//}
//
//bool sort_changes (CacheChange_t* c1,CacheChange_t* c2)
//{
//	return(c1->sequenceNumber.to64long() < c2->sequenceNumber.to64long());
//}
//
//
//
//
//bool StatefulWriter::removeMinSeqCacheChange()
//{
//	const char* const METHOD_NAME = "removeMinSeqCacheChange";
//	logInfo(RTPS_HISTORY,"Removing min seq from StatefulWriter");
//	CacheChange_t* change;
//	if(mp_history->get_min_change(&change))
//	{
//		if(is_acked_by_all(change))
//		{
//			for(std::vector<ReaderProxy*>::iterator it = this->matched_readers.begin();
//					it!=this->matched_readers.end();++it)
//			{
//				if(!(*it)->m_changesForReader.empty())
//					(*it)->m_changesForReader.erase((*it)->m_changesForReader.begin());
//			}
//			mp_history->remove_min_change();
//			return true;
//		}
//	}
//	return false;
//}
//
//bool StatefulWriter::removeAllCacheChange(size_t* removed)
//{
//	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
//	int32_t n_count = 0;
//	while(this->removeMinSeqCacheChange())
//	{
//		n_count++;
//	}
//	*removed = n_count;
//	if(this->mp_history->getHistorySize()==0)
//		return true;
//	else
//		return false;
//}
//
//

/*
 * PARAMETER_RELATED METHODS
 */

void StatefulWriter::updateAttributes(WriterAttributes& att)
{
	cout << "TO FINISH"<<endl;
	this->updateTimes(att.times);
}

void StatefulWriter::updateTimes(WriterTimes& times)
{
	if(m_times.heartbeatPeriod != times.heartbeatPeriod)
	{
		this->mp_periodicHB->update_interval(times.heartbeatPeriod);
	}
	if(m_times.nackResponseDelay != times.nackResponseDelay)
	{
		for(std::vector<ReaderProxy*>::iterator it = this->matched_readers.begin();
				it!=this->matched_readers.end();++it)
		{
			(*it)->mp_nackResponse->update_interval(times.nackResponseDelay);
		}
	}
	if(m_times.nackSupressionDuration != times.nackSupressionDuration)
	{
		for(std::vector<ReaderProxy*>::iterator it = this->matched_readers.begin();
				it!=this->matched_readers.end();++it)
		{
			(*it)->mp_nackSupression->update_interval(times.nackSupressionDuration);
		}
	}
	m_times = times;
}

} /* namespace rtps */
} /* namespace eprosima */
