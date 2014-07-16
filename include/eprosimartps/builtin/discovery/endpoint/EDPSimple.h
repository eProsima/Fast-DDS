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

namespace eprosima {
namespace rtps {

class StatefulReader;
class StatefulWriter;
class RTPSWriter;
class RTPSReader;

class EDPSimple : public EDP {
public:
	EDPSimple(PDPSimple* p);
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


	/**
	 * Initialization method.
	 * @param attributes Reference to the DiscoveryAttributes.
	 * @return True if correct.
	 */
	bool initEDP(DiscoveryAttributes& attributes);
	/**
	 * Create local SEDP Endpoints based on the DiscoveryAttributes.
	 * @return True if correct.
	 */
	bool createSEDPEndpoints();

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* EDPSIMPLE_H_ */
