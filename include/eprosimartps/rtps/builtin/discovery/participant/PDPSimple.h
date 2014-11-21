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

#include "eprosimartps/rtps/builtin/discovery/participant/PDPSimpleListener.h"
#include "eprosimartps/rtps/builtin/discovery/participant/PDPSimpleTopicDataType.h"
#include "eprosimartps/rtps/attributes/RTPSParticipantAttributes.h"

namespace eprosima {
namespace rtps {

class StatelessWriter;
class StatelessReader;
class RTPSParticipantImpl;
class BuiltinProtocols;
class EDP;
class ResendRTPSParticipantProxyDataPeriod;
class RemoteRTPSParticipantLeaseDuration;


/**
 * Class PDPSimple that implements the SimpleRTPSParticipantDiscoveryProtocol as defined in the RTPS specification.
 * @ingroup DISCOVERYMODULE
 */
class PDPSimple {
	friend class ResendRTPSParticipantProxyDataPeriod;
	friend class RemoteRTPSParticipantLeaseDuration;
	friend class PDPSimpleListener;
public:
	PDPSimple(BuiltinProtocols* builtin);
	virtual ~PDPSimple();
	/**
	 * Initialize the PDP.
	 * @param part Pointer to the RTPSParticipant.
	 * @param RTPSParticipantID RTPSParticipantID used to create the RTPSParticipant.
	 * @return
	 */
	bool initPDP(RTPSParticipantImpl* part);

	//!Force the sending of our local DPD to all remote RTPSParticipants and multicast Locators.
	/**
	 * Force the sending of our local DPD to all remote RTPSParticipants and multicast Locators.
	 * @param new_change If true a new change (with new seqNum) is created and sent; if false the last change is re-sent
	 */
	void announceRTPSParticipantState(bool new_change);
	//!Stop the RTPSParticipantAnnouncement (only used in tests).
	void stopRTPSParticipantAnnouncement();
	//!Reset the RTPSParticipantAnnouncement (only used in tests).
	void resetRTPSParticipantAnnouncement();

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
			RTPSParticipantProxyData** pdata = NULL);

	bool addWriterProxyData(WriterProxyData* wdata,bool copydata=false,
			WriterProxyData** returnWriterProxyData=NULL,
			RTPSParticipantProxyData** pdata = NULL);

	/**
	 * This method returns a pointer to a ReaderProxyData object if it is found among the registered RTPSParticipants (including the local RTPSParticipant).
	 * @param[in] reader GUID_t of the reader we are looking for.
	 * @param[out] rdata Pointer to pointer of the ReaderProxyData object.
	 * @return True if found.
	 */
	bool lookupReaderProxyData(const GUID_t& reader,ReaderProxyData** rdata);
	/**
	 * This method returns a pointer to a WriterProxyData object if it is found among the registered RTPSParticipants (including the local RTPSParticipant).
	 * @param[in] writer GUID_t of the writer we are looking for.
	 * @param[out] rdata Pointer to pointer of the WriterProxyData object.
	 * @return True if found.
	 */
	bool lookupWriterProxyData(const GUID_t& writer,WriterProxyData** rdata);
	/**
	 * This method returns a pointer to a RTPSParticipantProxyData object if it is found among the registered RTPSParticipants.
	 * @param[in] pguid GUID_t of the RTPSParticipant we are looking for.
	 * @param[out] pdata Pointer to pointer of the RTPSParticipantProxyData object.
	 * @return True if found.
	 */
	bool lookupRTPSParticipantProxyData(const GUID_t& pguid,RTPSParticipantProxyData** pdata);
	/**
	 * This method removes and deletes a ReaderProxyData object from its corresponding RTPSParticipant.
	 * @param rdata Pointer to the ReaderProxyData object.
	 * @return true if found and deleted.
	 */
	bool removeReaderProxyData(ReaderProxyData* rdata);
	/**
	 * This method removes and deletes a WriterProxyData object from its corresponding RTPSParticipant.
	 * @param wdata Pointer to the WriterProxyData object.
	 * @return true if found and deleted.
	 */
	bool removeWriterProxyData(WriterProxyData* wdata);

	/**
	 * This method assigns remtoe endpoints to the builtin endpoints defined in this protocol. It also calls the corresponding methods in EDP and WLP.
	 * @param pdata Pointer to the RTPSParticipantProxyData object.
	 */
	void assignRemoteEndpoints(RTPSParticipantProxyData* pdata);

	void removeRemoteEndpoints(RTPSParticipantProxyData* pdata);

	/**
	 * This method removes a remote RTPSParticipant and all its writers and readers.
	 * @param partGUID GUID_t of the remote RTPSParticipant.
	 * @return true if correct.
	 */
	bool removeRemoteRTPSParticipant(GUID_t& partGUID);
	//!Pointer to the builtin protocols object.
	BuiltinProtocols* mp_builtin;
	/**
	 * Get a pointer to the local RTPSParticipant RTPSParticipantProxyData object.
	 * @return Pointer.
	 */
	RTPSParticipantProxyData* getLocalRTPSParticipantProxyData()
	{
		return m_RTPSParticipantProxies.front();
	}
	/**
	 * Get a pointer to the EDP object.
	 * @return
	 */
	EDP* getEDP(){return mp_EDP;}
	/**
	 * Get a cons_iterator to the beginning of the RTPSParticipant Proxies.
	 * @return const_iterator.
	 */
	std::vector<RTPSParticipantProxyData*>::const_iterator RTPSParticipantProxiesBegin(){return m_RTPSParticipantProxies.begin();};
	/**
	 * Get a cons_iterator to the end RTPSParticipant Proxies.
	 * @return const_iterator.
	 */
	std::vector<RTPSParticipantProxyData*>::const_iterator RTPSParticipantProxiesEnd(){return m_RTPSParticipantProxies.end();};

	void assertRemoteRTPSParticipantLiveliness(GuidPrefix_t& guidP);

	void assertLocalWritersLiveliness(LivelinessQosPolicyKind kind);

	void assertRemoteWritersLiveliness(GuidPrefix_t& guidP,LivelinessQosPolicyKind kind);

	bool newRemoteEndpointStaticallyDiscovered(const GUID_t& pguid, int16_t userDefinedId,EndpointKind_t kind);

	RTPSParticipantImpl* getRTPSParticipant() const {return mp_RTPSParticipant;};

private:
	//!Pointer to the local RTPSParticipant.
	RTPSParticipantImpl* mp_RTPSParticipant;
	//!Discovery attributes.
	BuiltinAttributes m_discovery;
	//!Pointer to the SPDPWriter.
	StatelessWriter* mp_SPDPWriter;
	//!Pointer to the SPDPReader.
	StatelessReader* mp_SPDPReader;
	//!Pointer to the EDP object.
	EDP* mp_EDP;
	//!Registered RTPSParticipants (including the local one, that is the first one.)
	std::vector<RTPSParticipantProxyData*> m_RTPSParticipantProxies;
	//!Variable to indicate if any parameter has changed.
	bool m_hasChangedLocalPDP;
	//!TimedEvent to periodically resend the local RTPSParticipant information.
	ResendRTPSParticipantProxyDataPeriod* mp_resendRTPSParticipantTimer;
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
