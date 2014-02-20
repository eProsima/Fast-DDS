/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * DataSubMessage.hpp
 *
 *  Created on: Feb 19, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */

namespace eprosima{
namespace rtps{






bool CDRMessageCreator::createMessageData(CDRMessage_t* msg,
										GuidPrefix_t guidprefix,
										SubmsgData_t* DataSubM){
	Header_t H = Header_t();
	memcpy(H.guidPrefix,guidprefix,12);
	try{
		createHeader(msg,&H);
		CDRMessage_t submsg;
		createSubmessageData(&submsg,DataSubM);
		appendMsg(msg, &submsg);
	}
	catch(int e){
		return false;
	}
	return true;
}



bool CDRMessageCreator::createSubmessageData(CDRMessage_t* msg,
		SubmsgData_t* DataSubM) {
	CDRMessage_t* submsg = new CDRMessage_t();
	initCDRMsg(submsg);
	submsg->msg_endian = DataSubM->SubmessageHeader.flags[0] ? BIGEND : LITTLEEND;
	try{
		//extra flags
		addUshort(submsg,0);
		//octet to inline Qos is 12, may change in future versions
		addUshort(submsg,RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG);
		//Entity ids
		addEntityId(submsg,&DataSubM->readerId);
		addEntityId(submsg,&DataSubM->writerId);
		//Add Sequence Number
		addSequenceNumber(submsg,&DataSubM->writerSN);
		//Add Parameter List
		if(DataSubM->SubmessageHeader.flags[1]) //inlineQoS
		{
			std::vector<Parameter_t>::iterator it;
			for(it=DataSubM->inlineQos.begin();it!=DataSubM->inlineQos.end();it++){
				addParameter(submsg,&(*it));
			}
			addUshort(submsg,PID_SENTINEL);
			addUshort(submsg,0);
		}
		//Add Serialized Payload
		if(DataSubM->SubmessageHeader.flags[2] || DataSubM->SubmessageHeader.flags[3]){
			if(submsg->msg_endian ==DEFAULT_ENDIAN)
				addData(submsg,DataSubM->serializedPayload.data,DataSubM->serializedPayload.length);
			else
				addDataReversed(submsg,DataSubM->serializedPayload.data,DataSubM->serializedPayload.length);
		}
	}
	catch(int t){
		return false;
	}
	DataSubM->SubmessageHeader.flags[7] = DataSubM->endiannessFlag;
	DataSubM->SubmessageHeader.flags[6] = DataSubM->inlineQosFlag;
	DataSubM->SubmessageHeader.flags[5] = DataSubM->dataFlag;
	DataSubM->SubmessageHeader.flags[4] = DataSubM->keyFlag;
	for (uint i =0;i<=3;i++)
		DataSubM->SubmessageHeader.flags[i] = false;
	//Once the submessage elements are added, the header is created
	createSubmessageHeader(msg, &DataSubM->SubmessageHeader,submsg->w_pos);
	//Append Submessage elements to msg
	appendMsg(msg, submsg);

	return true;
}

}
}


