/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file WLPListener.cpp
 *
 */

#include "fastrtps/rtps/builtin/liveliness/WLPListener.h"
#include "fastrtps/rtps/builtin/liveliness/WLP.h"

#include "fastrtps/rtps/history/ReaderHistory.h"

#include "fastrtps/rtps/builtin/discovery/participant/PDPSimple.h"
#include "fastrtps/rtps/builtin/BuiltinProtocols.h"
//#include "fastrtps/common/types/Guid.h"

//
//#include "fastrtps/RTPSParticipant.h"
//#include "fastrtps/reader/WriterProxyData.h"
//
#include "fastrtps/rtps/reader/StatefulReader.h"

#include "fastrtps/utils/RTPSLog.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/lock_guard.hpp>



namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "WLPListener";

WLPListener::WLPListener(WLP* plwp):
										mp_WLP(plwp)
{
	free(aux_msg.buffer);
}

WLPListener::~WLPListener()
{

}


typedef std::vector<WriterProxy*>::iterator WPIT;

void WLPListener::onNewCacheChangeAdded(RTPSReader* reader,CacheChange_t* change)
{
	const char* const METHOD_NAME = "onNewCacheChangeAdded";
	boost::lock_guard<boost::recursive_mutex> guard(*reader->getMutex());
	boost::lock_guard<boost::recursive_mutex> guard2(*mp_WLP->getMutex());
	logInfo(RTPS_LIVELINESS,"",C_MAGENTA);
	GuidPrefix_t guidP;
	LivelinessQosPolicyKind livelinessKind;
	if(!computeKey(change))
	{
		logWarning(RTPS_LIVELINESS,"Problem obtaining the Key",C_MAGENTA);
		return;
	}
	//Check the serializedPayload:
	for(auto ch = this->mp_WLP->mp_builtinReaderHistory->changesBegin();
			ch!=mp_WLP->mp_builtinReaderHistory->changesEnd();++ch)
	{
		if((*ch)->instanceHandle == change->instanceHandle)
		{
			mp_WLP->mp_builtinReaderHistory->remove_change(*ch);
			break;
		}
	}
	if(change->serializedPayload.length>0)
	{
		for(uint8_t i =0;i<12;++i)
		{
			guidP.value[i] = change->serializedPayload.data[i];
		}
		livelinessKind = (LivelinessQosPolicyKind)(change->serializedPayload.data[15]-0x01);
		logInfo(RTPS_LIVELINESS,"RTPSParticipant "<<guidP<< " assert liveliness of "
				<<((livelinessKind == 0x00)?"AUTOMATIC":"")
				<<((livelinessKind==0x01)?"MANUAL_BY_RTPSParticipant":"")<< " writers",C_MAGENTA);
	}
	else
	{
		if(!separateKey(change->instanceHandle,&guidP,&livelinessKind))
			return;
	}
	if(guidP == reader->getGuid().guidPrefix)
	{
		logInfo(RTPS_LIVELINESS,"Message from own RTPSParticipant, ignoring",C_MAGENTA);
		this->mp_WLP->mp_builtinReaderHistory->remove_change(change);
		return;
	}
	this->mp_WLP->getBuiltinProtocols()->mp_PDP->assertRemoteWritersLiveliness(guidP,livelinessKind);

	return;
}


//bool WLPListener::processParameterList(ParameterList_t* param,GuidPrefix_t* guidP,LivelinessQosPolicyKind* liveliness)
//{
//	const char* const METHOD_NAME = "processParameterList";
//	for(std::vector<Parameter_t*>::iterator it=param->m_parameters.begin();
//			it!=param->m_parameters.end();++it)
//	{
//		switch((*it)->Pid)
//		{
//		case(PID_KEY_HASH):
//						{
//			ParameterKey_t* p = (ParameterKey_t*)(*it);
//			return separateKey(p->key,guidP,liveliness);
//						}
//		default:
//		{
//			logWarning(RTPS_LIVELINESS,"In this ParameterList should not be anything but the Key",C_MAGENTA);
//			break;
//		}
//		}
//	}
//	return false;
//}

bool WLPListener::separateKey(InstanceHandle_t& key,GuidPrefix_t* guidP,LivelinessQosPolicyKind* liveliness)
{
	for(uint8_t i=0;i<12;++i)
	{
		guidP->value[i] = key.value[i];
	}
	*liveliness = (LivelinessQosPolicyKind)key.value[15];
	return true;
}

bool WLPListener::computeKey(CacheChange_t* change)
{
	if(change->instanceHandle == c_InstanceHandle_Unknown)
	{
		SerializedPayload_t* pl = &change->serializedPayload;
		if(pl->length > 16)
		{
			CDRMessage::initCDRMsg(&aux_msg);
			aux_msg.buffer = pl->data;
			aux_msg.length = pl->length;
			aux_msg.max_size = pl->max_size;
			aux_msg.msg_endian = pl->encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;
			for(uint8_t i =0;i<16;++i)
			{
				change->instanceHandle.value[i] = aux_msg.buffer[i];
			}
			aux_msg.buffer = nullptr;
			return true;
		}
		return false;
	}
	return true;
}


} /* namespace rtps */
} /* namespace eprosima */
}
