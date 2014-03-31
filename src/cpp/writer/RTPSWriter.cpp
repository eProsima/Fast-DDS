/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * RTPSWriter.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *      		grcanosa@gmail.com
 */

#include "eprosimartps/writer/RTPSWriter.h"
#include "eprosimartps/HistoryCache.h"
#include "eprosimartps/CDRMessage.h"
#include "eprosimartps/dds/Publisher.h"

#include "eprosimartps/dds/ParameterList.h"

namespace eprosima {
namespace rtps {


RTPSWriter::RTPSWriter(uint16_t historysize,uint32_t payload_size):
		m_stateType(STATELESS),
		m_writer_cache(historysize,payload_size),
		m_pushMode(true),
		m_heartbeatCount(0),
		m_Pub(NULL)
{
	m_writer_cache.historyKind = WRITER;
	m_writer_cache.rtpswriter = this;
	pDebugInfo("RTPSWriter created"<<endl)
}

void RTPSWriter::init_header()
{
	CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_header);
	RTPSMessageCreator::addHeader(&m_cdrmessages.m_rtpsmsg_header,participant->m_guid.guidPrefix);
}



RTPSWriter::~RTPSWriter()
{
	pDebugInfo("RTPSWriter destructor"<<endl;);
}

bool RTPSWriter::new_change(ChangeKind_t changeKind,void* data,CacheChange_t** change_out)
{
	CacheChange_t* ch = m_writer_cache.reserve_Cache();
	if(changeKind == ALIVE)
	{
		m_type.serialize(&ch->serializedPayload,data);
	}

	ch->kind = changeKind;
	if(topicKind == WITH_KEY)
		m_type.getKey(data,&ch->instanceHandle);

	//change->sequenceNumber = lastChangeSequenceNumber;
	ch->writerGUID = guid;

	*change_out = ch;
	return true;
}






} /* namespace rtps */
} /* namespace eprosima */


