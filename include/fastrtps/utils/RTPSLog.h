// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
