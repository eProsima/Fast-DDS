/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TopicParameters.h
 *
 *  Created on: May 5, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef TOPICPARAMETERS_H_
#define TOPICPARAMETERS_H_

namespace eprosima {
namespace rtps {

//!@brief Enum TopicKind_t.
typedef enum TopicKind_t{
	NO_KEY=1,
	WITH_KEY=2
}TopicKind_t;

class TopicAttributes {
public:
	TopicAttributes()
{
	topicKind = NO_KEY;
	topicName = "UNDEF";
	topicDataType = "UNDEF";
};
	virtual ~TopicAttributes();
	TopicKind_t topicKind;
	std::string topicName;
	std::string topicDataType;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* TOPICPARAMETERS_H_ */
