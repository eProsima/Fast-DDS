/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file PDPSimpleListener.h
 *
 */

#ifndef PDPSIMPLELISTENER_H_
#define PDPSIMPLELISTENER_H_
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
 * @ingroup DISCOVERYMODULE
 */
class PDPSimpleListener: public ReaderListener {
public:
	PDPSimpleListener(PDPSimple* in_SPDP):mp_SPDP(in_SPDP){};
	virtual ~PDPSimpleListener(){};
	//!Pointer to the associated mp_SPDP;
	PDPSimple* mp_SPDP;
	//!new added cache
	void onNewCacheChangeAdded(RTPSReader* reader,CacheChange_t* change);
	//!Process a new added cache with this method.
	bool newAddedCache();
	//FIXME: TO IMPLEMENT WHEN A StaticEDP is created.
	void assignUserId(std::string& type,uint16_t userId,EntityId_t& entityId,DiscoveredParticipantData* pdata);

	//!Temporal RTPSParticipantProxyData object used to read the messages.
	ParticipantProxyData m_ParticipantProxyData;
};


}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* PDPSIMPLELISTENER_H_ */
