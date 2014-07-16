/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ReaderProxyData.h
 *
 */

#ifndef READERPROXYDATA_H_
#define READERPROXYDATA_H_

#include "eprosimartps/qos/ParameterList.h"
#include "eprosimartps/qos/ReaderQos.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

class CDRMessage_t;

class ReaderProxyData {
public:
	ReaderProxyData();
	virtual ~ReaderProxyData();
	bool toParameterList();

	bool readFromCDRMessage(CDRMessage_t* msg);

	GUID_t m_guid;
	bool m_expectsInlineQos;
	LocatorList_t m_unicastLocatorList;
	LocatorList_t m_multicastLocatorList;
	InstanceHandle_t m_key;
	InstanceHandle_t m_participantKey;
	std::string m_typeName;
	std::string m_topicName;
	uint16_t m_userDefinedId;
	ReaderQos m_qos;
	bool m_isAlive;
	TopicKind_t m_topicKind;
private:
	ParameterList_t m_parameterList;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* READERPROXYDATA_H_ */
