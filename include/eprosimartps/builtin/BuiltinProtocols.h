/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file BuiltinProtocols.h
 *
 */

#ifndef BUILTINPROTOCOLS_H_
#define BUILTINPROTOCOLS_H_

#include "eprosimartps/pubsub/attributes/ParticipantAttributes.h"


namespace eprosima {
namespace rtps {

class PDPSimple;
class WLP;
class ParticipantImpl;
class RTPSWriter;
class RTPSReader;

/**
 * Class BuiltinProtocols that contains builtin endpoints implementing the discovery and liveliness protocols.
 * @ingroup MANAGEMENTMODULE
 */
class BuiltinProtocols {
public:
	BuiltinProtocols();
	virtual ~BuiltinProtocols();
	/**
	 * Initialize the builtin protocols.
	 * @param attributes DiscoveryAttributes
	 * @param participantID ID of the local participant.
	 * @return True if correct.
	 */
	bool initBuiltinProtocols(ParticipantImpl* p_part,const BuiltinAttributes& attributes, uint32_t participantID);
	//!Update the metatraffic locatorlist after it was created. Because when you create the EDP readers you are not sure the selected endpoints can be used.
	bool updateMetatrafficLocators(LocatorList_t& loclist);

	BuiltinAttributes m_attributes;
	ParticipantImpl* mp_participant;
	PDPSimple* mp_PDP;
	WLP* mp_WLP;
	uint32_t m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	uint32_t m_SPDP_WELL_KNOWN_UNICAST_PORT;
	LocatorList_t m_metatrafficMulticastLocatorList;
	LocatorList_t m_metatrafficUnicastLocatorList;
	Locator_t m_mandatoryMulticastLocator;
	bool m_useMandatory;

	/**
	 * Add a local Writer to the BuiltinProtocols.
	 * @param w Pointer to the RTPSWriter.
	 * @return True if correct.
	 */
	bool addLocalWriter(RTPSWriter* w);
	/**
	 * Add a local Reader to the BuiltinProtocols.
	 * @param R Pointer to the RTPSReader.
	 * @return True if correct.
	 */
	bool addLocalReader(RTPSReader* R);
	/**
	 * Update a local Writer QOS (NOT IMPLEMENTED YET)
	 * @param W
	 * @return
	 */
	bool updateLocalWriter(RTPSWriter* W);
	/**
	 * Update a local Reader QOS (NOT IMPLEMENTED YET)
	 * @param R
	 * @return
	 */
	bool updateLocalReader(RTPSReader* R);
	/**
	 * Remove a local Writer from the builtinProtocols.
	 * @param W Pointer to the writer.
	 * @return True if correctly removed.
	 */
	bool removeLocalWriter(RTPSWriter* W);
	/**
	 * Remove a local Reader from the builtinProtocols.
	 * @param R Pointer to the reader.
	 * @return True if correctly removed.
	 */
	bool removeLocalReader(RTPSReader* R);

	//! Announce ParticipantState (force the sending of a DPD message.)
	void announceParticipantState();
	//!Stop the Participant Announcement (used in tests to avoid multiple packets being send)
	void stopParticipantAnnouncement();
	//!Reset to timer to make periodic Participant Announcements.
	void resetParticipantAnnouncement();

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* BUILTINPROTOCOLS_H_ */
