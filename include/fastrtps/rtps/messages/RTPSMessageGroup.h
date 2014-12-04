/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSMessageGroup.h
 *
 */

#ifndef RTPSMESSAGEGROUP_H_
#define RTPSMESSAGEGROUP_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <vector>
#include "fastrtps/rtps/common/CDRMessage_t.h"
#include "fastrtps/qos/ParameterList.h"

namespace eprosima {
namespace fastrtps{
namespace rtps {

/**
 * Class RTPSMessageGroup_t that contains the messages used to send multiples changes as one message.
 * @ingroup WRITERMODULE
 */
class RTPSMessageGroup_t{
public:
	CDRMessage_t m_rtpsmsg_header;
	CDRMessage_t m_rtpsmsg_submessage;
	CDRMessage_t m_rtpsmsg_fullmsg;
	RTPSMessageGroup_t():
		m_rtpsmsg_header(RTPSMESSAGE_HEADER_SIZE),
				m_rtpsmsg_submessage(RTPSMESSAGE_DEFAULT_SIZE),
				m_rtpsmsg_fullmsg(RTPSMESSAGE_DEFAULT_SIZE){};
		RTPSMessageGroup_t(uint32_t payload):
		m_rtpsmsg_header(RTPSMESSAGE_HEADER_SIZE),
				m_rtpsmsg_submessage(payload+RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE),
				m_rtpsmsg_fullmsg(payload+RTPSMESSAGE_COMMON_RTPS_PAYLOAD_SIZE){};
};

class RTPSWriter;

/**
 * RTPSMessageGroup class used to send multiple changes as a single CDRMessage.
 * @ingroup WRITERMODULE
 */
class RTPSMessageGroup {
public:

/**
 * @param msg_group
 * @param W
 * @param changesSeqNum
 * @param readerId
 * @param unicast
 * @param multicast
 * @return 
 */
static bool send_Changes_AsGap(RTPSMessageGroup_t* msg_group,
						RTPSWriter* W,
						std::vector<SequenceNumber_t>* changesSeqNum,
						const EntityId_t& readerId,
						LocatorList_t* unicast,
						LocatorList_t* multicast);


/**
 * @param changesSeqNum
 * @param Sequences
 */
static void prepare_SequenceNumberSet(std::vector<SequenceNumber_t>* changesSeqNum,
		std::vector<std::pair<SequenceNumber_t,SequenceNumberSet_t>>* Sequences);

/**
 * @param msg_group
 * @param W
 * @param changes
 * @param unicast
 * @param multicast
 * @param expectsInlineQos
 * @param ReaderId
 * @return 
 */
static bool send_Changes_AsData(RTPSMessageGroup_t* msg_group,
		RTPSWriter* W,
		std::vector<CacheChange_t*>* changes,
		LocatorList_t& unicast,
		LocatorList_t& multicast,
		bool expectsInlineQos,
		const EntityId_t& ReaderId);

/**
 * @param msg_group
 * @param W
 * @param changes
 * @param loc
 * @param expectsInlineQos
 * @param ReaderId
 * @return 
 */
static bool send_Changes_AsData(RTPSMessageGroup_t* msg_group,
		RTPSWriter* W,
		std::vector<CacheChange_t*>* changes,
		const Locator_t& loc,
		bool expectsInlineQos,
		const EntityId_t& ReaderId);

/**
 * @param W
 * @param submsg
 * @param expectsInlineQos
 * @param change
 * @param ReaderId
 * @return 
 */
static void prepareDataSubM(RTPSWriter* W,CDRMessage_t* submsg,bool expectsInlineQos,CacheChange_t* change,const EntityId_t& ReaderId);


};
}
} /* namespace rtps */
} /* namespace eprosima */

#endif
#endif /* RTPSMESSAGEGROUP_H_ */
