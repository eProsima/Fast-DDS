/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimplePDP.h
 *
 */

#ifndef SIMPLEDPD_H_
#define SIMPLEDPD_H_

#include "eprosimartps/discovery/ParticipantDiscoveryProtocol.h"
#include "eprosimartps/dds/attributes/ParticipantAttributes.h"

#include "eprosimartps/discovery/SPDPListener.h"
#include "eprosimartps/qos/QosList.h"
#include "eprosimartps/discovery/SimpleEDP.h"
#include "eprosimartps/discovery/StaticEDP.h"

namespace eprosima {
namespace rtps {

class StatelessWriter;
class StatelessReader;
class ParticipantImpl;
class ResendDiscoveryDataPeriod;


/**
 * SimpleParticipantDiscoveryProtocol class (SimplePDP), used for the Participant Discovery.
 * @ingroup DISCOVERYMODULE
 */
class SimplePDP: public eprosima::rtps::ParticipantDiscoveryProtocol {
public:
	SimplePDP(ParticipantImpl* p_part);
	virtual ~SimplePDP();
	/**
	 * Initialization method.
	 * @param attributes Discovery Attributes structure, indicates the type of discovery as well as some other parameters.
	 * @param participantID The ID of the Participant, used to calculate the ports numbers.
	 * @return True if correct.
	 */
	bool initPDP(const DiscoveryAttributes& attributes,uint32_t participantID);
	/**
	 * Update the local Participant DPD object when something has changed.
	 * @return
	 */
	bool updateLocalParticipantData();

	uint32_t m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	uint32_t m_SPDP_WELL_KNOWN_UNICAST_PORT;

	/**
	 * Create the SPDP Writer and Reader
	 * @return True if correct.
	 */
	bool createSPDPEndpoints();

	StatelessWriter* mp_SPDPWriter;
	StatelessReader* mp_SPDPReader;
	SPDPListener m_listener;
	bool m_hasChangedLocalPDP;

	//!Force the sending of our local DPD to all remote Participants and multicast Locators.
	void announceParticipantState(bool new_change);

	//!Stop the ParticipantAnnouncement (only used in tests).
	void stopParticipantAnnouncement();
	//!Reset the ParticipantAnnouncement (only used in tests).
	void resetParticipantAnnouncement();

	//!Update the ParameterList for our local DPD object.
	bool updateParameterList();


	QosList_t m_localDPDasQosList;

	//!Only used for StaticEDP, to add the Endpoins EntityId as a PropertyList.
	bool addStaticEDPInfo();

	//! Set the variable indicated that something has changed in the Participant.
	void localParticipantHasChanged();

	/**
	 * Call the same method in the EDP.
	 * @param W Pointer to the Writer.
	 * @param first_time Bool indicating if is the first time.
	 * @return True if matched.
	 */
	bool localWriterMatching(RTPSWriter* W,bool first_time);
	/**
	 * Call the same method in the EDP.
	 * @param R Pointer to the Reader.
	 * @param first_time Bool indicating if is the first time.
	 * @return True if matched.
	 */
	bool localReaderMatching(RTPSReader* R,bool first_time);

	//!Pointer to the TimedEvent used to periodically send the ParticipantDiscoveredData
	ResendDiscoveryDataPeriod* m_resendDataTimer;


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SIMPLEDPD_H_ */
