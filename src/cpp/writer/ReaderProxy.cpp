/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderProxy.cpp
 *
 */


#include "eprosimartps/writer/ReaderProxy.h"
#include "eprosimartps/utils/RTPSLog.h"
#include "eprosimartps/writer/StatefulWriter.h"



namespace eprosima {
namespace rtps {



ReaderProxy::ReaderProxy(const ReaderProxy_t& RPparam,const PublisherTimes& times,StatefulWriter* SW):
				m_param(RPparam),
				mp_SFW(SW),
				m_isRequestedChangesEmpty(true),
				//m_periodicHB(this,boost::posix_time::milliseconds((int64_t)ceil(Time_t2MicroSec(times.heartbeatPeriod)*1e-3))),
				m_nackResponse(this,boost::posix_time::milliseconds((int64_t)ceil(Time_t2MicroSec(times.nackResponseDelay)*1e-3))),
				m_nackSupression(this,boost::posix_time::milliseconds((int64_t)ceil(Time_t2MicroSec(times.nackSupressionDuration)*1e-3))),
				m_lastAcknackCount(0)
{
//	cout << "PeriodicHB: "<< m_periodicHB.getIntervalMsec()<<endl;
//	cout << "m_nackResponse: "<< m_nackResponse.getIntervalMsec()<<endl;
//	cout << "m_nackSupression: "<< m_nackSupression.getIntervalMsec()<<endl;
//	cout << "PeriodicHB is waiting: "<< m_periodicHB.m_isWaiting << endl;

}


ReaderProxy::~ReaderProxy() {

}



bool ReaderProxy::getChangeForReader(CacheChange_t* change,
		ChangeForReader_t* changeForReader)
{
	boost::lock_guard<ReaderProxy> guard(*this);
	for(std::vector<ChangeForReader_t>::iterator it=m_changesForReader.begin();it!=m_changesForReader.end();++it)
	{
		if(it->seqNum == change->sequenceNumber)
		{
			*changeForReader = *it;
			pDebugInfo("Change found in Reader Proxy " << endl);
			return true;
		}
	}

	return false;
}


bool ReaderProxy::getChangeForReader(SequenceNumber_t& seq,ChangeForReader_t* changeForReader)
{
	boost::lock_guard<ReaderProxy> guard(*this);
	for(std::vector<ChangeForReader_t>::iterator it=m_changesForReader.begin();it!=m_changesForReader.end();++it)
	{
		if(it->seqNum == seq)
		{
			*changeForReader = *it;
			pDebugInfo("Change found in Reader Proxy " << endl);
			return true;
		}
	}

	return false;
}

bool ReaderProxy::acked_changes_set(SequenceNumber_t& seqNum)
{
	boost::lock_guard<ReaderProxy> guard(*this);

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
	boost::lock_guard<ReaderProxy> guard(*this);

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
	pDebugInfo("Requested Changes Set" << endl);
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
	boost::lock_guard<ReaderProxy> guard(*this);
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
	changesList->clear();
	boost::lock_guard<ReaderProxy> guard(*this);

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
	boost::lock_guard<ReaderProxy> guard(*this);
	*changeForReader = **std::min_element(Changes->begin(),Changes->end(),change_min);
	return true;
}

bool ReaderProxy::dds_is_relevant(CacheChange_t* change)
{
	return true;
}

} /* namespace rtps */
} /* namespace eprosima */


