/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ParticipantProxyData.h
 *
 */

#ifndef PARTICIPANTPROXYDATA_H_
#define PARTICIPANTPROXYDATA_H_

#include "eprosimartps/qos/QosList.h"
#include "eprosimartps/qos/ParameterList.h"

#include "eprosimartps/reader/WriterProxyData.h"
#include "eprosimartps/writer/ReaderProxyData.h"



#define DISCOVERY_PARTICIPANT_DATA_MAX_SIZE 500
#define DISCOVERY_TOPIC_DATA_MAX_SIZE 400
#define DISCOVERY_PUBLICATION_DATA_MAX_SIZE 600
#define DISCOVERY_SUBSCRIPTION_DATA_MAX_SIZE 600

#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER 0x00000001 << 0;
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR 0x00000001 << 1;
#define DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER 0x00000001 << 2;
#define DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR 0x00000001 << 3;
#define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER 0x00000001 << 4;
#define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR 0x00000001 << 5;
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_ANNOUNCER 0x00000001 << 6;
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_DETECTOR 0x00000001 << 7;
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_ANNOUNCER 0x00000001 << 8;
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_DETECTOR 0x00000001 << 9;
#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER 0x00000001 << 10;
#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER 0x00000001 << 11;

namespace eprosima {
namespace rtps {

struct CDRMessage_t;
class PDPSimple;
class RemoteParticipantLeaseDuration;
class ParticipantImpl;


class ParticipantProxyData {
public:
	ParticipantProxyData();
	virtual ~ParticipantProxyData();

	ProtocolVersion_t m_protocolVersion;
	GUID_t m_guid;
	VendorId_t m_VendorId;
	bool m_expectsInlineQos;
	BuiltinEndpointSet_t m_availableBuiltinEndpoints;
	LocatorList_t m_metatrafficUnicastLocatorList;
	LocatorList_t m_metatrafficMulticastLocatorList;
	LocatorList_t m_defaultUnicastLocatorList;
	LocatorList_t m_defaultMulticastLocatorList;
	Count_t m_manualLivelinessCount;
	std::string m_participantName;
	InstanceHandle_t m_key;
	Duration_t m_leaseDuration;
	bool isAlive;
	QosList_t m_QosList;
	ParameterPropertyList_t m_properties;
	bool m_hasChanged;
	RemoteParticipantLeaseDuration* mp_leaseDurationTimer;
	std::vector<ReaderProxyData*> m_readers;
	std::vector<WriterProxyData*> m_writers;
	std::vector<ReaderProxyData*> m_builtinReaders;
	std::vector<WriterProxyData*> m_builtinWriters;
	/**
	 * Initialize the object with the data of the lcoal participant.
	 * @param part Pointer to the participant.
	 * @param pdp Pointer to the PDPSimple object.
	 * @return True if correctly initialized.
	 */
	bool initializeData(ParticipantImpl* part, PDPSimple* pdp);
	/**
	 * Update the data.
	 * @param pdata
	 * @return
	 */
	bool updateData(ParticipantProxyData& pdata);
	//!Convert information to parameter list.
	bool toParameterList();
	//!Read the parameter list from a recevied CDRMessage_t
	bool readFromCDRMessage(CDRMessage_t* msg);
	//!Clear the data (restore to default state.)
	void clear();
	//!Copy the data from another object.
	void copy(ParticipantProxyData& pdata);


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PARTICIPANTPROXYDATA_H_ */
