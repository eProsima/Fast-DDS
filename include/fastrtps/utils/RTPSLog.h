/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSLog.h
 *
 */

#ifndef RTPSLOG_H_
#define RTPSLOG_H_

#include "../log/Log.h"

namespace eprosima{

/**
 * @enum LOG_CATEGORY
 * The user of this library can change the following names or add more if he wants to build something on top of the RTPS
 * library. As long as the user defined categories stay larger than 1000 there shouldn't be any problem.
 *  @ingroup UTILITIES_MODULE
 */
enum LOG_CATEGORY : uint32_t
{
	RTPS_PDP = 1,    //!< RTPS Participant Discovery Protocol log messages.
	RTPS_EDP,        //!< RTPS Endpoint Discovery Protocol log messages.
	RTPS_LIVELINESS, //!< RTPS Liveliness log messages.

	RTPS_QOS_CHECK,  //!< RTPS QOS Check log messages.
	RTPS_CDR_MSG,    //!< RTPS CDR Messages log messages.
	RTPS_UTILS,      //!< RTPS Utils log messages.
	RTPS_HISTORY,    //!< RTPS History log messages.
	RTPS_WRITER,     //!< RTPS Writer log messages.
	RTPS_READER,     //!< RTPS Reader log messages.
	RTPS_MSG_IN,     //!< RTPS Incomming messages
	RTPS_MSG_OUT,    //!< RTPS Outgoing messages log messages.
	RTPS_PROXY_DATA, //!< RTPS Proxy Data Structures log messages.
	RTPS_PARTICIPANT,//!< RTPS Participant log messages.
	PUBLISHER,       //!< Publisher log messages.
	SUBSCRIBER,      //!< Subscriber log messages.
	PARTICIPANT,     //!< Participant log messages.

	//The user of this library can change the following names or add more if he wants to build something on top of the RTPS
	//library. As long as the user defined categories stay larger than 1000 there shouldn't be any problem.

	USER = 1000,     //!< USER
	USER2,           //!< USER2
	USER3,           //!< USER3

};

}




#endif /* RTPSLOG_H_ */
