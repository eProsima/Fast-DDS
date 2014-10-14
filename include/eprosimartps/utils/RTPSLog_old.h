/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSLog.h
 *
 */


#ifndef RTPSLOG_H_
#define RTPSLOG_H_

#include <iostream>
#include <ostream>

#include "eprosimartps/common/types/common_types.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread.hpp>



namespace eprosima {

typedef enum EPROSIMA_LOG_VERBOSITY_LEVEL
	{
		EPROSIMA_QUIET_VERB_LEVEL = 0,
		EPROSIMA_ERROR_VERB_LEVEL = 1,
		EPROSIMA_WARNING_VERB_LEVEL = 2,
		EPROSIMA_INFO_VERB_LEVEL = 3,
		EPROSIMA_DEBUGINFO_VERB_LEVEL = 4
	} EPROSIMA_LOG_VERBOSITY_LEVEL;


class RTPSThreadLog
{
	friend class RTPSLog;
public:
	std::ostringstream m_log;
	boost::thread::id m_id;
	EPROSIMA_LOG_VERBOSITY_LEVEL m_verbosity;
	RTPSThreadLog():m_verbosity(EPROSIMA_DEBUGINFO_VERB_LEVEL){};
	RTPSThreadLog(boost::thread::id id,EPROSIMA_LOG_VERBOSITY_LEVEL lvl):
		m_id(id),m_verbosity(lvl){};

	void printString(EPROSIMA_LOG_VERBOSITY_LEVEL lvl,std::string& s);
	virtual ~RTPSThreadLog(){};
};


/**
 * Class RTPSLog designed to output information in 3 different levels.
 * @ingroup UTILITIESMODULE
 */
class RTPSLog: public boost::lockable_adapter<boost::recursive_mutex>
{
public:
	//! Verbosity levels available


	/**
	 * Set verbosity for all outputs.
	 * @param level Verbosity level
	 */
	RTPS_DllAPI static void setVerbosity(EPROSIMA_LOG_VERBOSITY_LEVEL level);

	/**
	 * Print a string if the verbosity level allows it
	 * @param lvl Verbosity level.
	 * @param s String to print.
	 */
	RTPS_DllAPI static void printString(EPROSIMA_LOG_VERBOSITY_LEVEL lvl,std::string s);


	static EPROSIMA_LOG_VERBOSITY_LEVEL getVerbosity()
	{
		return RTPSLog::getInstance()->m_verbosityLevel;
	}

private:
	EPROSIMA_LOG_VERBOSITY_LEVEL m_verbosityLevel;
	static bool instanceFlag;
	static RTPSLog *single;
	RTPSLog()
	{
		m_verbosityLevel = EPROSIMA_ERROR_VERB_LEVEL;
	}
	std::vector<RTPSThreadLog*> m_logs;
public:
	 static RTPSLog* getInstance();
	 ~RTPSLog();

};


#if defined(__DEBUG)
#define pError(strin) {std::stringstream ss;ss<<strin;RTPSLog::printString(EPROSIMA_ERROR_VERB_LEVEL,ss.str());}
#define pWarning(strin) {std::stringstream ss;ss<<strin;RTPSLog::printString(EPROSIMA_WARNING_VERB_LEVEL,ss.str());}
#define pInfo(strin) {std::stringstream ss;ss<<strin;RTPSLog::printString(EPROSIMA_INFO_VERB_LEVEL,ss.str());}
#define pDebugInfo(strin) {std::stringstream ss;ss<<strin;RTPSLog::printString(EPROSIMA_DEBUGINFO_VERB_LEVEL,ss.str());}
#else
#define pError(strin) {std::stringstream ss;ss<<strin;RTPSLog::printString(EPROSIMA_ERROR_VERB_LEVEL,ss.str());}
#define pWarning(strin) {std::stringstream ss;ss<<strin;RTPSLog::printString(EPROSIMA_WARNING_VERB_LEVEL,ss.str());}
#define pInfo(strin)
#define pDebugInfo(strin)
#endif

}; /* namespace eprosima */





#endif /* RTPSLOG_H_ */
