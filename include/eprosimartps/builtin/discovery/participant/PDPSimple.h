/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PDPSimple.h
 *
 */

#ifndef PDPSIMPLE_H_
#define PDPSIMPLE_H_

#include "eprosimartps/builtin/discovery/participant/PDPSimpleListener.h"
#include "eprosimartps/builtin/discovery/participant/PDPSimpleTopicDataType.h"

namespace eprosima {
namespace rtps {

class StatelessWriter;
class StatelessReader;
class ParticipantImpl;
class BuiltinProtocols;
class EDP;
class ResendParticipantProxyDataPeriod;

class PDPSimple {
public:
	PDPSimple(BuiltinProtocols* builtin);
	virtual ~PDPSimple();

	bool initPDP(ParticipantImpl* part,uint32_t participantID);

	//!Force the sending of our local DPD to all remote Participants and multicast Locators.
	void announceParticipantState(bool new_change);
	//!Stop the ParticipantAnnouncement (only used in tests).
	void stopParticipantAnnouncement();
	//!Reset the ParticipantAnnouncement (only used in tests).
	void resetParticipantAnnouncement();

private:
	BuiltinProtocols* mp_builtin;
	ParticipantImpl* mp_participant;
	DiscoveryAttributes m_discovery;

	StatelessWriter* mp_SPDPWriter;
	StatelessReader* mp_SPDPReader;

	EDP* mp_EDP;
	std::vector<ParticipantProxyData*> m_participantProxies;
	bool m_hasChangedLocalPDP;

	ResendParticipantProxyDataPeriod* mp_resendParticipantTimer;
	PDPSimpleListener m_listener;
	PDPSimpleTopicDataType m_topicDataType;

	/**
	 * Create the SPDP Writer and Reader
	 * @return True if correct.
	 */
	bool createSPDPEndpoints();



};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* PDPSIMPLE_H_ */
