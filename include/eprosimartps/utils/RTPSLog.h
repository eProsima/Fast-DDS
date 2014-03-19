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
 *  Created on: Mar 11, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include <iostream>
#include <ostream>
#include <eprosimartps/common/colors.h>

#include <eprosimartps/rtps_all.h>

#ifndef RTPSLOG_H_
#define RTPSLOG_H_

#if defined(__DEBUG)
#define pError(str) {RTPSLog::Error << str;RTPSLog::printError();}
#define pWarning(str) {RTPSLog::Warning << str;RTPSLog::printWarning();}
#define pInfo(str) {RTPSLog::Info << str;RTPSLog::printInfo();}
#define pDebugInfo(str) {RTPSLog::DebugInfo << str ;RTPSLog::printDebugInfo();}
#define pLongInfo(str) {RTPSLog::LongInfo << str ;}
#define pLongInfoPrint {RTPSLog::printLongInfo();}
#else
#define pError(str) {RTPSLog::Error << str;RTPSLog::printError();}
#define pWarning(str)
#define pInfo(str)
#define pDebugInfo(str)
#define pLongInfo(str)
#define pLongInfoPrint
#endif



namespace eprosima {

/**
 * Class RTPSLog designed to output information in 3 different levels.
 * @ingroup UTILITIESMODULE
 */
class RTPS_DllAPI RTPSLog {
public:
	//! Verbosity levels available
	typedef enum EPROSIMA_LOG_VERBOSITY_LEVEL
	{
		EPROSIMA_QUIET_VERBOSITY_LEVEL = 0,
		EPROSIMA_ERROR_VERBOSITY_LEVEL = 1,
		EPROSIMA_WARNING_VERBOSITY_LEVEL = 2,
		EPROSIMA_INFO_VERBOSITY_LEVEL = 3,
		EPROSIMA_DEBUGINFO_VERBOSITY_LEVEL = 4,
		EPROSIMA_LONGINFO_VERBOSITY_LEVEL = 5
	} EPROSIMA_LOG_VERBOSITY_LEVEL;

	/**
	 * Set verbosity for all outputs.
	 * @param level Verbosity level
	 */
	static void setVerbosity(EPROSIMA_LOG_VERBOSITY_LEVEL level);

	/**
	 * @name Print String functions.
	 *   This functions print the provided string if the verbosity level allow it.
	 */
	///@{
	/**
	 * Print a string if the verbosity level allows it
	 * @param lvl Verbosity level.
	 * @param s String to print.
	 */
	static void printString(EPROSIMA_LOG_VERBOSITY_LEVEL lvl,std::string s);
	///@}
	/**
	 * @name Print Streams functions.
	 *   This functions print the static ostringstreams and clear them.
	 */
	///@{
	static void printError();
	static void printWarning();
	static void printInfo();
	static void printDebugInfo();
	static void printLongInfo();

	///@}

	/**
	 * @name Static string streams.
	 */
	///@{
	static std::ostringstream Error;
	static std::ostringstream Warning;
	static std::ostringstream Info;
	static std::ostringstream DebugInfo;
	static std::ostringstream LongInfo;
	///@}


private:
	EPROSIMA_LOG_VERBOSITY_LEVEL verbosityLevel;
	static bool instanceFlag;
	static RTPSLog *single;
	RTPSLog()
	{
		verbosityLevel = EPROSIMA_QUIET_VERBOSITY_LEVEL;
	}

public:
	 static RTPSLog* getInstance();
	 ~RTPSLog()
	 {
		 instanceFlag = false;
	 }
};



} /* namespace eprosima */

#endif /* RTPSLOG_H_ */
