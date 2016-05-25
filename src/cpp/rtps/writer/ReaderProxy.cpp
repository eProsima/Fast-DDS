/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.endl
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderProxy.cpp
 *
 */


#include <fastrtps/rtps/writer/ReaderProxy.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/utils/TimeConversion.h>
#include <fastrtps/rtps/writer/timedevent/NackResponseDelay.h>
#include <fastrtps/rtps/writer/timedevent/NackSupressionDuration.h>
#include <fastrtps/utils/RTPSLog.h>
#include <fastrtps/rtps/resources/AsyncWriterThread.h>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <cassert>

using namespace eprosima::fastrtps::rtps;

static const char* const CLASS_NAME = "ReaderProxy";

ReaderProxy::ReaderProxy(RemoteReaderAttributes& rdata,const WriterTimes& times,StatefulWriter* SW) :
				m_att(rdata), mp_SFW(SW), m_isRequestedChangesEmpty(true),
				mp_nackResponse(nullptr), mp_nackSupression(nullptr), m_lastAcknackCount(0),
				mp_mutex(new boost::recursive_mutex()), lastNackfragCount_(0)
{
	const char* const METHOD_NAME = "ReaderProxy";
	mp_nackResponse = new NackResponseDelay(this,TimeConv::Time_t2MilliSecondsDouble(times.nackResponseDelay));
	mp_nackSupression = new NackSupressionDuration(this,TimeConv::Time_t2MilliSecondsDouble(times.nackSupressionDuration));
	logInfo(RTPS_WRITER,"Reader Proxy created");
}


ReaderProxy::~ReaderProxy()
{
    if(mp_nackResponse != nullptr)
	    delete(mp_nackResponse);
    if(mp_nackSupression != nullptr)
	    delete(mp_nackSupression);
	delete(mp_mutex);
}

void ReaderProxy::destroy_timers()
{
    delete(mp_nackResponse);
    mp_nackResponse = nullptr;
    delete(mp_nackSupression);
    mp_nackSupression = nullptr;
}

void ReaderProxy::addChange(const ChangeForReader_t& change)
{
   m_changesForReader.insert(change);
   if (change.getStatus() == UNSENT)
      AsyncWriterThread::instance()->wakeUp();
}

size_t ReaderProxy::countChangesForReader() const
{
   return m_changesForReader.size();
}

bool ReaderProxy::getChangeForReader(const CacheChange_t* change,
		ChangeForReader_t* changeForReader)
{
	const char* const METHOD_NAME = "getChangeForReader";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
    auto chit = m_changesForReader.find(ChangeForReader_t(change));

    if(chit != m_changesForReader.end())
    {
        *changeForReader = *chit;
        logInfo(RTPS_WRITER,"Change " << change->sequenceNumber << " found in Reader Proxy ");
        return true;
    }

	return false;
}

bool ReaderProxy::getChangeForReader(const SequenceNumber_t& seqNum, ChangeForReader_t* changeForReader)
{
	const char* const METHOD_NAME = "getChangeForReader";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
    auto chit = m_changesForReader.find(ChangeForReader_t(seqNum));

    if(chit != m_changesForReader.end())
    {
        *changeForReader = *chit;
        logInfo(RTPS_WRITER,"Change " << seqNum <<" found in Reader Proxy ");
        return true;
    }

	return false;
}

//TODO Return value for what?
bool ReaderProxy::acked_changes_set(const SequenceNumber_t& seqNum)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    auto chit = m_changesForReader.begin();

    while(chit != m_changesForReader.end() && chit->getSequenceNumber() < seqNum)
    {
        chit = m_changesForReader.erase(chit);
    }

    return m_changesForReader.size() == 0;
}

bool ReaderProxy::requested_changes_set(std::vector<SequenceNumber_t>& seqNumSet)
{
	const char* const METHOD_NAME = "requested_changes_set";
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

	for(std::vector<SequenceNumber_t>::iterator sit=seqNumSet.begin();sit!=seqNumSet.end();++sit)
	{
        auto chit = m_changesForReader.find(ChangeForReader_t(*sit));

        if(chit != m_changesForReader.end())
        {
            ChangeForReader_t newch(*chit);
            newch.setStatus(REQUESTED);

            auto hint = m_changesForReader.erase(chit);

            m_changesForReader.insert(hint, newch);

            m_isRequestedChangesEmpty = false;
        }
	}

	if(!m_isRequestedChangesEmpty)
	{
		logInfo(RTPS_WRITER,"Requested Changes: " << seqNumSet);
	}
	return true;
}


