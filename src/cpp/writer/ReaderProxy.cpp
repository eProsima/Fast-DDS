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
 *  Created on: Mar 17, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/writer/ReaderProxy.h"
#include "eprosimartps/writer/StatefulWriter.h"

namespace eprosima {
namespace rtps {



ReaderProxy::ReaderProxy(ReaderProxy_t* RPparam,StatefulWriter* SW):
				mp_SFW(SW),
				m_param(*RPparam),
				m_isRequestedChangesEmpty(true),
				m_periodicHB(this,boost::posix_time::milliseconds(SW->reliability.heartbeatPeriod.to64time()*1000)),
				m_nackResponse(this,boost::posix_time::milliseconds(SW->reliability.nackResponseDelay.to64time()*1000)),
				m_nackSupression(this,boost::posix_time::milliseconds(SW->reliability.nackSupressionDuration.to64time()*1000)),
				m_lastAcknackCount(0)
{

}


ReaderProxy::~ReaderProxy() {
	pDebugInfo("ReaderProxy destructor"<<endl;);
}



bool ReaderProxy::getChangeForReader(CacheChange_t* change,
		ChangeForReader_t* changeForReader)
{
	boost::lock_guard<ReaderProxy> guard(*this);
	for(std::vector<ChangeForReader_t>::iterator it=m_changesForReader.begin();it!=m_changesForReader.end();++it)
	{
		if(it->change->sequenceNumber.to64long() == change->sequenceNumber.to64long()
				&& it->change->writerGUID == change->writerGUID)
		{
			changeForReader->is_relevant = it->is_relevant;
			changeForReader->status = it->status;
			changeForReader->change = it->change;
			pDebugInfo("Change found in Reader Proxy " << endl);
			return true;
		}
	}

	return false;
}

bool ReaderProxy::acked_changes_set(SequenceNumber_t* seqNum)
{
	boost::lock_guard<ReaderProxy> guard(*this);

	for(std::vector<ChangeForReader_t>::iterator it=m_changesForReader.begin();it!=m_changesForReader.end();++it)
	{
		if(it->change->sequenceNumber.to64long() < seqNum->to64long())
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
			if(it->change->sequenceNumber.to64long() == sit->to64long())
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
	return ch1->change->sequenceNumber.to64long() < ch2->change->sequenceNumber.to64long();
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


