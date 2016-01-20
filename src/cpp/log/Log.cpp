/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * @file Log.cpp
 *
 */



#include <fastrtps/log/Log.h>

#define START_N_MESSAGES 2000
#define MIN_N_MESSAGES 500

#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
namespace eprosima {

Log* Log::m_instance = NULL;
bool Log::m_instanceFlag = false;

static const int MAXWIDTH = 43;


Log::Log(): mp_logFile(NULL),
		m_logFileDefined(false),
#if defined(__DEBUG) || defined(_DEBUG)
		m_defaultVerbosityLevel(VERB_INFO),
#else
		m_defaultVerbosityLevel(VERB_ERROR),
#endif
		mp_logMesgMutex(NULL),
		mp_printMutex(NULL)

{

	mp_logMesgMutex = new boost::mutex();
	mp_printMutex = new boost::mutex();


	for(uint32_t i = 0;i< START_N_MESSAGES;++i)
	{
		m_logMessages.push(new LogMessage());
	}

}

void Log::removeLog()
{
	delete(Log::getInstance());
}

Log::~Log() {
	// TODO Auto-generated destructor stub
	if(mp_logFile !=NULL)
	{
		mp_logFile->close();
		delete(mp_logFile);
	}

	delete(mp_logMesgMutex);
	delete(mp_printMutex);

	m_instanceFlag = false;
	m_instance = nullptr;
	while(m_logMessages.size()>0)
	{
		delete(m_logMessages.front());
		m_logMessages.pop();
	}
}

Log* Log::getInstance()
{
	if(!m_instanceFlag)
	{
		m_instance = new Log();
		m_instanceFlag = true;
	}
	return m_instance;
}

void Log::setVerbosity(LOG_VERBOSITY_LVL level)
{
	Log* ELOG = Log::getInstance();
	ELOG->m_defaultVerbosityLevel = level;
	for(CategoryVerbosity::iterator it = ELOG->m_categories.begin();
			it!= ELOG->m_categories.end();++it)
	{
		it->second = level;
	}
}

void Log::setCategoryVerbosity(LOG_CATEGORY cat, LOG_VERBOSITY_LVL level)
{
	Log* ELOG = Log::getInstance();
	ELOG->getCategory(cat)->second = level;
}

void Log::logFileName(const char* filename,bool add_date_to_name)
{
	Log* ELOG = Log::getInstance();
	std::stringstream ss;
	if(add_date_to_name)
	{
		boost::posix_time::time_facet* facet = new boost::posix_time::time_facet();
		facet->format("%Y%m%d_%H%M%S");
		ss.imbue(std::locale(std::locale::classic(), facet));
		ss << boost::posix_time::ptime(boost::posix_time::second_clock::local_time())<<"_";
	}
	ss << filename;
	ELOG->mp_logFile = new std::ofstream();
	ELOG->mp_logFile->open(ss.str().c_str(),std::ios::app);
	ELOG->m_logFileDefined = true;
	(*ELOG->mp_logFile) << std::endl;
	(*ELOG->mp_logFile) << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"<<std::endl;
	(*ELOG->mp_logFile) << "+                                EPROSIMA LOG                                  +"<<std::endl;
	(*ELOG->mp_logFile) << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"<<std::endl;
	(*ELOG->mp_logFile) << "+                            "<< boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time());
	(*ELOG->mp_logFile) << "                              +"<<std::endl;
	(*ELOG->mp_logFile) << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"<<std::endl;
	(*ELOG->mp_logFile) << std::endl;
}

LogMessage* Log::getLogMessage()
{
	mp_logMesgMutex->lock();
	if(m_logMessages.size()<MIN_N_MESSAGES)
	{
		for(uint32_t i = 0;i<MIN_N_MESSAGES;++i)
			m_logMessages.push(new LogMessage());
	}
	LogMessage* msg = m_logMessages.front();
	m_logMessages.pop();
	mp_logMesgMutex->unlock();
	return msg;
}

CategoryVerbosity::iterator Log::getCategory(LOG_CATEGORY cat)
{
	mp_logMesgMutex->lock();
	CategoryVerbosity::iterator it = m_categories.find(cat);
	if(it != m_categories.end())
		return it;
	m_categories[cat] = m_defaultVerbosityLevel;
	mp_logMesgMutex->unlock();
	return m_categories.find(cat);
}


