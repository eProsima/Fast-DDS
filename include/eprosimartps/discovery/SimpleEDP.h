/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file SimpleEDP.h
 *
*/

#ifndef SIMPLEEDP_H_
#define SIMPLEEDP_H_

#include "eprosimartps/common/types/Guid.h"
#include "eprosimartps/discovery/EndpointDiscoveryProtocol.h"
#include "eprosimartps/discovery/SEDPListeners.h"

#include "eprosimartps/dds/attributes/ParticipantAttributes.h"

#include "eprosimartps/discovery/SEDPTopicDataType.h"

namespace eprosima {
namespace rtps {

class StatefulReader;
class StatefulWriter;
class RTPSWriter;
class RTPSReader;
class ParticipantDiscoveryProtocol;
class DiscoveredReaderData;
class DiscoveredWriterData;
class DiscoveredParticipantData;


/**
 * Simple EndpointDiscoveryProtocol class, implementation of the SEDP defined in the RTPS document.
 * @ingroup DISCOVERYMODULE
 */
class SimpleEDP: public eprosima::rtps::EndpointDiscoveryProtocol {
	friend class SEDPPubListener;
	friend class SEDPSubListener;
public:
	SimpleEDP(ParticipantDiscoveryProtocol* p);
	virtual ~SimpleEDP();
	/**
	 * Initialization method.
	 * @param attributes Reference to the DiscoveryAttributes.
	 * @return True if correct.
	 */
	bool initEDP(DiscoveryAttributes& attributes);

	DiscoveryAttributes m_discovery;

	//!Pointer to the Publications Writer (only created if indicated in the DiscoveryAtributes).
	StatefulWriter* mp_PubWriter;
	//!Pointer to the Subscriptions Writer (only created if indicated in the DiscoveryAtributes).
	StatefulWriter* mp_SubWriter;
	//!Pointer to the Publications Reader (only created if indicated in the DiscoveryAtributes).
	StatefulReader* mp_PubReader;
	//!Pointer to the Subscriptions Reader (only created if indicated in the DiscoveryAtributes).
	StatefulReader* mp_SubReader;

	/**
	 * Match a local Writer against all possible remote and local Readers with the same topic.
	 * @param writer Pointer to the Writer.
	 * @param first_time Whether or not is the first time (to create the corresponding DWD object).
	 * @return True if correct.
	 */
	bool localWriterMatching(RTPSWriter* writer,bool first_time);
	/**
	 * Match a local Reader against all possible remote and local Readers with the same topic.
	 * @param reader Pointer to the Reader.
	 * @param first_time Whether or not is the first time (to create the corresponding DRD object).
	 * @return True if correct.
	 */
	bool localReaderMatching(RTPSReader* reader,bool first_time);
	/**
	 * Try to pair a local Writer with a specific DRD object.
	 * @param writer Pointer to the writer.
	 * @param rdata Pointer to the DRD object.
	 * @return True if matched.
	 */
	bool pairLocalWriterDiscoveredReader(RTPSWriter* writer,DiscoveredReaderData* rdata);
	/**
	 * Try to pair a local Reader with a specific DWD object.
	 * @param reader Pointer to the reader.
	 * @param wdata Pointer to the DWD object.
	 * @return True if matched.
	 */
	bool pairLocalReaderDiscoveredWriter(RTPSReader* reader,DiscoveredWriterData* wdata);

	/**
	 * Method to update a specific matching when something changes (Qos, for example) or when the discovered reader is removed (NOT YET IMPLEMENTED).
	 * @param writer Pointer to the Writer.
	 * @param rdata Pointer to the DRD object.
	 * @return True if correct.
	 */
	bool updateWriterMatching(RTPSWriter* writer,DiscoveredReaderData* rdata);
	/**
	 * Method to update a specific matching when something changes (Qos, for example) or when the discovered writer is removed (NOT YET IMPLEMENTED).
	 * @param reader Pointer to the reader.
	 * @param wdata Pointer to the DWD object.
	 * @return True if correct.
	 */
	bool updateReaderMatching(RTPSReader* reader,DiscoveredWriterData* wdata);

	/**
	 * Create local SEDP Endpoints based on the DiscoveryAttributes.
	 * @return True if correct.
	 */
	bool createSEDPEndpoints();

	/**
	 * Assign the remote Endpoint to our local SEDP endpoints when a new participant is discovered.
	 * @param pdata Pointer to the DPD object of the discovered Participant.
	 */
	void assignRemoteEndpoints(DiscoveredParticipantData* pdata);

	/**
	 * Add a new local Writer (create a DWD object) to communicate its existence to remote participants.
	 * @param W Pointer to the Writer.
	 * @return True if correct.
	 */
	bool addNewLocalWriter(RTPSWriter* W);
	/**
	 * Add a new local Reader (create a DRD object) to communicate its existence to remote participants.
	 * @param R Pointer to the Reader.
	 * @return True if correct.
	 */
	bool addNewLocalReader(RTPSReader* R);

	//!SEDPListener object.
	SEDPListeners m_listeners;

	//! Method to repare the LocatorList when something changes (the previously defined port couldn't be used because it was already in use.)
	void repareDiscoveredDataLocatorList(LocatorList_t* loclist);

	bool removeRemoteEndpoints(const GuidPrefix_t& partguidP);

	SEDPTopicDataType m_PubReaderDataType;
	SEDPTopicDataType m_SubReaderDataType;
};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* SIMPLEEDP_H_ */
