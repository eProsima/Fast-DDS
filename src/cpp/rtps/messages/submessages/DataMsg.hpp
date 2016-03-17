/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * DataSubMessage.hpp
 *
 */

using namespace eprosima::fastrtps;

namespace eprosima{
namespace fastrtps{
namespace rtps{




bool RTPSMessageCreator::addMessageData(CDRMessage_t* msg, GuidPrefix_t& guidprefix,
        const CacheChange_t* change, TopicKind_t topicKind, const EntityId_t& readerId, bool expectsInlineQos, ParameterList_t* inlineQos)
{

	const char* const METHOD_NAME = "addMessageData";
	try{

		RTPSMessageCreator::addHeader(msg,guidprefix);

		RTPSMessageCreator::addSubmessageInfoTS_Now(msg,false);

		RTPSMessageCreator::addSubmessageData(msg,change,topicKind,readerId,expectsInlineQos,inlineQos);

		msg->length = msg->pos;
	}
	catch(int e)
	{
		logError(RTPS_CDR_MSG, "Data message error" << e << endl)

		return false;
	}
	return true;
}



bool RTPSMessageCreator::addSubmessageData(CDRMessage_t* msg, const CacheChange_t* change,
		TopicKind_t topicKind, const EntityId_t& readerId, bool expectsInlineQos, ParameterList_t* inlineQos) {
	const char* const METHOD_NAME = "addSubmessageData";
	CDRMessage_t& submsgElem = g_pool_submsg.reserve_CDRMsg(change->serializedPayload.length);
	CDRMessage::initCDRMsg(&submsgElem);
	//Create the two CDR msgs
	//CDRMessage_t submsgElem;
	octet flags = 0x0;
#if EPROSIMA_BIG_ENDIAN
    submsgElem.msg_endian = BIGEND;
#else
	flags = flags | BIT(0);
	submsgElem.msg_endian = LITTLEEND;
#endif

	//Find out flags
	bool dataFlag = false;
	bool keyFlag = false;
	bool inlineQosFlag = false;
	if(change->kind == ALIVE && change->serializedPayload.length>0 && change->serializedPayload.data!=NULL)
	{
		dataFlag = true;
		keyFlag = false;
	}
	else
	{
		dataFlag = false;
		keyFlag = true;
	}
	 if(topicKind == NO_KEY)
		 keyFlag = false;
	 inlineQosFlag = false;
	// cout << "expects inline qos: " << expectsInlineQos << " topic KIND: " << (topicKind == WITH_KEY) << endl;
	if(inlineQos != NULL || expectsInlineQos || change->kind != ALIVE) //expects inline qos
	{
		if(topicKind == WITH_KEY)
		{
			flags = flags | BIT(1);
			inlineQosFlag = true;
			//cout << "INLINE QOS FLAG TO 1 " << endl;
			keyFlag = false;
		}
	}
    // Maybe the inline QoS because a WriteParam.
    else if(change->write_params.related_sample_identity() != SampleIdentity::unknown())
    {
        inlineQosFlag = true;
        flags = flags | BIT(1);
    }

	if(dataFlag)
		flags = flags | BIT(2);
	if(keyFlag)
		flags = flags | BIT(3);

	octet status = 0;
	if(change->kind == NOT_ALIVE_DISPOSED)
		status = status | BIT(0);
	if(change->kind == NOT_ALIVE_UNREGISTERED)
		status = status | BIT(1);
	if(change->kind == NOT_ALIVE_DISPOSED_UNREGISTERED)
	{
		status = status | BIT(0);
		status = status | BIT(1);
	}

    // TODO Check, because I saw init the message two times (other on RTPSMessageGroup::prepareDataSubM)
	CDRMessage::initCDRMsg(&submsgElem);
	bool added_no_error = true;
	try{
		//First we create the submsgElements:
		//extra flags. not in this version.
		added_no_error &= CDRMessage::addUInt16(&submsgElem,0);
		//octet to inline Qos is 12, may change in future versions
		added_no_error &= CDRMessage::addUInt16(&submsgElem,RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG);
		//Entity ids
		added_no_error &= CDRMessage::addEntityId(&submsgElem,&readerId);
		added_no_error &= CDRMessage::addEntityId(&submsgElem,&change->writerGUID.entityId);
		//Add Sequence Number
		added_no_error &= CDRMessage::addSequenceNumber(&submsgElem,&change->sequenceNumber);
		//Add INLINE QOS AND SERIALIZED PAYLOAD DEPENDING ON FLAGS:


		if(inlineQosFlag) //inlineQoS
		{
			if(inlineQos != NULL)
			{
				if(inlineQos->m_hasChanged || inlineQos->m_cdrmsg.msg_endian != submsgElem.msg_endian)
				{
					ParameterList::updateCDRMsg(inlineQos,submsgElem.msg_endian);
				}
			}

            if(change->write_params.related_sample_identity() != SampleIdentity::unknown())
            {
                CDRMessage::addParameterSampleIdentity(&submsgElem, change->write_params.related_sample_identity());
            }

			if(topicKind == WITH_KEY)
			{
				//cout << "ADDDING PARAMETER KEY " << endl;
				CDRMessage::addParameterKey(&submsgElem,&change->instanceHandle);
			}

			if(change->kind != ALIVE)
				CDRMessage::addParameterStatus(&submsgElem,status);

			if(inlineQos!=NULL)
				CDRMessage::appendMsg(&submsgElem,&inlineQos->m_cdrmsg);
			else
				CDRMessage::addParameterSentinel(&submsgElem);
		}
		//Add Serialized Payload
		if(dataFlag)
		{
			added_no_error &= CDRMessage::addOctet(&submsgElem,0); //ENCAPSULATION
			added_no_error &= CDRMessage::addOctet(&submsgElem,(octet)change->serializedPayload.encapsulation); //ENCAPSULATION
			added_no_error &= CDRMessage::addUInt16(&submsgElem,0); //OPTIONS
			//cout << "Adding Data of length: "<<change->serializedPayload.length<<endl;
			//cout << "Msg size: "<<submsgElem.max_size << " length: "<< submsgElem.length<< " pos "<< submsgElem.pos<<endl;
			added_no_error &= CDRMessage::addData(&submsgElem,change->serializedPayload.data,change->serializedPayload.length);
		}
		if(keyFlag)
		{
			added_no_error &= CDRMessage::addOctet(&submsgElem,0); //ENCAPSULATION
			if(submsgElem.msg_endian == BIGEND)
				added_no_error &= CDRMessage::addOctet(&submsgElem,PL_CDR_BE); //ENCAPSULATION
			else
				added_no_error &= CDRMessage::addOctet(&submsgElem,PL_CDR_LE); //ENCAPSULATION

			added_no_error &= CDRMessage::addUInt16(&submsgElem,0); //ENCAPSULATION OPTIONS
			added_no_error &= CDRMessage::addParameterKey(&submsgElem,&change->instanceHandle);
			added_no_error &= CDRMessage::addParameterStatus(&submsgElem,status);
			added_no_error &= CDRMessage::addParameterSentinel(&submsgElem);
		}

        // Align submessage to rtps alignment (4).
        uint32_t align = (4 - submsgElem.pos % 4) & 3;
        for(uint32_t count = 0; count < align; ++count)
            added_no_error &= CDRMessage::addOctet(&submsgElem, 0);

        //if(align > 0)
        {
            //submsgElem.pos += align;
            //submsgElem.length += align;
        }

		//Once the submessage elements are added, the submessage header is created, assigning the correct size.
		added_no_error &= RTPSMessageCreator::addSubmessageHeader(msg, DATA,flags, (uint16_t)submsgElem.length);
		//Append Submessage elements to msg

		added_no_error &= CDRMessage::appendMsg(msg, &submsgElem);
		g_pool_submsg.release_CDRMsg(submsgElem);
	}
	catch(int t){
		logError(RTPS_CDR_MSG,"Data SUBmessage not created"<<t<<endl)

		return false;
	}
	return added_no_error;
}


bool RTPSMessageCreator::addMessageDataFrag(CDRMessage_t* msg, GuidPrefix_t& guidprefix,
        const CacheChange_t* change, uint32_t fragment_number, TopicKind_t topicKind, const EntityId_t& readerId,
        bool expectsInlineQos, ParameterList_t* inlineQos)
{

	const char* const METHOD_NAME = "addMessageDataFrag";
	try{

		RTPSMessageCreator::addHeader(msg, guidprefix);

		RTPSMessageCreator::addSubmessageInfoTS_Now(msg, false);

		RTPSMessageCreator::addSubmessageDataFrag(msg, change, fragment_number, topicKind, readerId, expectsInlineQos, inlineQos);

		msg->length = msg->pos;
	}
	catch (int e)
	{
		logError(RTPS_CDR_MSG, "Data message error" << e << endl)
		return false;
	}
	return true;
}



bool RTPSMessageCreator::addSubmessageDataFrag(CDRMessage_t* msg, const CacheChange_t* change, uint32_t fragment_number,
	TopicKind_t topicKind, const EntityId_t& readerId, bool expectsInlineQos, ParameterList_t* inlineQos) {

	const char* const METHOD_NAME = "addSubmessageDataFrag";

	// Calculate fragment start
	uint32_t fragment_start = change->getFragmentSize() * (fragment_number - 1);
	uint32_t fragment_size = change->getFragmentSize();
	if(fragment_number >= change->getFragmentCount()) // If last fragment, size may be smaller
		fragment_size = change->serializedPayload.length - fragment_start;

	CDRMessage_t& submsgElem = g_pool_submsg.reserve_CDRMsg(fragment_size);
	CDRMessage::initCDRMsg(&submsgElem);
	//Create the two CDR msgs
	//CDRMessage_t submsgElem;
	octet flags = 0x0;
#if EPROSIMA_BIG_ENDIAN
	submsgElem.msg_endian = BIGEND;
#else
	flags = flags | BIT(0);
	submsgElem.msg_endian = LITTLEEND;
#endif

	//Find out flags
	bool keyFlag = false;
	bool inlineQosFlag = false;
	if (change->kind == ALIVE && change->serializedPayload.length>0 && change->serializedPayload.data != NULL)
	{
		keyFlag = false;
	}
	else
	{
		keyFlag = true;
	}

	if (topicKind == NO_KEY)
		keyFlag = false;

	// cout << "expects inline qos: " << expectsInlineQos << " topic KIND: " << (topicKind == WITH_KEY) << endl;
	if (inlineQos != NULL || expectsInlineQos || change->kind != ALIVE) //expects inline qos
	{
		if (topicKind == WITH_KEY)
		{
			flags = flags | BIT(1);
			inlineQosFlag = true;
			//cout << "INLINE QOS FLAG TO 1 " << endl;
			keyFlag = false;
		}
	}
	// Maybe the inline QoS because a WriteParam.
	else if (change->write_params.related_sample_identity() != SampleIdentity::unknown())
	{
		inlineQosFlag = true;
		flags = flags | BIT(1);
	}

	if (keyFlag)
		flags = flags | BIT(2);

	octet status = 0;
	if (change->kind == NOT_ALIVE_DISPOSED)
		status = status | BIT(0);
	if (change->kind == NOT_ALIVE_UNREGISTERED)
		status = status | BIT(1);

	if (change->kind == NOT_ALIVE_DISPOSED_UNREGISTERED)
	{
		status = status | BIT(0);
		status = status | BIT(1);
	}

	// TODO Check, because I saw init the message two times (other on RTPSMessageGroup::prepareDataSubM)
	CDRMessage::initCDRMsg(&submsgElem);
	bool added_no_error = true;

	try
    {
		//First we create the submsgElements:
		//extra flags. not in this version.
		added_no_error &= CDRMessage::addUInt16(&submsgElem, 0);

		//octet to inline Qos is 28, may change in future versions
		added_no_error &= CDRMessage::addUInt16(&submsgElem, RTPSMESSAGE_OCTETSTOINLINEQOS_DATAFRAGSUBMSG);

		//Entity ids
		added_no_error &= CDRMessage::addEntityId(&submsgElem, &readerId);
		added_no_error &= CDRMessage::addEntityId(&submsgElem, &change->writerGUID.entityId);

		//Add Sequence Number
		added_no_error &= CDRMessage::addSequenceNumber(&submsgElem, &change->sequenceNumber);

		// Add fragment starting number
		added_no_error &= CDRMessage::addUInt32(&submsgElem, fragment_number); // fragments start in 1

		// Add fragments in submessage
		added_no_error &= CDRMessage::addUInt16(&submsgElem, 1); // we are sending one fragment

		// Add fragment size
		added_no_error &= CDRMessage::addUInt16(&submsgElem, change->getFragmentSize());

		// Add total sample size
		added_no_error &= CDRMessage::addUInt32(&submsgElem, change->serializedPayload.length);
		
		//Add INLINE QOS AND SERIALIZED PAYLOAD DEPENDING ON FLAGS:
		if (inlineQosFlag) //inlineQoS
		{
			if (inlineQos != NULL)
				if (inlineQos->m_hasChanged || inlineQos->m_cdrmsg.msg_endian != submsgElem.msg_endian)
					ParameterList::updateCDRMsg(inlineQos, submsgElem.msg_endian);

			if(change->write_params.related_sample_identity() != SampleIdentity::unknown())
				CDRMessage::addParameterSampleIdentity(&submsgElem, change->write_params.related_sample_identity());

			if(topicKind == WITH_KEY)
				CDRMessage::addParameterKey(&submsgElem,&change->instanceHandle);

			if(change->kind != ALIVE)
				CDRMessage::addParameterStatus(&submsgElem,status);

			if(inlineQos!=NULL)
				CDRMessage::appendMsg(&submsgElem,&inlineQos->m_cdrmsg);
			else
				CDRMessage::addParameterSentinel(&submsgElem);
		}

		//Add Serialized Payload XXX TODO
		if (!keyFlag) // keyflag = 0 means that the serializedPayload SubmessageElement contains the serialized Data 
		{
            // Encapsulation is added only in the first fragment.
            if(fragment_number == 1)
            {
                added_no_error &= CDRMessage::addOctet(&submsgElem, 0); //ENCAPSULATION
                added_no_error &= CDRMessage::addOctet(&submsgElem, (octet)change->serializedPayload.encapsulation); //ENCAPSULATION
                added_no_error &= CDRMessage::addUInt16(&submsgElem, 0); //OPTIONS
            }
			added_no_error &= CDRMessage::addData(&submsgElem,
				change->serializedPayload.data + fragment_start,
				fragment_size);
		}
		else
		{	// keyflag = 1 means that the serializedPayload SubmessageElement contains the serialized Key 
			/*
			added_no_error &= CDRMessage::addOctet(&submsgElem, 0); //ENCAPSULATION
			if (submsgElem.msg_endian == BIGEND)
				added_no_error &= CDRMessage::addOctet(&submsgElem, PL_CDR_BE); //ENCAPSULATION
			else
				added_no_error &= CDRMessage::addOctet(&submsgElem, PL_CDR_LE); //ENCAPSULATION

			added_no_error &= CDRMessage::addUInt16(&submsgElem, 0); //ENCAPSULATION OPTIONS
			added_no_error &= CDRMessage::addParameterKey(&submsgElem, &change->instanceHandle);
			added_no_error &= CDRMessage::addParameterStatus(&submsgElem, status);
			added_no_error &= CDRMessage::addParameterSentinel(&submsgElem);
			*/
			return false;
		}
		
		// Align submessage to rtps alignment (4).
		uint32_t align = (4 - submsgElem.pos % 4) & 3;
		for (uint32_t count = 0; count < align; ++count)
			added_no_error &= CDRMessage::addOctet(&submsgElem, 0);

		//Once the submessage elements are added, the submessage header is created, assigning the correct size.
		added_no_error &= RTPSMessageCreator::addSubmessageHeader(msg, DATA_FRAG, flags, (uint16_t)submsgElem.length);

		//Append Submessage elements to msg
		added_no_error &= CDRMessage::appendMsg(msg, &submsgElem);
		g_pool_submsg.release_CDRMsg(submsgElem);

	}
	catch (int t){
		logError(RTPS_CDR_MSG, "Data SUBmessage not created" << t << endl)
		return false;
	}

	return added_no_error;
}


}
}
}











