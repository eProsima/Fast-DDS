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
#include "eprosimartps/qos/DDSQosPolicies.h"
#include "eprosimartps/utils/RTPSLog.h"

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

	HistoryQosPolicy historyQos;

	ResourceLimitsQosPolicy resourceLimitsQos;
	bool checkQos()
	{
		if(resourceLimitsQos.max_samples_per_instance > resourceLimitsQos.max_samples)
		{
			pError("INCORRECT TOPIC QOS:max_samples_per_instance must be <= than max_samples"<<endl);
			return false;
		}
		if(resourceLimitsQos.max_samples_per_instance*resourceLimitsQos.max_instances > resourceLimitsQos.max_samples)
			pWarning("TOPIC QOS: max_samples < max_samples_per_instance*max_instances"<<endl);
		if(historyQos.kind == KEEP_LAST_HISTORY_QOS)
		{
			if(historyQos.depth > resourceLimitsQos.max_samples)
			{
				pError("INCORRECT TOPIC QOS: depth must be <= max_samples"<<endl;)
				return false;
			}
			if(historyQos.depth > resourceLimitsQos.max_samples_per_instance)
			{
				pError("INCORRECT TOPIC QOS: depth must be <= max_samples_per_instance"<<endl;)
				return false;
			}
		}
		return true;
	}
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* TOPICPARAMETERS_H_ */
