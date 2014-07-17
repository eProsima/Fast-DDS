/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DiscoveredWriterData.h
 *
 */

#ifndef DISCOVEREDWRITERDATA_H_
#define DISCOVEREDWRITERDATA_H_

#include "eprosimartps/qos/DDSQosPolicies.h"
#include "eprosimartps/reader/WriterProxy.h"
#include "eprosimartps/dds/attributes/TopicAttributes.h"


#include "eprosimartps/qos/WriterQos.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {




/**
 * Class DiscoveredWriterData used by the SEDP.
 * @ingroup DISCOVERYMODULE
 */
class DiscoveredWriterData {
public:
	DiscoveredWriterData():
		userDefinedId(-1),m_typeMaxSerialized(0),isAlive(false),topicKind(NO_KEY){};
	virtual ~DiscoveredWriterData(){};
	WriterProxy_t m_writerProxy;
	InstanceHandle_t m_key;
	InstanceHandle_t m_participantKey;
	std::string m_typeName;
	std::string m_topicName;
	uint16_t userDefinedId;
	WriterQos m_qos;
	uint32_t m_typeMaxSerialized;

	bool isAlive;
	TopicKind_t topicKind;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* DISCOVEREDWRITERDATA_H_ */
