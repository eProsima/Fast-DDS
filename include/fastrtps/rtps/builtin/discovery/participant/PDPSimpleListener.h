/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PDPSimpleListener.h
 *
 */

#ifndef PDPSIMPLELISTENER_H_
#define PDPSIMPLELISTENER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "fastrtps/rtps/reader/ReaderListener.h"
#include "fastrtps/qos/ParameterList.h"
#include "fastrtps/rtps/builtin/data/ParticipantProxyData.h"
using namespace eprosima::fastrtps;

namespace eprosima {
namespace fastrtps{
namespace rtps {

class PDPSimple;
class DiscoveredParticipantData;
class RTPSReader;


/**
 * Class PDPSimpleListener, specification of SubscriberListener used by the SPDP to perform the History check when a new message is received.
 * This class is implemented in order to use the same structure than with any other RTPSReader.
 *@ingroup DISCOVERY_MODULE
 */
class PDPSimpleListener: public ReaderListener {
public:
	/**
	* @param in_SPDP
	*/
	PDPSimpleListener(PDPSimple* in_SPDP):mp_SPDP(in_SPDP)
	{
		free(aux_msg.buffer);
		aux_msg.buffer = nullptr;
	};
	virtual ~PDPSimpleListener(){};
	//!Pointer to the associated mp_SPDP;
	PDPSimple* mp_SPDP;
	/**
	 * New added cache
	 * @param reader
	 * @param change
	 */
	void onNewCacheChangeAdded(RTPSReader* reader,const CacheChange_t* const change);
	/**
	 * Process a new added cache with this method.
	 * @return True on success
	 */
	bool newAddedCache();
	/**
	 * Get the key of a CacheChange_t
	 * @param change Pointer to the CacheChange_t
	 * @return True on success
	 */
	bool getKey(CacheChange_t* change);
	//!Temporal RTPSParticipantProxyData object used to read the messages.
	ParticipantProxyData m_ParticipantProxyData;
	//!Auxiliary message.
	CDRMessage_t aux_msg;
};


}
} /* namespace rtps */
} /* namespace eprosima */

#endif
#endif /* PDPSIMPLELISTENER_H_ */
