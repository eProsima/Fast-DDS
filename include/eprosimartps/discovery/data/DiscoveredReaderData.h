/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DiscoveredReaderData.h
 *
*/

#ifndef DISCOVEREDREADERDATA_H_
#define DISCOVEREDREADERDATA_H_

#include "eprosimartps/qos/DDSQosPolicies.h"
#include "eprosimartps/writer/ReaderProxy.h"
#include "eprosimartps/dds/attributes/TopicAttributes.h"

#include "eprosimartps/qos/ReaderQos.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {




/**
 * Class DiscoveredReaderData used by the SEDP.
 * @ingroup DISCOVERYMODULE
 */
class DiscoveredReaderData {
public:
	DiscoveredReaderData():
		userDefinedId(-1),isAlive(false),topicKind(NO_KEY){};
	virtual ~DiscoveredReaderData(){};
	ReaderProxy_t m_readerProxy;
	InstanceHandle_t m_key;
	InstanceHandle_t m_participantKey;
	std::string m_typeName;
	std::string m_topicName;
	uint16_t userDefinedId;
ReaderQos m_qos;


	bool isAlive;
	TopicKind_t topicKind;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* DISCOVEREDREADERDATA_H_ */
