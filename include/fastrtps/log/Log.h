/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Log is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Log.h
 *
 */

/** @defgroup LOG_MODULE Log Module
 * Preprocessor definitions to use the Log Module
 * @{
 */


#ifndef _FASTRTPS_LOG_LOG_H_
#define _FASTRTPS_LOG_LOG_H_

#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <cstdint>
#include <queue>
#include <map>

#include "Colors.h"
#include "../fastrtps_dll.h"

namespace boost
{
	class mutex;
}


namespace eprosima {
	/**
	* LOG_CATEGORY enumeration forward declaration. Is defined for each application.
	*/
#if defined(_WIN32)
enum RTPS_DllAPI LOG_CATEGORY;
#else
enum LOG_CATEGORY:uint32_t;
#endif

/**
* Verbosity Level.
*/
	enum LOG_VERBOSITY_LVL : uint32_t
	{
		VERB_QUIET,
		VERB_ERROR,
		VERB_WARNING,
		VERB_INFO
	};

	enum LOG_TYPE :uint32_t
	{
		T_GENERAL,
		T_ERROR,
		T_WARNING,
		T_INFO
	};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC



typedef std::map<LOG_CATEGORY,LOG_VERBOSITY_LVL> CategoryVerbosity;

/**
 * Class LogMessage
 */
class LogMessage
{
public:
	LogMessage() :m_type(T_GENERAL), m_cat((LOG_CATEGORY)0), m_ready(false){};
	virtual ~LogMessage(){};
	std::stringstream m_msg;
	LOG_TYPE m_type;
	LOG_CATEGORY m_cat;
	bool m_ready;
	std::stringstream m_date;
	std::stringstream m_color;
	void reset()
	{
		m_msg.str("");
		m_date.str("");
		m_color.str("");
		m_ready = false;
		m_type = T_GENERAL;
		m_cat = (LOG_CATEGORY)0;
	}
};
#endif
/**
 * Class Log used to manage the log entries.
 * @ingroup LOG_MODULE
 */
class Log {
public:

	/**
	 * Set the maximum verbosity level for ALL categories.
	 * @param level Verbosity level.
	 */
	RTPS_DllAPI static void setVerbosity(LOG_VERBOSITY_LVL level);
	/**
	 * Set the maximum verbosity level for a specific category.
	 * @param cat LOG_CATEGORY
	 * @param level Verbosity level.
	 */
	RTPS_DllAPI static void setCategoryVerbosity(LOG_CATEGORY cat, LOG_VERBOSITY_LVL level);
	/**
	 * Set the log Filename. If this method is not called no text log will be created.
	 * @param filename The name of the log file to create
	 * @param add_date_to_filename If set to true the name of the log file will be date_filename.txt
	 */
	RTPS_DllAPI static void logFileName(const char* filename, bool add_date_to_filename = false);

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
	RTPS_DllAPI static LogMessage& logMessage(LOG_TYPE type, LOG_CATEGORY cat,
						const char* CLASS_NAME,
						const char* METHOD_NAME,
						const char* COLOR = nullptr);
	RTPS_DllAPI static LogMessage& logMessage(LOG_TYPE type, const char* COLOR = nullptr);

	RTPS_DllAPI static void addMessage(LogMessage& lm);

	RTPS_DllAPI static void removeLog();



private:
	Log();
	virtual ~Log();

	static Log* getInstance();

	static Log* m_instance;
	static bool m_instanceFlag;

	std::ofstream* mp_logFile;
	bool m_logFileDefined;

	LOG_VERBOSITY_LVL m_defaultVerbosityLevel;

	CategoryVerbosity m_categories;

	CategoryVerbosity::iterator getCategory(LOG_CATEGORY cat);

	LogMessage* getLogMessage();

	std::stringstream m_stream;

	std::queue<LogMessage*> m_logMessages;
//	std::queue<LogMessage*> m_readyLogMsg;
//	std::queue<LogMessage*> m_toPrint;

	boost::mutex* mp_logMesgMutex;
	boost::mutex* mp_printMutex;

	void printLogMessage(LogMessage& lm);
	void printMessageString(LogMessage& lm);

	//void printLogMsg(LogMessage* lm);
	//void printMsg(LogMessage* lm);
#endif
};




} /* namespace eprosima */



/**
* @def logUser
* @brief Create a user log entry.
* @code logUser("My Log User Message " << auxInt << auxString,C_BLUE);   @endcode
*/
#define logUser(str,...) {eprosima::LogMessage& lm = eprosima::Log::logMessage(eprosima::LOG_TYPE::T_GENERAL,##__VA_ARGS__);lm.m_msg << str;eprosima::Log::addMessage(lm); }

/**
* @def logError
* Log an error into a category.
* const char* METHOD_NAME and const char* CLASS_NAME MUST be defined.
* @code logError(LOG_CATEGORY,auxInt << " I print the error message " << otherVariable, C_RED); @endcode
*/
#define logError(cat,str,...){eprosima::LogMessage& lm = eprosima::Log::logMessage(eprosima::LOG_TYPE::T_ERROR,cat,CLASS_NAME,METHOD_NAME,##__VA_ARGS__);lm.m_msg << str;eprosima::Log::addMessage(lm); }
/**
* @def logWarning
* Log a Warning into a category. The warning will only show if the verbosity of a specific category is greater that the VERB_WARNING level.
* const char* METHOD_NAME and const char* CLASS_NAME MUST be defined.
* @code logWarning(LOG_CATEGORY,auxInt << " I print the info message " << otherVariable, C_RED);
* logWarning(LOG_CATEGORY,auxInt << " Whathever stream i want " << otherVariable);   @endcode
*/
#define logWarning(cat,str,...){eprosima::LogMessage& lm = eprosima::Log::logMessage(eprosima::LOG_TYPE::T_WARNING,cat,CLASS_NAME,METHOD_NAME,##__VA_ARGS__);lm.m_msg << str;eprosima::Log::addMessage(lm); }
#if defined(__DEBUG) || defined(_DEBUG)
/**
* @def logInfo
* Log a Info message into a category. This method only works in DEBUG compilation to ensure that release programs are faster.
* const char* METHOD_NAME and const char* CLASS_NAME MUST be defined.
*  @code logInfo(LOG_CATEGORY,auxInt << " I print the info message " << otherVariable, C_RED);
* logInfo(LOG_CATEGORY,auxInt << " Whathever stream i want " << otherVariable);   @endcode
*/
#define logInfo(cat,str,...){eprosima::LogMessage& lm = eprosima::Log::logMessage(eprosima::LOG_TYPE::T_INFO,cat,CLASS_NAME,METHOD_NAME,##__VA_ARGS__);lm.m_msg << str;eprosima::Log::addMessage(lm); }
#else
#define logInfo(cat,str,...)
#endif

/**
 * @}
 */

#endif /* _FASTRTPS_LOG_LOG_H_ */
