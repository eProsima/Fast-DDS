/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSLog.cpp
 *
 */

#include <eprosimartps/utils/RTPSLog.h>

namespace eprosima {


void RTPSThreadLog::printString(EPROSIMA_LOG_VERBOSITY_LEVEL type,std::string& s)
{
	if(type <= m_verbosity)
	{
		switch(type)
		{
		case(EPROSIMA_QUIET_VERB_LEVEL):
		default:
		{
			break;
		}
		case(EPROSIMA_ERROR_VERB_LEVEL):
		{
			m_log << RTPS_B_RED << "[eRTPS- Err] " << RTPS_DEF << s << RTPS_DEF;
			break;
		}
		case(EPROSIMA_WARNING_VERB_LEVEL):
		{
			m_log << RTPS_B_YELLOW << "[eRTPS-Warn] " << RTPS_DEF << s << RTPS_DEF;
			break;
		}
		case(EPROSIMA_INFO_VERB_LEVEL):
		{
			m_log << RTPS_B_GREEN << "[eRTPS-Info] " << RTPS_DEF << s << RTPS_DEF;
			break;
		}
		case(EPROSIMA_DEBUGINFO_VERB_LEVEL):
		{
			m_log << RTPS_GREEN << "[eRTPS-DebugInfo] " << RTPS_DEF << s << RTPS_DEF;
			break;
		}
		}
		if(RTPSLog::getInstance()->try_lock())
		{
			std::cout<< m_log.str()<< std::flush;
			m_log.str("");m_log.clear();
			RTPSLog::getInstance()->unlock();
		}
	}
}

bool RTPSLog::instanceFlag = false;
RTPSLog* RTPSLog::single = NULL;

void RTPSLog::printString(EPROSIMA_LOG_VERBOSITY_LEVEL type,std::string stri)
{
	RTPSLog* rl = RTPSLog::getInstance();
	boost::thread::id id = boost::this_thread::get_id();
	for(std::vector<RTPSThreadLog*>::iterator it = rl->m_logs.begin();
			it!= rl->m_logs.end();++it)
	{
		if(id == (*it)->m_id)
		{
			(*it)->printString(type,stri);
			return;
		}
	}
	RTPSThreadLog* rtl = new RTPSThreadLog(id,rl->m_verbosityLevel);
	rl->m_logs.push_back(rtl);
	(*(rl->m_logs.end()-1))->printString(type,stri);
	return;
}

RTPSLog* RTPSLog::getInstance()
{
	//boost::this_thread::get_id();
	if(! instanceFlag)
	{
		single = new RTPSLog();
		instanceFlag = true;
		return single;
	}
	else
	{
		return single;
	}
}
RTPSLog::~RTPSLog()
{
	for(std::vector<RTPSThreadLog*>::iterator it=m_logs.begin();
			it!=m_logs.end();++it)
	{
		delete(*it);
	}
	m_logs.clear();
	RTPSLog::instanceFlag = false;

}

void RTPSLog::setVerbosity(EPROSIMA_LOG_VERBOSITY_LEVEL level)
{
	RTPSLog* RL = getInstance();
	RL->m_verbosityLevel = level;
	for(std::vector<RTPSThreadLog*>::iterator it = RL->m_logs.begin();
			it!=RL->m_logs.end();++it)
	{
		(*it)->m_verbosity = level;
	}
}



} /* namespace eprosima */

