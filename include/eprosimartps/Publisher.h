/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * Publisher.h
 *
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include <iostream>

#include "rtps_all.h"
#include "ParameterList_t.h"
#include "common/rtps_messages.h"

#ifndef PUBLISHER_H_
#define PUBLISHER_H_



namespace eprosima {

namespace rtps{
class RTPSWriter;
}

using namespace rtps;

/**
 * DDS namespace. Contains the public API to interact with the DDS-RTPS protocol.
 * @ingroup DDSMODULE
 */
namespace dds {


/**
 * Class Publisher, contains the public API to send new data. This class should not be instantiated directly.
 * DomainParticipant class should be used to correctly initialize this element.
 * @ingroup DDSMODULE
 */
class RTPS_DllAPI Publisher {
	friend class DomainParticipant;
public:
	Publisher();
	Publisher(RTPSWriter* Win);
	virtual ~Publisher();

	const std::string& getTopicName() const {
		return topicName;
	}

	const std::string& getTopicDataType() const {
		return topicDataType;
	}
	bool write(void*Data);

	//Since there is no discovery:
	/**
	 * Add a Reader Locator to the publisher.
	 * @param Loc Locator to add.
	 * @param expectsInlineQos Parameter to indicate wheter or not the locataor expects inline Qos with its Data messages.
	 * @return True if correct.
	 */
	bool addReaderLocator(Locator_t Loc,bool expectsInlineQos);


	ParameterList_t ParamList;
private:
	RTPSWriter* W;
	//bool initialized;
	std::string topicName;
	std::string topicDataType;
	TypeReg_t type;


};

} /* namespace dds */
} /* namespace eprosima */

#endif /* PUBLISHER_H_ */
