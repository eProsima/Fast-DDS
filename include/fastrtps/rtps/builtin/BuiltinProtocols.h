/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file BuiltinProtocols.h
 *
 */

#ifndef BUILTINPROTOCOLS_H_
#define BUILTINPROTOCOLS_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include "../attributes/RTPSParticipantAttributes.h"


namespace eprosima {

namespace fastrtps{

	class WriterQos;
	class ReaderQos;
	class TopicAttributes;

namespace rtps {

class PDPSimple;
class WLP;
class RTPSParticipantImpl;
class RTPSWriter;
class RTPSReader;

/**
 * Class BuiltinProtocols that contains builtin endpoints implementing the discovery and liveliness protocols.
 * *@ingroup BUILTIN_MODULE
 */
class BuiltinProtocols {
	friend class RTPSParticipantImpl;
private:
	BuiltinProtocols();
	virtual ~BuiltinProtocols();
public:
	/**
	 * Initialize the builtin protocols.
	 * @param attributes DiscoveryAttributes
	 * @param RTPSParticipantID ID of the local RTPSParticipant.
	 * @return True if correct.
	 */
	bool initBuiltinProtocols(RTPSParticipantImpl* p_part, BuiltinAttributes& attributes);
	
	/**
	 * Update the metatraffic locatorlist after it was created. Because when you create the EDP readers you are not sure the selected endpoints can be used.
	 * @param loclist LocatorList to update
	 * @return True on success
	 */
	bool updateMetatrafficLocators(LocatorList_t& loclist);

	//!BuiltinAttributes of the builtin protocols.
	BuiltinAttributes m_att;
	//!Pointer to the RTPSParticipantImpl.
	RTPSParticipantImpl* mp_participantImpl;
	//!Pointer to the PDPSimple.
	PDPSimple* mp_PDP;
	//!Pointer to the WLP
	WLP* mp_WLP;
	//!Port number for the multicast locators.
	uint32_t m_SPDP_WELL_KNOWN_MULTICAST_PORT;
	//!Port number for the unicast locators.
	uint32_t m_SPDP_WELL_KNOWN_UNICAST_PORT;
	//!Locator list for metatraffic
	LocatorList_t m_metatrafficMulticastLocatorList;
	//!Locator List for metatraffic unicast
	LocatorList_t m_metatrafficUnicastLocatorList;
	//!Mandatory Multicast Locator (239.255.0.1)
	Locator_t m_mandatoryMulticastLocator;
	//!Use the mandatory locator.
	bool m_useMandatory;

	/**
	 * Add a local Writer to the BuiltinProtocols.
	 * @param w Pointer to the RTPSWriter.
	 * @return True if correct.
	 */
	bool addLocalWriter(RTPSWriter* w,TopicAttributes& topicAtt,WriterQos& wqos);
	/**
	 * Add a local Reader to the BuiltinProtocols.
	 * @param R Pointer to the RTPSReader.
	 * @return True if correct.
	 */
	bool addLocalReader(RTPSReader* R,TopicAttributes& topicAtt, ReaderQos& rqos);
	/**
	 * Update a local Writer QOS
	 * @param W Writer to update
	 * @param qos New Writer QoS
	 * @return
	 */
	bool updateLocalWriter(RTPSWriter* W,WriterQos& wqos);
	/**
	 * Update a local Reader QOS
	 * @param R Reader to update
	 * @param qos New Reader QoS
	 * @return
	 */
	bool updateLocalReader(RTPSReader* R,ReaderQos& qos);
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

	//! Announce RTPSParticipantState (force the sending of a DPD message.)
	void announceRTPSParticipantState();
	//!Stop the RTPSParticipant Announcement (used in tests to avoid multiple packets being send)
	void stopRTPSParticipantAnnouncement();
	//!Reset to timer to make periodic RTPSParticipant Announcements.
	void resetRTPSParticipantAnnouncement();

};

}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* BUILTINPROTOCOLS_H_ */
