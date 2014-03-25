/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * aux_test.cpp
 *
 *  Created on: Feb 24, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */





inline bool readEntityId(EntityId_t* id,octet* data,bool endianness){
	uint32_t* aux1 = (uint32_t*) id->value;
	uint32_t* aux2 = (uint32_t*) data;
	*aux1 = *aux2;
	return true;
}

inline bool readDataReversed(octet* o,octet*data,uint length){
	for(uint i=0;i<length;i++)
		*(o+i)=*(data+length-1-i);
	return true;
}

inline bool readData(octet* o,octet*data,uint length){
	for(uint i=0;i<length;i++)
		*(o+i)=*(data+i);
	return true;
}


inline bool readLong(int32_t* lo,octet* data,bool endianness){
	octet* dest = (octet*)lo;

	if(endianness == DEFAULT_ENDIAN){
		memcpy(dest,data,4);
		cout << "copied as it is:" << endl;
	}
	else{
		cout << "copied reversed" << endl;
		readDataReversed(dest,data,4);
	}

	return true;
}


inline bool readULong(uint32_t* ulo,octet* data,bool endianness){
	if(endianness == DEFAULT_ENDIAN){

		*ulo = *(uint32_t*)data;

	}
	else{

		readDataReversed((octet*)ulo,data,4);

	}
	return true;
}


inline bool readSequenceNumber(SequenceNumber_t* sn,octet* data,bool endianness){
	cout << "def: " << DEFAULT_ENDIAN << " msgend: " << endianness << endl;
	readLong(&sn->high,data,endianness);
	readULong(&sn->low,data+4,endianness);
	cout << sn->high << " " << sn->low << endl;
	return true;
}

inline bool readShort(short* sh,octet*data, bool endianness){
	if(endianness == DEFAULT_ENDIAN)
		*sh = *(short*)data;
	else
		readDataReversed((octet*)sh,data,2);

	return true;
}


inline bool readParameterList(std::vector<Parameter_t>* list,octet*data, bool endianness,unsigned long* list_length){
	short paramId,paramlength;

	unsigned long length = 0;
	while(1){
		Parameter_t p;
		//cout << "Reading first parameter at pos: " << length << endl;
		readShort(&paramId,data+length,endianness);
		//cout << "Param Type: " << paramId;
		length +=2;
		if(paramId == PID_SENTINEL)
			break;
		//cout << " Saved ID: " << p.parameterId;
		readShort(&paramlength,data+length,endianness);
		//cout << " - Param Length: " << paramlength;
		length +=2;
		if(p.reset(paramlength))
			memcpy(p.value,data+length,paramlength);
		else
			return false;
		p.length = paramlength;
		p.parameterId = paramId;
		//cout << " MemCpied " << endl;;
		list->push_back(p);
		length += paramlength;
	}
	length+=2;
	*list_length = length;
	//cout << " --- Parameters retrieved: " << list->size() << endl;
	return true;
}

inline bool compareParameterLists(ParameterList_t* l1,ParameterList_t* l2){
	ParameterList_t::iterator it1,it2;
	bool result = false;
	for(it1=l1->begin();it1!=l1->end();it1++)
	{
		//	cout << "Looking for param: " << it1->parameterId << endl;
		result = false; //We suppose there is no match
		for(it2=l2->begin();it2!=l2->end();it2++) //Look in the other list, same order not guaranteed
		{
			if((it1->parameterId == it2->parameterId) && (it1->length == it2->length)) //Same Parameter
			{
				result = true; //We assume the data is correct.
				//cout << "Found parameter with same length: " << it1->length << endl;
				if(strcmp((const char*)it1->value,(const char*)it2->value)!=0){
					result = false;//Some data was not correct
					break;//dont check the rest of param.
				}
			}
		}
		result_print("Param: ",result);
		if(result == false) //No match for one parameter, the rest is irrelevant
			break;
	}
	return result;
}



