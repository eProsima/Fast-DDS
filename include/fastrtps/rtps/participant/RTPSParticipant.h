/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSParticipant.h
 */


#ifndef RTPSParticipant_H_
#define RTPSParticipant_H_

#include <cstdlib>
#include "fastrtps/config/fastrtps_dll.h"
#include "fastrtps/rtps/common/Guid.h"

namespace eprosima {
namespace fastrtps{

class TopicAttributes;
class WriterQos;
class ReaderQos;

namespace rtps {

class RTPSParticipantImpl;
class RTPSWriter;
class RTPSReader;


/**
 * @brief Class RTPSParticipant, contains the public API for a RTPSParticipant.
 * @ingroup MANAGEMENTMODULE
 */
class RTPS_DllAPI RTPSParticipant
{
	friend class RTPSParticipantImpl;
public:
	/**
	* @param pimpl Implementation
	*/
	RTPSParticipant(RTPSParticipantImpl* pimpl);
	virtual ~ RTPSParticipant();
	//!Get the GUID_t of the RTPSParticipant.
	const GUID_t& getGuid() const ;
	//!Force the announcement of the RTPSParticipant state.
	void announceRTPSParticipantState();
	//!Method to loose the next change (ONLY FOR TEST).
	void loose_next_change();
	//!Stop the RTPSParticipant announcement period.
	void stopRTPSParticipantAnnouncement();
	//!Reset the RTPSParticipant announcement period.
	void resetRTPSParticipantAnnouncement();
	/**
	 * Indicate the Participant that you have discovered a new Remote Writer.
	 * This method can be used by the user to implements its own Static Endpoint
	 * Discovery Protocol
	 * @param pguid GUID_t of the discovered Writer.
	 * @param userDefinedId ID of the discovered Writer.
	 * @return True if correctly added.
	 */
	bool newRemoteWriterDiscovered(const GUID_t& pguid, int16_t userDefinedId);
	/**
	 * Indicate the Participant that you have discovered a new Remote Reader.
	 * This method can be used by the user to implements its own Static Endpoint
	 * Discovery Protocol
	 * @param pguid GUID_t of the discovered Reader.
	 * @param userDefinedId ID of the discovered Reader.
	 * @return True if correctly added.
	 */
	bool newRemoteReaderDiscovered(const GUID_t& pguid, int16_t userDefinedId);
	/**
	 * Get the Participant ID.
	 * @return Participant ID.
	 */
	uint32_t getRTPSParticipantID() const;
	/**
	 * Register a RTPSWriter in the builtin Protocols.
	 * @param Writer Pointer to the RTPSWriter.
	 * @param topicAtt Topic Attributes where you want to register it.
	 * @param wqos WriterQos.
	 * @return True if correctly registered.
	 */
	bool registerWriter(RTPSWriter* Writer,TopicAttributes& topicAtt,WriterQos& wqos);
	/**
	 * Register a RTPSReader in the builtin Protocols.
	 * @param Reader Pointer to the RTPSReader.
	 * @param topicAtt Topic Attributes where you want to register it.
	 * @param rqos ReaderQos.
	 * @return True if correctly registered.
	 */
	bool registerReader(RTPSReader* Reader,TopicAttributes& topicAtt,ReaderQos& wqos);
	/**
	 * Update writer QOS
	 * @param Writer to update
	 * @param wqos New writer QoS
	 * return true on success
	 */
	bool updateWriter(RTPSWriter* Writer,WriterQos& wqos);
	/**
	 * Update reader QOS
	 * @param Reader to update
	 * @param rqos New reader QoS
	 * return true on success
	 */
	bool updateReader(RTPSReader* Reader,ReaderQos& rqos);

private:
	RTPSParticipantImpl* mp_impl;
};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSParticipant_H_ */





