/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TopicAttributes.h
 */

#ifndef TOPICPARAMETERS_H_
#define TOPICPARAMETERS_H_

#include <string>

#include "fastrtps/rtps/common/Types.h"

#include "fastrtps/qos/QosPolicies.h"
#include "fastrtps/utils/RTPSLog.h"

using namespace eprosima::fastrtps::rtps;

namespace eprosima {
namespace fastrtps{

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
	
	/**
	 * Get the topic data type
	 * @return Topic data type
	 */
	const std::string& getTopicDataType() const {
		return topicDataType;
	}

	/**
	 * Get the topic kind
	 * @return Topic kind
	 */
	TopicKind_t getTopicKind() const {
		return topicKind;
	}

	/**
	 * Get the topic name
	 * @return Topic name
	 */
	const std::string& getTopicName() const {
		return topicName;
	}

	//! TopicKind_t (WITH_KEY or NO_KEY)
	TopicKind_t topicKind;
	//! Topic Name.
	std::string topicName;
	//!Topic Data Type.
	std::string topicDataType;

	HistoryQosPolicy historyQos;

	ResourceLimitsQosPolicy resourceLimitsQos;
	bool checkQos()
	{
		const char* const CLASS_NAME = "TopicAttributes";
		const char* const METHOD_NAME = "checkQos";
		if(resourceLimitsQos.max_samples_per_instance > resourceLimitsQos.max_samples && topicKind == WITH_KEY)
		{

			logError(RTPS_QOS_CHECK,"INCORRECT TOPIC QOS:max_samples_per_instance must be <= than max_samples");
			return false;
		}
		if(resourceLimitsQos.max_samples_per_instance*resourceLimitsQos.max_instances > resourceLimitsQos.max_samples && topicKind == WITH_KEY)
			logWarning(RTPS_QOS_CHECK,"TOPIC QOS: max_samples < max_samples_per_instance*max_instances");
		if(historyQos.kind == KEEP_LAST_HISTORY_QOS)
		{
			if(historyQos.depth > resourceLimitsQos.max_samples)
			{
				logError(RTPS_QOS_CHECK,"INCORRECT TOPIC QOS: depth must be <= max_samples");
				return false;
			}
			if(historyQos.depth > resourceLimitsQos.max_samples_per_instance && topicKind == WITH_KEY)
			{
				logError(RTPS_QOS_CHECK,"INCORRECT TOPIC QOS: depth must be <= max_samples_per_instance"<<endl;)
										return false;
			}
			if(historyQos.depth <=0 )
			{
				logError(RTPS_QOS_CHECK,"INCORRECT TOPIC QOS: depth must be > 0");
				return false;
			}
		}
		return true;
	}
};

/**
 * Check if two topic attributes are not equal
 * @param t1 First instance of TopicAttributes to compare 
 * @param t2 Second instance of TopicAttributes to compare 
 * @return True if the instances are not equal. False if the instances are equal.
 */
bool inline operator!=(TopicAttributes& t1, TopicAttributes& t2)
{
	if(t1.topicKind != t2.topicKind)
	{
		//cout << "a"<<endl;
		return true;
	}
	if(t1.topicName != t2.topicName)
	{
		//cout << "b"<<endl;
		return true;
	}
	if(t1.topicDataType != t2.topicDataType)
	{
		//cout << "c"<<endl;
		return true;
	}
	if(t1.historyQos.kind != t2.historyQos.kind)
	{
		//cout << "d"<<endl;
		return true;
	}
	if(t1.historyQos.kind == KEEP_LAST_HISTORY_QOS && t1.historyQos.depth != t2.historyQos.depth)
	{
		//cout << "e"<<endl;
		return true;
	}
	return false;
};


} /* namespace fastrtps */
} /* namespace eprosima */

#endif /* TOPICPARAMETERS_H_ */
