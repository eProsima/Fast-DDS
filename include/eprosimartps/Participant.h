/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Participant.h
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

#include "rtps_all.h"

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>


#ifndef PARTICIPANT_H_
#define PARTICIPANT_H_

namespace eprosima {
namespace rtps {

class Participant {
public:
	Participant();
	virtual ~Participant();
	ProtocolVersion_t protocolVersion;
	VendorId_t vendorId;
	std::vector<Locator_t> defaultunicastLocatorList;
	std::vector<Locator_t> defaultmulticastLocatorList;
	GUID_t guid;




	boost::mutex sendMutex;
	boost::asio::io_service sendService;
	void sendSync(CDRMessage_t msg,Locator_t loc);
	//void sendSync();
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARTICIPANT_H_ */
