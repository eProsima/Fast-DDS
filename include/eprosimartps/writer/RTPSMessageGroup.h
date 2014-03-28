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
 *  Created on: Mar 28, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef RTPSMESSAGEGROUP_H_
#define RTPSMESSAGEGROUP_H_

#include "eprosimartps/rtps_all.h"
#include "eprosimartps/dds/ParameterList.h"

namespace eprosima {
namespace rtps {

class RTPSMessageGroup_t{
public:
	CDRMessage_t m_rtpsmsg_header;
	CDRMessage_t m_rtpsmsg_submessage;
	CDRMessage_t m_rtpsmsg_fullmsg;
	RTPSMessageGroup_t():
		m_rtpsmsg_header(RTPSMESSAGE_HEADER_SIZE),
				m_rtpsmsg_submessage(RTPSMESSAGE_HEADER_SIZE),
				m_rtpsmsg_fullmsg(RTPSMESSAGE_HEADER_SIZE){};
};

class RTPSWriter;

class RTPSMessageGroup {
public:
static void send_Changes_AsGap(RTPSMessageGroup_t* msg_group,
						RTPSWriter* W,
						std::vector<CacheChange_t*>* changes,
						const EntityId_t& readerId,
						std::vector<Locator_t>* unicast,
						std::vector<Locator_t>* multicast);



static void prepare_SequenceNumberSet(std::vector<CacheChange_t*>* changes,
		std::vector<std::pair<SequenceNumber_t,SequenceNumberSet_t>>* Sequences);

static void send_Changes_AsData(RTPSMessageGroup_t* msg_group,
		RTPSWriter* W,
		std::vector<CacheChange_t*>* changes,
		std::vector<Locator_t>* unicast,std::vector<Locator_t>* multicast,
		bool expectsInlineQos,const EntityId_t& ReaderId);

static void send_Changes_AsData(RTPSMessageGroup_t* msg_group,
		RTPSWriter* W,
		std::vector<CacheChange_t*>* changes,Locator_t* loc,
		bool expectsInlineQos,const EntityId_t& ReaderId);

static void prepareDataSubM(RTPSWriter* W,CDRMessage_t* submsg,bool expectsInlineQos,CacheChange_t* change,const EntityId_t& ReaderId);


};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSMESSAGEGROUP_H_ */