LogMessage* Log::logMessage(LOG_TYPE type, LOG_CATEGORY cat, const char* CLASS_NAME,
		const char* METHOD_NAME,const char * COLOR)
{
	Log* ELOG = Log::getInstance();

    switch(type)
    {
        default:
        case T_GENERAL:
            break;
        case T_ERROR:
                if(ELOG->getCategory(cat)->second < VERB_ERROR)
                    return nullptr;
                break;
        case T_WARNING:
                if(ELOG->getCategory(cat)->second < VERB_WARNING)
                    return nullptr;
                break;
        case T_INFO:
                if(ELOG->getCategory(cat)->second < VERB_INFO)
                    return nullptr;
                break;
    }

	LogMessage* log = ELOG->getLogMessage();
	log->m_type = type;
	log->m_cat = cat;
	log->m_color.str("");
	if (COLOR != nullptr)
		log->m_color << COLOR;
	else
		log->m_color << C_DEF;
	log->m_msg.str("");
	log->m_msg <<  "["<<CLASS_NAME << "::"<<METHOD_NAME;
    log->m_msg << std::left << std::setw(log->m_msg.str().size() >= MAXWIDTH ? 0 : MAXWIDTH - log->m_msg.str().size()) << "] ";
	log->m_date.str("");
	log->m_date << "["<< boost::posix_time::to_iso_string(boost::posix_time::microsec_clock::local_time())<<"]";

	return log;
}

LogMessage* Log::logMessage(LOG_TYPE type,const char* COLOR)
{
	Log* ELOG = Log::getInstance();

    switch(type)
    {
        default:
        case T_GENERAL:
            break;
        case T_ERROR:
                if(ELOG->getCategory((LOG_CATEGORY)0)->second < VERB_ERROR)
                    return nullptr;
                break;
        case T_WARNING:
                if(ELOG->getCategory((LOG_CATEGORY)0)->second < VERB_WARNING)
                    return nullptr;
                break;
        case T_INFO:
                if(ELOG->getCategory((LOG_CATEGORY)0)->second < VERB_INFO)
                    return nullptr;
                break;
    }

	LogMessage* log = ELOG->getLogMessage();
	log->m_type = type;
	log->m_msg.str("");
	if (COLOR != nullptr)
			log->m_color << COLOR;
		else
			log->m_color << C_DEF;
	log->m_date.str("");
	log->m_date << "["<< boost::posix_time::to_iso_string(boost::posix_time::microsec_clock::local_time())<<"]";

	return log;
}


void Log::addMessage(LogMessage* lm)
{
	Log* ELOG = Log::getInstance();
	ELOG->mp_printMutex->lock();
	ELOG->printLogMessage(lm);
	ELOG->mp_printMutex->unlock();
	lm->reset();
	ELOG->mp_logMesgMutex->lock();
	ELOG->m_logMessages.push(lm);
	ELOG->mp_logMesgMutex->unlock();
}


//void Log::printMessages()
//{
//	mp_PrintMutex->lock();
//	//if(mp_PrintMutex->try_lock())
//	{
//		mp_readyLogMsgMutex->lock();
//		while(!m_readyLogMsg.empty())
//		{
//			m_toPrint.push(m_readyLogMsg.front());
//			m_readyLogMsg.pop();
//		}
//		mp_readyLogMsgMutex->unlock();
//
//		while(!m_toPrint.empty())
//		{
//			LogMessage* lm = m_toPrint.front();
//			m_toPrint.pop();
//			printLogMsg(lm);
//			lm->reset();
//			m_logMessages.push(lm);
//		}
//		mp_PrintMutex->unlock();
//	}
//}

void Log::printLogMessage(LogMessage* lm)
{
	switch(lm->m_type)
	{
	default:
	case T_GENERAL:
	{
		std::cout << C_BRIGHT<<"[UserLog]"<< C_DEF;
		if(this->m_logFileDefined)
			(*this->mp_logFile) <<lm->m_date.str().c_str()<< "[UserLog]";
		printMessageString(lm);
		break;
	}
	case T_ERROR:
	{
		if(getCategory(lm->m_cat)->second >= VERB_ERROR)
		{
			std::cout << C_B_RED << "[Error]" << C_DEF;
			if(this->m_logFileDefined)
				(*this->mp_logFile) << lm->m_date.str().c_str()<< "[Error]";
			printMessageString(lm);
		}
		break;
	}
	case T_WARNING:
	{
		if(getCategory(lm->m_cat)->second >= VERB_WARNING)
		{
			std::cout << C_B_YELLOW << "[Warning]" << C_DEF;
			if(this->m_logFileDefined)
				(*this->mp_logFile) <<lm->m_date.str().c_str()<< "[Warning]";
			printMessageString(lm);
		}
		break;
	}
	case T_INFO:
	{
		if(getCategory(lm->m_cat)->second >= VERB_INFO)
		{
			std::cout << C_B_GREEN << "[Info]" << C_DEF;
			if(this->m_logFileDefined)
				(*this->mp_logFile) <<lm->m_date.str().c_str()<< "[Info]";
			printMessageString(lm);
		}
		break;
	}
	}
}

void Log::printMessageString(LogMessage* lm)
{
	std::cout << lm->m_color.str() << lm->m_msg.str() << C_DEF<< std::endl;
	if(this->m_logFileDefined)
	{
		(*this->mp_logFile) << lm->m_msg.str().c_str() << std::endl;

	}
}

} /* namespace eprosima */


#endif
