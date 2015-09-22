/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * GapMsg.hpp
 *
 */



namespace eprosima{
namespace fastrtps{
namespace rtps{

bool RTPSMessageCreator::addMessageGap(CDRMessage_t* msg, const GuidPrefix_t& guidprefix,
        const GuidPrefix_t& remoteGuidPrefix,
		SequenceNumber_t& seqNumFirst,SequenceNumberSet_t& seqNumList,
		const EntityId_t& readerId,const EntityId_t& writerId)
{
	const char* const METHOD_NAME = "addSubmessageData";
	try
	{
		RTPSMessageCreator::addHeader(msg, guidprefix);
        RTPSMessageCreator::addSubmessageInfoDST(msg, remoteGuidPrefix);
		RTPSMessageCreator::addSubmessageInfoTS_Now(msg,false);
		RTPSMessageCreator::addSubmessageGap(msg,seqNumFirst,seqNumList,readerId, writerId);
	}
	catch(int e)
	{
		logError(RTPS_CDR_MSG,"Gap message error"<<e<<endl)
		return false;
	}
	return true;
}

bool RTPSMessageCreator::addSubmessageGap(CDRMessage_t* msg,SequenceNumber_t& seqNumFirst,SequenceNumberSet_t& seqNumList,const EntityId_t& readerId,const EntityId_t& writerId)
{
	const char* const METHOD_NAME = "addSubmessageData";
	CDRMessage_t& submsgElem = g_pool_submsg.reserve_CDRMsg();
		CDRMessage::initCDRMsg(&submsgElem);
	octet flags = 0x0;
#if EPROSIMA_BIG_ENDIAN
    submsgElem.msg_endian = BIGEND;
#else
	flags = flags | BIT(0);
	submsgElem.msg_endian   = LITTLEEND;
#endif

	try{
		CDRMessage::addEntityId(&submsgElem,&readerId);
		CDRMessage::addEntityId(&submsgElem,&writerId);
		//Add Sequence Number
		CDRMessage::addSequenceNumber(&submsgElem,&seqNumFirst);
		CDRMessage::addSequenceNumberSet(&submsgElem,&seqNumList);
	}
	catch(int e)
	{
		logError(RTPS_CDR_MSG,"Gap submessage error"<<e<<endl)
		return false;
	}


	//Once the submessage elements are added, the header is created
	RTPSMessageCreator::addSubmessageHeader(msg, GAP, flags, (uint16_t)submsgElem.length);
	//Append Submessage elements to msg
	CDRMessage::appendMsg(msg, &submsgElem);
	g_pool_submsg.release_CDRMsg(submsgElem);

	return true;
}

}
}
}