bool MessageDataTest(){
	cout << "Init" << endl;
	CDRMessageCreator MC;
	MessageReceiver MR;
	CDRMessage_t msg;
	GuidPrefix_t guidprefix;
	for(uint8_t i = 0;i<12;i++)
		guidprefix.value[i] = (octet)i;
	//octet guidprefix[12] = {0x0,0x1,0x2,0x4,0x8,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
	//GUIDPREFIX_UNKNOWN(guidprefix);
	SubmsgData_t DataSubM;
	DataSubM.SubmessageHeader.submessageId = DATA;
	DataSubM.endiannessFlag = false;
	DataSubM.inlineQosFlag = true;
	DataSubM.dataFlag = true;
	DataSubM.keyFlag = false;
	octet flags = 0x0;
	if(DataSubM.endiannessFlag) flags = flags | BIT(0);
	if(DataSubM.inlineQosFlag) flags = flags | BIT(1);
	if(DataSubM.dataFlag) flags = flags | BIT(2);
	if(DataSubM.keyFlag) flags = flags | BIT(3);
	DataSubM.readerId = EntityId_t(ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER);
	DataSubM.writerId = EntityId_t(ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER);
	DataSubM.writerSN.low = 205210;
	DataSubM.writerSN.high = 2568;
	//DATA
	uint16_t numbers[] = {0,1,2,3,4,5,6,7,8,9};
	octet* oc_numbers = (octet*)numbers;
	DataSubM.serializedPayload.data = (octet*)malloc(sizeof(uint16_t)*10);
	DataSubM.serializedPayload.length = sizeof(uint16_t)*10;
	memcpy(DataSubM.serializedPayload.data,oc_numbers,sizeof(uint16_t)*10);
	//PARAMETERS
	Parameter_t p1,p2;
	p1.create(PID_TOPIC_NAME,std::string("Hola, este es un mensaje largo"));
	DataSubM.inlineQos.push_back(p1);
	p2.create(PID_TYPE_NAME,std::string("Tipo de top"));
	DataSubM.inlineQos.push_back(p2);
	//CREATE MESSAGE
	cout << "Creating message" << endl;
	if(MC.createMessageData(&msg,guidprefix,&DataSubM)){
		cout << "Mensaje creado, testing...." << endl;
		//CHECK THE MESSAGE
		bool result,hres;
		short ioc = 0;
		cout << B_WHITE << "Checking Header " << DEF << endl;
		hres = result = msg.buffer[ioc] == 'R';
		hres &= result &= msg.buffer[++ioc] == 'T';
		hres &= result &= msg.buffer[++ioc] == 'P';
		hres &= result &= msg.buffer[++ioc] == 'S';
		result_print("Protocol String: ",result);
		hres &= result = msg.buffer[++ioc] == 2;
		hres &= result &= msg.buffer[++ioc] == 1;
		result_print("Protocol Version: ",result);
		hres &= result = msg.buffer[++ioc] == 0;
		hres &= result &= msg.buffer[++ioc] == 0;
		result_print("Vendor ID: ",result);
		hres &= result = msg.buffer[++ioc] == 0;
		hres &= result &= msg.buffer[++ioc] == (octet)1;
		hres &= result &= msg.buffer[++ioc] == (octet)2;
		hres &= result &= msg.buffer[++ioc] == (octet)3;
		hres &= result &= msg.buffer[++ioc] == (octet)4;
		hres &= result &= msg.buffer[++ioc] == (octet)5;
		hres &= result &= msg.buffer[++ioc] == (octet)6;
		hres &= result &= msg.buffer[++ioc] == (octet)7;
		hres &= result &= msg.buffer[++ioc] == (octet)8;
		hres &= result &= msg.buffer[++ioc] == (octet)9;
		hres &= result &= msg.buffer[++ioc] == (octet)10;
		hres &= result &= msg.buffer[++ioc] == (octet)11;
		result_print("Guid Prefix: ",result);
		result_print_part("HEADER: ",hres);
		cout << B_WHITE << "Checking SubmessageHeader " << DEF << endl;
		hres = result = msg.buffer[++ioc] == DATA;
		result_print("SubMessageId ",result);
		hres &= result = msg.buffer[++ioc] == flags;
		result_print("Flags: ",result);
		//FIND OUT IS IT IS BIG OR LITTLE ENDIAN
		bool endianness;
		octet end = (flags & BIT(0));
		//		cout << "FLAGS BITS: " <<(bitset<8>) flags << endl;
		//		cout << "ENDIA BITS: "<<(bitset<8>) end << endl;
		if(end == 0x00)
			endianness = LITTLEEND; //LITTLEEND
		else if(end == 0x01)
			endianness = BIGEND;  //BIGEND
		octet* p;
		p = &msg.buffer[++ioc];
		unsigned short size;
		//cout << "Msg endianness CDR: " << endianness << " msg struct: " << DataSubM.SubmessageHeader.flags[0] << " default: " << DEFAULT_ENDIAN << endl;
		if(endianness == DEFAULT_ENDIAN)//!endianness)
			size = *(unsigned short*)p;
		else
		{
			memcpy((octet*)&size,p+1,1);
			memcpy((octet*)(&size)+1,p,1);
		}
		//cout << "Size CDR: " << size << " vs size submsg: " <<DataSubM.SubmessageHeader.submessageLength << endl;
		hres &= result = size == DataSubM.SubmessageHeader.submessageLength;
		ioc++;
		result_print("Syze in Bytes: ",result);
		result_print_part("SUBMESSAGEHEADER: ",hres);
		cout << B_WHITE << "Checking EXTRA FLAGS AND OCTETS TO INLINE QOS " << DEF << endl;
		hres = result = msg.buffer[++ioc] == 0x0;
		hres &= result &= msg.buffer[++ioc] == 0x0;
		result_print("Extra Flags: ",result);
		p = &msg.buffer[++ioc];
		//cout << "Msg endianness CDR: " << endianness << " msg struct: " << DataSubM.SubmessageHeader.flags[0] << " default: " << DEFAULT_ENDIAN << endl;
		if(endianness == DEFAULT_ENDIAN)//!endianness)
			size = *(unsigned short*)p;
		else
		{
			memcpy((octet*)&size,p+1,1);
			memcpy((octet*)(&size)+1,p,1);
		}
		hres &= result = size == RTPSMESSAGE_OCTETSTOINLINEQOS_DATASUBMSG;
		result_print("Octets to InlineQos: ",result);
		result_print_part("FLAGS AND INLINE: ",hres);
		cout << B_WHITE << "Checking EntityIDs and SeqNum " << DEF << endl;
		ioc++;
		EntityId_t ID;
		readEntityId(&ID,&msg.buffer[++ioc],endianness);
		hres = result = ID == DataSubM.readerId;
		result_print("Reader ID: ",result);
		ioc += 4;
		readEntityId(&ID,&msg.buffer[ioc],endianness);
		hres &= result = ID == DataSubM.writerId;
		result_print("Writer ID: ",result);
		//SequenceNumber
		SequenceNumber_t sn;
		ioc+=4;
		readSequenceNumber(&sn,&msg.buffer[ioc],endianness);
		hres &= result = sn == DataSubM.writerSN;
		result_print("Sequence Number: ",result);
		result_print_part("ENTITY AND SEQNUM  ",hres);
		cout << B_WHITE << "Parameter List " << DEF << endl;
		ioc+=8;
		octet inlineflag = flags & BIT(1);
		octet serializedpayloadflag = flags & BIT(2);
		if(inlineflag)
		{
			vector<Parameter_t> plist;
			unsigned long param_list_size;
			readParameterList(&plist,&msg.buffer[ioc],endianness,&param_list_size);
			cout << "Total length: " << param_list_size << " in parameters: " << plist.size() << endl;
			result = compareParameterLists(&DataSubM.inlineQos,&plist);
			hres = result;
			result_print_part("Param List  ",hres);
			ioc+=param_list_size;
		}
		if(serializedpayloadflag)
		{
			octet* data = (octet*)malloc(sizeof(uint16_t)*10);
			readData(data,&msg.buffer[ioc],sizeof(uint16_t)*10);
			uint16_t* dd = (uint16_t*)data;
			hres = result = true;
			for(uint i = 0;i<10;i++){
				//cout << "Number, data: " << numbers[i] << " " << dd[i]<< endl;
				hres &= result = numbers[i] == dd[i];
				result_print("payload: ",result);
			}
			result_print_part("PAYLOAD  ",hres);
		}

		cout << "Leida" << endl;
	}
	else{
		cout << "Fallo en creacion mensaje" << endl;
		return false;
	}

	return true;
}
