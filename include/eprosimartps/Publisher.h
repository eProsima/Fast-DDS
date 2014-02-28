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
namespace dds {

class Publisher {
public:
	Publisher();
	virtual ~Publisher();

	const std::string& getTopicName() const {
		return topicName;
	}

	void setTopicName(const std::string& topicName) {
		if(!initialized)
			this->topicName = topicName;
	}

private:
	bool initialized;
	std::string topicName;
	std::string topicDataType;

};

} /* namespace dds */
} /* namespace eprosima */

#endif /* PUBLISHER_H_ */
