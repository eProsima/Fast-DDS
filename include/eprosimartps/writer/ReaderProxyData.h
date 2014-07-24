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
#include "eprosimartps/dds/attributes/TopicAttributes.h"
#include "eprosimartps/qos/ParameterList.h"
#include "eprosimartps/qos/ReaderQos.h"

using namespace eprosima::dds;

namespace eprosima {
namespace rtps {

struct CDRMessage_t;

/**
 * Class ReaderProxyData, used to represent all the information on a Reader (both local and remote) with the purpose of
 * implementing the discovery.
 */
class ReaderProxyData {
public:
	ReaderProxyData();
	virtual ~ReaderProxyData();
	/**
	 * Convert the data to a parameter list to send this information as a RTPS message.
	 * @return true if correct.
	 */
	bool toParameterList();
	/**
	 *  Read the information from a CDRMessage_t. The position of hte message must be in the beggining on the parameter list.
	 * @param msg Pointer to the message.
	 * @return
	 */
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

	ParameterList_t m_parameterList;
	/**
	 * Clear (put to default) the information.
	 */
	void clear();
	/**
	 * Update the information (only certain fields can be updated).
	 * @param rdata Poitner to the object from which we are going to update.
	 */
	void update(ReaderProxyData* rdata);
	/**
	 * Copy ALL the information from another object.
	 * @param rdata Pointer to the object from where the information must be copied.
	 */
	void copy(ReaderProxyData* rdata);
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* READERPROXYDATA_H_ */
