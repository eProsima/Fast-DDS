/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPSimple.h
 *
 */

#ifndef EDPSIMPLE_H_
#define EDPSIMPLE_H_

#include "eprosimartps/builtin/discovery/endpoint/EDP.h"

#include "eprosimartps/builtin/discovery/endpoint/EDPSimpleListeners.h"
#include "eprosimartps/builtin/discovery/endpoint/EDPSimpleTopicDataType.h"

namespace eprosima {
namespace rtps {

class StatefulReader;
class StatefulWriter;
class RTPSWriter;
class RTPSReader;

class EDPSimple : public EDP {
public:
	EDPSimple(PDPSimple* p,ParticipantImpl* part);
	virtual ~EDPSimple();

	DiscoveryAttributes m_discovery;

	//!Pointer to the Publications Writer (only created if indicated in the DiscoveryAtributes).
	StatefulWriter* mp_PubWriter;
	//!Pointer to the Subscriptions Writer (only created if indicated in the DiscoveryAtributes).
	StatefulWriter* mp_SubWriter;
	//!Pointer to the Publications Reader (only created if indicated in the DiscoveryAtributes).
	StatefulReader* mp_PubReader;
	//!Pointer to the Subscriptions Reader (only created if indicated in the DiscoveryAtributes).
	StatefulReader* mp_SubReader;

	EDPSimpleListeners m_listeners;
	EDPSimpleTopicDataType m_pubReaderTopicDataType;
	EDPSimpleTopicDataType m_subReaderTopicDataType;


	/**
	 * Initialization method.
	 * @param attributes Reference to the DiscoveryAttributes.
	 * @return True if correct.
	 */
	bool initEDP(DiscoveryAttributes& attributes);

	void assignRemoteEndpoints(ParticipantProxyData* pdata);

	/**
	 * Create local SEDP Endpoints based on the DiscoveryAttributes.
	 * @return True if correct.
	 */
	bool createSEDPEndpoints();

	bool processLocalReaderProxyData(ReaderProxyData* rdata);

	bool processLocalWriterProxyData(WriterProxyData* rdata);

	bool removeLocalReader(RTPSReader*);
	bool removeLocalWriter(RTPSWriter*);

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* EDPSIMPLE_H_ */
