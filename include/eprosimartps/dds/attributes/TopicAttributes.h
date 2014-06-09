/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TopicAttributes.h
 */

#ifndef TOPICPARAMETERS_H_
#define TOPICPARAMETERS_H_

#include <string>

namespace eprosima {
namespace rtps {

//!@brief Enum TopicKind_t.
typedef enum TopicKind_t{
	NO_KEY=1,
	WITH_KEY=2
}TopicKind_t;

}

using namespace rtps;

namespace dds{

/**
 * Class TopicAttributes, used by the user to define the attributes of the topic associated with a Publisher or Subscriber.
 * @ingroup ATTRIBUTESMODULE
 */
class TopicAttributes {
public:
	TopicAttributes()
{
		topicKind = NO_KEY;
		topicName = "UNDEF";
		topicDataType = "UNDEF";
}
	virtual ~TopicAttributes() {
	}
	const std::string& getTopicDataType() const {
		return topicDataType;
	}

	TopicKind_t getTopicKind() const {
		return topicKind;
	}

	const std::string& getTopicName() const {
		return topicName;
	}

	//! TopicKind_t (WITH_KEY or NO_KEY)
	TopicKind_t topicKind;
	//! Topic Name.
	std::string topicName;
	//!Topic Data Type.
	std::string topicDataType;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* TOPICPARAMETERS_H_ */