std::vector<const ChangeForReader_t*> ReaderProxy::get_unsent_changes() const
{
   std::vector<const ChangeForReader_t*> unsent_changes;
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

   auto it = m_changesForReader.begin();
   for (; it!= m_changesForReader.end(); ++it)
		if(it->getStatus() == UNSENT)
         unsent_changes.push_back(&(*it));

    return unsent_changes;
}

std::vector<const ChangeForReader_t*> ReaderProxy::get_requested_changes() const
{
   std::vector<const ChangeForReader_t*> unsent_changes;
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

   auto it = m_changesForReader.begin();
   for (; it!= m_changesForReader.end(); ++it)
		if(it->getStatus() == REQUESTED)
         unsent_changes.push_back(&(*it));

    return unsent_changes;
}

void ReaderProxy::set_change_to_status(const CacheChange_t* change, ChangeForReaderStatus_t status)
{
   bool mustWakeUpAsyncThread = false; 
   for (auto it = m_changesForReader.begin(); it!= m_changesForReader.end(); ++it)
   {
      if (it->getChange() == change){
         ChangeForReader_t newch(*it);
         newch.setStatus(status);
         if (status == UNSENT) mustWakeUpAsyncThread = true;
         auto hint = m_changesForReader.erase(it);
         m_changesForReader.insert(hint, newch);
         break;
      }
   }

   if (mustWakeUpAsyncThread)
      AsyncWriterThread::instance()->wakeUp();
}

void ReaderProxy::convert_status_on_all_changes(ChangeForReaderStatus_t previous, ChangeForReaderStatus_t next)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
   bool mustWakeUpAsyncThread = false; 

    auto it = m_changesForReader.begin();
    while(it != m_changesForReader.end())
	{
		if(it->getStatus() == previous)
		{
            ChangeForReader_t newch(*it);
            newch.setStatus(next);
            if (next == UNSENT && previous != UNSENT)
               mustWakeUpAsyncThread = true;
            auto hint = m_changesForReader.erase(it);

            it = m_changesForReader.insert(hint, newch);
		}

        ++it;
	}

   if (mustWakeUpAsyncThread)
      AsyncWriterThread::instance()->wakeUp();
}

void ReaderProxy::setNotValid(const CacheChange_t* change)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    // Check sequence number is in the container, because it was not clean up.
    if(m_changesForReader.size() == 0 || change->sequenceNumber < m_changesForReader.begin()->getSequenceNumber())
        return;

    auto chit = m_changesForReader.find(ChangeForReader_t(change));

    // Element must be in the container. In other case, bug.
    assert(chit != m_changesForReader.end());

    if(chit == m_changesForReader.begin())
    {
        m_changesForReader.erase(chit);
        cleanup();
    }
    else
    {
        ChangeForReader_t newch(*chit);
        newch.notValid();

        auto hint = m_changesForReader.erase(chit);

        m_changesForReader.insert(hint, newch);
    }
}

void ReaderProxy::cleanup()
{
    auto chit = m_changesForReader.begin();

    while(chit != m_changesForReader.end() &&
            (!chit->isValid() || chit->getStatus() == ACKNOWLEDGED))
            chit = m_changesForReader.erase(chit);
}

bool ReaderProxy::thereIsUnacknowledged() const
{
    bool returnedValue = false;
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

	for(auto it = m_changesForReader.begin(); it!=m_changesForReader.end(); ++it)
	{
		if(it->getStatus() == UNACKNOWLEDGED)
		{
            returnedValue = true;
            break;
		}
	}

    return returnedValue;
}

bool change_min(const ChangeForReader_t* ch1, const ChangeForReader_t* ch2)
{
	return ch1->getSequenceNumber() < ch2->getSequenceNumber();
}

bool change_min2(const ChangeForReader_t ch1, const ChangeForReader_t ch2)
{
	return ch1.getSequenceNumber() < ch2.getSequenceNumber();
}

bool ReaderProxy::minChange(std::vector<ChangeForReader_t*>* Changes,
		ChangeForReader_t* changeForReader)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
	*changeForReader = **std::min_element(Changes->begin(),Changes->end(),change_min);
	return true;
}

bool ReaderProxy::requested_fragment_set(const SequenceNumber_t& sequence_number, const FragmentNumberSet_t& frag_set)
{
    std::set<FragmentNumber_t> requested_fragments(frag_set.get_begin(), frag_set.get_end());
    size_t requested_number = requested_fragments.size();

    auto ret = requested_fragments_.insert(std::make_pair(sequence_number, std::move(requested_fragments)));

    if(!ret.second)
    {
        ret.first->second.insert(requested_fragments.begin(), requested_fragments.end());
    }

    return requested_number > 0;
}
