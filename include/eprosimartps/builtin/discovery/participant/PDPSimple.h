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
#include "eprosimartps/dds/attributes/ParticipantAttributes.h"

namespace eprosima {
namespace rtps {

class StatelessWriter;
class StatelessReader;
class ParticipantImpl;
class BuiltinProtocols;
class EDP;
class ResendParticipantProxyDataPeriod;
class RemoteParticipantLeaseDuration;


/**
 * Class PDPSimple that implements the SimpleParticipantDiscoveryProtocol as defined in the RTPS specification.
 * @ingroup DISCOVERYMODULE
 */
class PDPSimple {
	friend class ResendParticipantProxyDataPeriod;
	friend class RemoteParticipantLeaseDuration;
	friend class PDPSimpleListener;
public:
	PDPSimple(BuiltinProtocols* builtin);
	virtual ~PDPSimple();
	/**
	 * Initialize the PDP.
	 * @param part Pointer to the participant.
	 * @param participantID participantID used to create the participant.
	 * @return
	 */
	bool initPDP(ParticipantImpl* part,uint32_t participantID);

	//!Force the sending of our local DPD to all remote Participants and multicast Locators.
	/**
	 * Force the sending of our local DPD to all remote Participants and multicast Locators.
	 * @param new_change If true a new change (with new seqNum) is created and sent; if false the last change is re-sent
	 */
	void announceParticipantState(bool new_change);
	//!Stop the ParticipantAnnouncement (only used in tests).
	void stopParticipantAnnouncement();
	//!Reset the ParticipantAnnouncement (only used in tests).
	void resetParticipantAnnouncement();

	/**
	 *
	 * @param rdata
	 * @param copydata
	 * @param returnReaderProxyData
	 * @param pdata
	 * @return
	 */
	bool addReaderProxyData(ReaderProxyData* rdata,bool copydata=false,
			ReaderProxyData** returnReaderProxyData=NULL,
			ParticipantProxyData** pdata = NULL);

	bool addWriterProxyData(WriterProxyData* wdata,bool copydata=false,
			WriterProxyData** returnWriterProxyData=NULL,
			ParticipantProxyData** pdata = NULL);

	/**
	 * This method returns a pointer to a ReaderProxyData object if it is found among the registered participants (including the local participant).
	 * @param[in] reader GUID_t of the reader we are looking for.
	 * @param[out] rdata Pointer to pointer of the ReaderProxyData object.
	 * @return True if found.
	 */
	bool lookupReaderProxyData(const GUID_t& reader,ReaderProxyData** rdata);
	/**
	 * This method returns a pointer to a WriterProxyData object if it is found among the registered participants (including the local participant).
	 * @param[in] writer GUID_t of the writer we are looking for.
	 * @param[out] rdata Pointer to pointer of the WriterProxyData object.
	 * @return True if found.
	 */
	bool lookupWriterProxyData(const GUID_t& writer,WriterProxyData** rdata);
	/**
	 * This method removes and deletes a ReaderProxyData object from its corresponding participant.
	 * @param rdata Pointer to the ReaderProxyData object.
	 * @return true if found and deleted.
	 */
	bool removeReaderProxyData(ReaderProxyData* rdata);
	/**
	 * This method removes and deletes a WriterProxyData object from its corresponding participant.
	 * @param wdata Pointer to the WriterProxyData object.
	 * @return true if found and deleted.
	 */
	bool removeWriterProxyData(WriterProxyData* wdata);

	/**
	 * This method assigns remtoe endpoints to the builtin endpoints defined in this protocol. It also calls the corresponding methods in EDP and WLP.
	 * @param pdata Pointer to the ParticipantProxyData object.
	 */
	void assignRemoteEndpoints(ParticipantProxyData* pdata);

	void removeRemoteEndpoints(ParticipantProxyData* pdata);

	/**
	 * This method removes a remote participant and all its writers and readers.
	 * @param partGUID GUID_t of the remote participant.
	 * @return true if correct.
	 */
	bool removeRemoteParticipant(GUID_t& partGUID);
	//!Pointer to the builtin protocols object.
	BuiltinProtocols* mp_builtin;
	/**
	 * Get a pointer to the local participant ParticipantProxyData object.
	 * @return Pointer.
	 */
	ParticipantProxyData* getLocalParticipantProxyData()
	{
		return m_participantProxies.front();
	}
	/**
	 * Get a pointer to the EDP object.
	 * @return
	 */
	EDP* getEDP(){return mp_EDP;}
	/**
	 * Get a cons_iterator to the beginning of the participant Proxies.
	 * @return const_iterator.
	 */
	std::vector<ParticipantProxyData*>::const_iterator participantProxiesBegin(){return m_participantProxies.begin();};
	/**
	 * Get a cons_iterator to the end participant Proxies.
	 * @return const_iterator.
	 */
	std::vector<ParticipantProxyData*>::const_iterator participantProxiesEnd(){return m_participantProxies.end();};

	void assertRemoteParticipantLiveliness(GuidPrefix_t& guidP);

	void assertLocalWritersLiveliness(LivelinessQosPolicyKind kind);

	void assertRemoteWritersLiveliness(GuidPrefix_t& guidP,LivelinessQosPolicyKind kind);

private:
	//!Pointer to the local participant.
	ParticipantImpl* mp_participant;
	//!Discovery attributes.
	BuiltinAttributes m_discovery;
	//!Pointer to the SPDPWriter.
	StatelessWriter* mp_SPDPWriter;
	//!Pointer to the SPDPReader.
	StatelessReader* mp_SPDPReader;
	//!Pointer to the EDP object.
	EDP* mp_EDP;
	//!Registered participants (including the local one, that is the first one.)
	std::vector<ParticipantProxyData*> m_participantProxies;
	//!Variable to indicate if any parameter has changed.
	bool m_hasChangedLocalPDP;
	//!TimedEvent to periodically resend the local participant information.
	ResendParticipantProxyDataPeriod* mp_resendParticipantTimer;
	//!Listener for the SPDP messages.
	PDPSimpleListener m_listener;
	//!TopicDataType object to extract the key from unregistering messages.
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
