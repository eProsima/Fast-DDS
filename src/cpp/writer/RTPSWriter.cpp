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
		mp_type(NULL),
		m_Pub(NULL)

{
	m_writer_cache.m_historyKind = WRITER;
	m_writer_cache.mp_rtpswriter = this;
	pDebugInfo("RTPSWriter created"<<endl)
}

void RTPSWriter::init_header()
{
	CDRMessage::initCDRMsg(&m_cdrmessages.m_rtpsmsg_header);
	RTPSMessageCreator::addHeader(&m_cdrmessages.m_rtpsmsg_header,m_guid.guidPrefix);
}



RTPSWriter::~RTPSWriter()
{

	pDebugInfo("RTPSWriter destructor"<<endl;);
}

bool RTPSWriter::new_change(ChangeKind_t changeKind,void* data,CacheChange_t** change_out)
{
	pDebugInfo("Creating new change"<<endl);
	CacheChange_t* ch = m_writer_cache.reserve_Cache();
	if(ch == NULL)
	{
		pWarning("Problem reserving Cache"<<endl);
		return false;
	}
	if(changeKind == ALIVE && data !=NULL && mp_type !=NULL)
	{
		if(!mp_type->serialize(data,&ch->serializedPayload))
		{
			pWarning("RTPSWriter:Serialization returns false"<<endl);
			return false;
		}
	}

	ch->kind = changeKind;

	if(topicKind == WITH_KEY && mp_type !=NULL)
	{
		if(mp_type->m_isGetKeyDefined)
		{
			mp_type->getKey(data,&ch->instanceHandle);
		}
		else
		{
			pWarning("Get key function not defined"<<endl);
		}
	}


	//change->sequenceNumber = lastChangeSequenceNumber;
	ch->writerGUID = m_guid;

	*change_out = ch;
	return true;
}






} /* namespace rtps */
} /* namespace eprosima */


