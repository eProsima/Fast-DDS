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


#ifndef PUBLISHER_H_
#define PUBLISHER_H_



namespace eprosima {

namespace rtps{
class RTPSWriter;
}

using namespace rtps;

namespace dds {



class Publisher {
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
	bool addReaderLocator(Locator_t Loc,bool expectsInlineQos);


	ParameterList ParamList;
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
