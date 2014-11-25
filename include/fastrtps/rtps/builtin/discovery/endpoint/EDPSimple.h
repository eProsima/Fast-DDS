/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPSimple.h
 *
 */

#ifndef EDPSIMPLE_H_
#define EDPSIMPLE_H_

#include "fastrtps/rtps/builtin/discovery/endpoint/EDP.h"
//#include "fastrtps/rtps/builtin/discovery/endpoint/EDPSimpleListeners.h"
//#include "fastrtps/rtps/builtin/discovery/endpoint/EDPSimpleTopicDataType.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

class StatefulReader;
class StatefulWriter;
class RTPSWriter;
class RTPSReader;

/**
 * Class EDPSimple, implements the SimpleEndpointDiscoveryProtocol defined in the RTPS specification. Inherits from EDP class.
 * @ingroup DISCOVERYMODULE
 */
class EDPSimple : public EDP {
public:
	EDPSimple(PDPSimple* p,RTPSParticipantImpl* part);
	virtual ~EDPSimple();
	//!Discovery attributes.
	BuiltinAttributes m_discovery;
	//!Pointer to the Publications Writer (only created if indicated in the DiscoveryAtributes).
	StatefulWriter* mp_PubWriter;
	//!Pointer to the Subscriptions Writer (only created if indicated in the DiscoveryAtributes).
	StatefulWriter* mp_SubWriter;
	//!Pointer to the Publications Reader (only created if indicated in the DiscoveryAtributes).
	StatefulReader* mp_PubReader;
	//!Pointer to the Subscriptions Reader (only created if indicated in the DiscoveryAtributes).
	StatefulReader* mp_SubReader;
//	//!EDPSimpleListeners object, contains two listeners for the Publication and Subscription readers.
//	EDPSimpleListeners* m_listeners;
//	//!EDPSimpleTopicDataType to extract the key from unregistering and disposing messages.
//	EDPSimpleTopicDataType* m_pubReaderTopicDataType;
//	//!EDPSimpleTopicDataType to extract the key from unregistering and disposing messages.
//	EDPSimpleTopicDataType* m_subReaderTopicDataType;


	/**
	 * Initialization method.
	 * @param attributes Reference to the DiscoveryAttributes.
	 * @return True if correct.
	 */
	bool initEDP(BuiltinAttributes& attributes);
	/**
	 * This method assigns the remote builtin endpoints that the remote RTPSParticipant indicates is using to our local builtin endpoints.
	 * @param pdata Pointer to the RTPSParticipantProxyData object.
	 */
	void assignRemoteEndpoints(ParticipantProxyData* pdata);

	void removeRemoteEndpoints(ParticipantProxyData* pdata);

	/**
	 * Create local SEDP Endpoints based on the DiscoveryAttributes.
	 * @return True if correct.
	 */
	bool createSEDPEndpoints();
	/**
	 * This method generates the corresponding change in the subscription writer and send it to all known remote endpoints.
	 * @param rdata Pointer to the ReaderProxyData object.
	 * @return true if correct.
	 */
	bool processLocalReaderProxyData(ReaderProxyData* rdata);
	/**
	 * This method generates the corresponding change in the publciations writer and send it to all known remote endpoints.
	 * @param wdata Pointer to the WriterProxyData object.
	 * @return true if correct.
	 */
	bool processLocalWriterProxyData(WriterProxyData* wdata);
	/**
	 * This methods generates the change disposing of the local Reader and calls the unpairing and removal methods of the base class.
	 * @param R Pointer to the RTPSReader object.
	 * @return True if correct.
	 */
	bool removeLocalReader(RTPSReader*R);
	/**
	 * This methods generates the change disposing of the local Writer and calls the unpairing and removal methods of the base class.
	 * @param W Pointer to the RTPSWriter object.
	 * @return True if correct.
	 */
	bool removeLocalWriter(RTPSWriter*W);

};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* EDPSIMPLE_H_ */
