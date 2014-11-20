/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.endl
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderProxy.cpp
 *
 */


#include "eprosimartps/rtps/writer/ReaderProxy.h"

#include "eprosimartps/rtps/writer/StatefulWriter.h"
#include "eprosimartps/utils/TimeConversion.h"

#include "eprosimartps/rtps/writer/timedevent/NackResponseDelay.h"
#include "eprosimartps/rtps/writer/timedevent/NackSupressionDuration.h"

#include "eprosimartps/utils/RTPSLog.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "ReaderProxy";

ReaderProxy::ReaderProxy(RemoteReaderAttributes& rdata,const WriterTimes& times,StatefulWriter* SW):
				m_att(rdata),
				mp_SFW(SW),
				m_isRequestedChangesEmpty(true),
				mp_nackResponse(nullptr),
				mp_nackSupression(nullptr),
				m_lastAcknackCount(0),
				mp_mutex(new boost::recursive_mutex())
{
	const char* const METHOD_NAME = "ReaderProxy";
	mp_nackResponse = new NackResponseDelay(this,TimeConv::Time_t2MilliSecondsDouble(times.nackResponseDelay));
	mp_nackSupression = new NackSupressionDuration(this,TimeConv::Time_t2MilliSecondsDouble(times.nackSupressionDuration));
	logInfo(RTPS_HISTORY,"Reader Proxy created");
}


ReaderProxy::~ReaderProxy()
{

}

bool ReaderProxy::getChangeForReader(CacheChange_t* change,
		ChangeForReader_t* changeForReader)
{
	const char* const METHOD_NAME = "getChangeForReader";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	for(std::vector<ChangeForReader_t>::iterator it=m_changesForReader.begin();it!=m_changesForReader.end();++it)
	{
		if(it->seqNum == change->sequenceNumber)
		{
			*changeForReader = *it;
			logInfo(RTPS_HISTORY,"Change " << change->sequenceNumber.to64long()<< " found in Reader Proxy " << endl);
			return true;
		}
	}

	return false;
}


bool ReaderProxy::getChangeForReader(SequenceNumber_t& seq,ChangeForReader_t* changeForReader)
{
	const char* const METHOD_NAME = "getChangeForReader";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	for(std::vector<ChangeForReader_t>::iterator it=m_changesForReader.begin();it!=m_changesForReader.end();++it)
	{
		if(it->seqNum == seq)
		{
			*changeForReader = *it;
			logInfo(RTPS_HISTORY,"Change " << seq.to64long()<<" found in Reader Proxy " << endl);
			return true;
		}
	}

	return false;
}

bool ReaderProxy::acked_changes_set(SequenceNumber_t& seqNum)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

	for(std::vector<ChangeForReader_t>::iterator it=m_changesForReader.begin();it!=m_changesForReader.end();++it)
	{
		if(it->seqNum < seqNum)
		{
			it->status = ACKNOWLEDGED;
		}
	}
	return true;
}

bool ReaderProxy::requested_changes_set(std::vector<SequenceNumber_t>& seqNumSet)
{
	const char* const METHOD_NAME = "requested_changes_set";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

	for(std::vector<SequenceNumber_t>::iterator sit=seqNumSet.begin();sit!=seqNumSet.end();++sit)
	{
		for(std::vector<ChangeForReader_t>::iterator it=m_changesForReader.begin();it!=m_changesForReader.end();++it)
		{
			if(it->seqNum == *sit)
			{
				it->status = REQUESTED;
				m_isRequestedChangesEmpty = false;
				break;
			}
		}
	}
	logInfo(RTPS_HISTORY,"Requested Changes Set" << endl);
	return true;
}


bool ReaderProxy::requested_changes(std::vector<ChangeForReader_t*>* Changes)
{
	return changesList(Changes,REQUESTED);
}

bool ReaderProxy::unsent_changes(std::vector<ChangeForReader_t*>* Changes)
{
	return changesList(Changes,UNSENT);
}

bool ReaderProxy::unacked_changes(std::vector<ChangeForReader_t*>* Changes)
{
	return changesList(Changes,UNACKNOWLEDGED);
}



bool ReaderProxy::next_requested_change(ChangeForReader_t* changeForReader)
{
	std::vector<ChangeForReader_t*> changesList;
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	if(requested_changes(&changesList))
	{
		return minChange(&changesList,changeForReader);
	}
	return false;
}

bool ReaderProxy::next_unsent_change(ChangeForReader_t* changeForReader)
{
	std::vector<ChangeForReader_t*> changesList;
	if(unsent_changes(&changesList))
	{
		return minChange(&changesList,changeForReader);
	}
	return false;
}

bool ReaderProxy::changesList(std::vector<ChangeForReader_t*>* changesList,
								ChangeForReaderStatus_t status)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	changesList->clear();
	for(std::vector<ChangeForReader_t>::iterator it=m_changesForReader.begin();it!=m_changesForReader.end();++it)
	{
		if(it->status == status)
		{
			changesList->push_back(&(*it));
		}
	}
	return true;
}

bool change_min(ChangeForReader_t* ch1,ChangeForReader_t* ch2)
{
	return ch1->seqNum < ch2->seqNum;
}

bool change_min2(ChangeForReader_t ch1,ChangeForReader_t ch2)
{
	return ch1.seqNum < ch2.seqNum;
}

bool ReaderProxy::max_acked_change(SequenceNumber_t* sn)
{
	if(!m_changesForReader.empty())
	{
		for(std::vector<ChangeForReader_t>::iterator it=m_changesForReader.begin();
				it!=m_changesForReader.end();++it)
		{
			if(it->status != ACKNOWLEDGED)
			{
				*sn = ((*it).seqNum-1);
				return true;
			}
		}
		*sn = (m_changesForReader.end()-1)->seqNum;
	}
	return false;
}


bool ReaderProxy::minChange(std::vector<ChangeForReader_t*>* Changes,
		ChangeForReader_t* changeForReader)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	*changeForReader = **std::min_element(Changes->begin(),Changes->end(),change_min);
	return true;
}

bool ReaderProxy::pubsub_is_relevant(CacheChange_t* change)
{
	return true;
}

} /* namespace rtps */
} /* namespace eprosima */


