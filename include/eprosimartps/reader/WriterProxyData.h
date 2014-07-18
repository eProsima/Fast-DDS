/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterProxyData.h
 *
 */

#ifndef WRITERPROXYDATA_H_
#define WRITERPROXYDATA_H_
#include "eprosimartps/dds/attributes/TopicAttributes.h"
#include "eprosimartps/qos/ParameterList.h"
#include "eprosimartps/qos/WriterQos.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

struct CDRMessage_t;

class WriterProxyData {
public:
	WriterProxyData();
	virtual ~WriterProxyData();

	bool toParameterList();

	bool readFromCDRMessage(CDRMessage_t* msg);
	GUID_t m_guid;
	LocatorList_t m_unicastLocatorList;
	LocatorList_t m_multicastLocatorList;

	InstanceHandle_t m_key;
	InstanceHandle_t m_participantKey;
	std::string m_typeName;
	std::string m_topicName;
	uint16_t m_userDefinedId;
	WriterQos m_qos;
	uint32_t m_typeMaxSerialized;

	bool m_isAlive;
	TopicKind_t m_topicKind;


	ParameterList_t m_parameterList;

	void clear();
	void update(WriterProxyData* rdata);
	void copy(WriterProxyData* rdata);
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* WRITERPROXYDATA_H_ */
