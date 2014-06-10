/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSMessageGroup.h
 *
 */

#ifndef RTPSMESSAGEGROUP_H_
#define RTPSMESSAGEGROUP_H_
#include <vector>
#include "eprosimartps/common/types/CDRMessage_t.h"
#include "eprosimartps/qos/ParameterList.h"

namespace eprosima {
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
				m_rtpsmsg_submessage(RTPSMESSAGE_MAX_SIZE),
				m_rtpsmsg_fullmsg(RTPSMESSAGE_MAX_SIZE){};
		RTPSMessageGroup_t(uint32_t payload):
		m_rtpsmsg_header(RTPSMESSAGE_HEADER_SIZE),
				m_rtpsmsg_submessage(payload+200),
				m_rtpsmsg_fullmsg(payload+200){};
};

class RTPSWriter;

/**
 * RTPSMessageGroup class used to send multiple changes as a single CDRMessage.
 * @ingroup WRITERMODULE
 */
class RTPSMessageGroup {
public:
static void send_Changes_AsGap(RTPSMessageGroup_t* msg_group,
						RTPSWriter* W,
						std::vector<CacheChange_t*>* changes,
						const EntityId_t& readerId,
						LocatorList_t* unicast,
						LocatorList_t* multicast);



static void prepare_SequenceNumberSet(std::vector<CacheChange_t*>* changes,
		std::vector<std::pair<SequenceNumber_t,SequenceNumberSet_t>>* Sequences);

static void send_Changes_AsData(RTPSMessageGroup_t* msg_group,
		RTPSWriter* W,
		std::vector<CacheChange_t*>* changes,
		 LocatorList_t& unicast, LocatorList_t& multicast,
		bool expectsInlineQos,const EntityId_t& ReaderId);

static void send_Changes_AsData(RTPSMessageGroup_t* msg_group,
		RTPSWriter* W,
		std::vector<CacheChange_t*>* changes,const Locator_t& loc,
		bool expectsInlineQos,const EntityId_t& ReaderId);

static void prepareDataSubM(RTPSWriter* W,CDRMessage_t* submsg,bool expectsInlineQos,CacheChange_t* change,const EntityId_t& ReaderId);


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSMESSAGEGROUP_H_ */
