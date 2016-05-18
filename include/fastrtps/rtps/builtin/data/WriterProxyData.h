/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WriterProxyData.h
 *
 */

#ifndef WRITERPROXYDATA_H_
#define WRITERPROXYDATA_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "../../../attributes/TopicAttributes.h"
#include "../../../qos/ParameterList.h"
#include "../../../qos/WriterQos.h"

#include "../../attributes/ReaderAttributes.h"

using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {

struct CDRMessage_t;

/**
 **@ingroup BUILTIN_MODULE
 */
class RTPS_DllAPI WriterProxyData {
public:
	WriterProxyData();
	virtual ~WriterProxyData();

	//!GUID
	GUID_t m_guid;
	//!Unicast locator list
	LocatorList_t m_unicastLocatorList;
	//!Multicast locator list
	LocatorList_t m_multicastLocatorList;
	//!GUID_t of the Writer converted to InstanceHandle_t
	InstanceHandle_t m_key;
	//!GUID_t of the participant converted to InstanceHandle
	InstanceHandle_t m_RTPSParticipantKey;
	//!Type name
	std::string m_typeName;
	//!Topic name
	std::string m_topicName;
	//!User defined ID
	uint16_t m_userDefinedId;
	//!WriterQOS
	WriterQos m_qos;
	//!Maximum size of the type associated with this Wrtiter, serialized.
	uint32_t m_typeMaxSerialized;
	//!Indicates if the Writer is Alive.
	bool m_isAlive;
	//!Topic kind
	TopicKind_t m_topicKind;
	//!
	ParameterList_t m_parameterList;
	//!Clear the information and return the object to the default state.
	void clear();
	//!Update certain parameters from another object.
	void update(WriterProxyData* rdata);
	//!Copy all information from another object.
	void copy(WriterProxyData* rdata);
	//!Convert the information to a parameter list to be send in a CDRMessage.
	bool toParameterList();
	//!Read a parameter list from a CDRMessage_t.
	bool readFromCDRMessage(CDRMessage_t* msg);
	/**
	* Convert the ProxyData information to RemoteWriterAttributes object.
	* @return Reference to the RemoteWriterAttributes object.
	*/
	RemoteWriterAttributes& toRemoteWriterAttributes();
	//!Remote Attributes associated with this proxy data.
	RemoteWriterAttributes m_remoteAtt;
};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif
#endif /* WRITERPROXYDATA_H_ */
