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
 *  Created on: Mar 11, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include <eprosimartps/utils/RTPSLog.h>

namespace eprosima {


std::ostringstream RTPSLog::Error;
std::ostringstream RTPSLog::Warning;
std::ostringstream RTPSLog::Info;
std::ostringstream RTPSLog::DebugInfo;
std::ostringstream RTPSLog::LongInfo;



bool RTPSLog::instanceFlag = false;
RTPSLog* RTPSLog::single = NULL;


void RTPSLog::printError()
{
	std::stringstream ss;
	ss << B_RED << "[eRTPS- Err] " << DEF <<  RTPSLog::Error.str() << DEF;
	RTPSLog::printString(EPROSIMA_ERROR_VERBOSITY_LEVEL,ss.str());
	RTPSLog::Error.str("");
	RTPSLog::Error.clear();
}

void RTPSLog::printWarning()
{
	std::stringstream ss;
		ss << B_YELLOW << "[eRTPS-Warn] " << DEF <<  RTPSLog::Warning.str() << DEF;
	RTPSLog::printString(EPROSIMA_WARNING_VERBOSITY_LEVEL,ss.str());
	RTPSLog::Warning.str("");
	RTPSLog::Warning.clear();
}

void RTPSLog::printInfo()
{
	std::stringstream ss;
		ss << B_GREEN << "[eRTPS-Info] " << DEF <<  RTPSLog::Info.str() << DEF;
	RTPSLog::printString(EPROSIMA_INFO_VERBOSITY_LEVEL,ss.str());
	RTPSLog::Info.str("");
	RTPSLog::Info.clear();
}

void RTPSLog::printDebugInfo()
{
	std::stringstream ss;
		ss << B_GREEN << "[eRTPS-DebugInfo] " << DEF <<  RTPSLog::DebugInfo.str() << DEF;
	RTPSLog::printString(EPROSIMA_DEBUGINFO_VERBOSITY_LEVEL,ss.str());
	RTPSLog::DebugInfo.str("");
	RTPSLog::DebugInfo.clear();
}

void RTPSLog::printLongInfo()
{
	std::stringstream ss;
			ss << B_GREEN << "[eRTPS-Long Info] " << std::endl << DEF <<  RTPSLog::LongInfo.str() << DEF <<  B_GREEN << "[/eRTPS-Long Info] "<< DEF << endl ;
	RTPSLog::printString(EPROSIMA_LONGINFO_VERBOSITY_LEVEL,ss.str());
	RTPSLog::LongInfo.str("");
	RTPSLog::LongInfo.clear();
}



void RTPSLog::printString(EPROSIMA_LOG_VERBOSITY_LEVEL lvl,std::string s)
{
	RTPSLog* RL = getInstance();
	if(RL->verbosityLevel >= lvl)
	{
		RL->print_mutex.lock();
		std::cout << s;
		RL->print_mutex.unlock();
	}
}

RTPSLog* RTPSLog::getInstance()
{
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

void RTPSLog::setVerbosity(EPROSIMA_LOG_VERBOSITY_LEVEL level)
{
	RTPSLog* RL = getInstance();
	RL->verbosityLevel = level;
}


} /* namespace eprosima */

