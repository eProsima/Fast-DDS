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

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <cassert>

using namespace eprosima::fastrtps::rtps;

static const char* const CLASS_NAME = "ReaderProxy";

ReaderProxy::ReaderProxy(RemoteReaderAttributes& rdata,const WriterTimes& times,StatefulWriter* SW) :
				m_att(rdata), mp_SFW(SW), m_isRequestedChangesEmpty(true),
				mp_nackResponse(nullptr), mp_nackSupression(nullptr), m_lastAcknackCount(0),
				mp_mutex(new boost::recursive_mutex())
{
	const char* const METHOD_NAME = "ReaderProxy";
	mp_nackResponse = new NackResponseDelay(this,TimeConv::Time_t2MilliSecondsDouble(times.nackResponseDelay));
	mp_nackSupression = new NackSupressionDuration(this,TimeConv::Time_t2MilliSecondsDouble(times.nackSupressionDuration));
	logInfo(RTPS_WRITER,"Reader Proxy created");
}


ReaderProxy::~ReaderProxy()
{
	delete(mp_nackResponse);
	delete(mp_nackSupression);
	delete(mp_mutex);
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
        logInfo(RTPS_WRITER,"Change " << change->sequenceNumber << " found in Reader Proxy " << endl);
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
        logInfo(RTPS_WRITER,"Change " << seqNum <<" found in Reader Proxy " << endl);
        return true;
    }

	return false;
}

bool ReaderProxy::acked_changes_set(const SequenceNumber_t& seqNum)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
    auto chit = m_changesForReader.find(ChangeForReader_t(seqNum));

    if(chit != m_changesForReader.end())
    {
        if(chit == m_changesForReader.begin())
        { // If first, remove and cleanup.
            m_changesForReader.erase(chit);
            cleanup();
        }
        else
        {
            ChangeForReader_t newch(*chit);
            newch.setStatus(ACKNOWLEDGED);
            m_changesForReader.erase(chit);
            m_changesForReader.insert(newch);
        }
    }

    return false;
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

            m_changesForReader.erase(chit);

            auto ret = m_changesForReader.insert(newch);
            (void)ret;
            assert(ret.second);

            m_isRequestedChangesEmpty = false;
        }
	}

	if(!m_isRequestedChangesEmpty)
	{
		logInfo(RTPS_WRITER,"Requested Changes: " << seqNumSet);
	}
	return true;
}


std::vector<const ChangeForReader_t*> ReaderProxy::requested_changes_to_underway()
{
    std::vector<const ChangeForReader_t*> returnedValue;
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    auto it = m_changesForReader.begin();
    while(it != m_changesForReader.end())
	{
		if(it->getStatus() == REQUESTED)
		{
            ChangeForReader_t newch(*it);
            newch.setStatus(UNDERWAY);

            m_changesForReader.erase(it);

            auto ret = m_changesForReader.insert(newch);
            assert(ret.second);
            returnedValue.push_back(&(*ret.first));
            it = ret.first;
		}

        ++it;
	}

    return returnedValue;
}

std::vector<const ChangeForReader_t*> ReaderProxy::unsent_changes_to_underway()
{
    std::vector<const ChangeForReader_t*> returnedValue;
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    auto it = m_changesForReader.begin();
    while(it != m_changesForReader.end())
	{
		if(it->getStatus() == UNSENT)
		{
            ChangeForReader_t newch(*it);
            newch.setStatus(UNDERWAY);

            m_changesForReader.erase(it);

            auto ret = m_changesForReader.insert(newch);
            assert(ret.second);
            returnedValue.push_back(&(*ret.first));
            it = ret.first;
		}

        ++it;
	}

    return returnedValue;
}

void ReaderProxy::underway_changes_to_unacknowledged()
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    auto it = m_changesForReader.begin();
    while(it != m_changesForReader.end())
	{
		if(it->getStatus() == UNDERWAY)
		{
            ChangeForReader_t newch(*it);
            newch.setStatus(UNACKNOWLEDGED);

            m_changesForReader.erase(it);

            auto ret = m_changesForReader.insert(newch);
            assert(ret.second);
            it = ret.first;
		}

        ++it;
	}
}

void ReaderProxy::underway_changes_to_acknowledged()
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);

    auto it = m_changesForReader.begin();
    while(it != m_changesForReader.end())
	{
		if(it->getStatus() == UNDERWAY)
		{
            ChangeForReader_t newch(*it);
            newch.setStatus(ACKNOWLEDGED);

            m_changesForReader.erase(it);

            auto ret = m_changesForReader.insert(newch);
            assert(ret.second);
            it = ret.first;
		}

        ++it;
	}

    cleanup();
}

void ReaderProxy::setNotValid(const CacheChange_t* change)
{
	boost::lock_guard<boost::recursive_mutex> guard(*mp_mutex);
    auto chit = m_changesForReader.find(ChangeForReader_t(change));

    if(chit != m_changesForReader.end())
    {
        if(chit == m_changesForReader.begin())
        {
            m_changesForReader.erase(chit);
            cleanup();
        }
        else
        {
            ChangeForReader_t newch(*chit);
            newch.notValid();

            m_changesForReader.erase(chit);

            auto ret = m_changesForReader.insert(newch);
            (void)ret;
            assert(ret.second);
        }
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
