/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Guid.h 	
 */

#ifndef RTPS_GUID_H_
#define RTPS_GUID_H_
#include <cstdint>
#include "eprosimartps/common/types/common_types.h"

namespace eprosima{
namespace rtps{

#define GUIDPREFIX_UNKNOWN(g) {for(uint8_t i=0;i<12;i++) g.value[i]=0x0;}

//!@brief Structure GuidPrefix_t, Guid Prefix of GUID_t.
typedef struct GuidPrefix_t{
	octet value[12];
	GuidPrefix_t()
	{
		for(uint8_t i =0;i<12;i++)
			value[i] = 0;
	}
	GuidPrefix_t(octet guid[12]){
		for(uint8_t i =0;i<12;i++)
			value[i] = guid[i];
	}
	GuidPrefix_t& operator=(const GuidPrefix_t& guidpre)
	{
		for(uint8_t i =0;i<12;i++)
		{
			value[i] = guidpre.value[i];
		}
		return *this;
	}
}GuidPrefix_t;

inline bool operator==(const GuidPrefix_t& guid1,const GuidPrefix_t& guid2)
{
	for(uint8_t i =0;i<12;i++)
	{
		if(guid1.value[i] != guid2.value[i])
			return false;
	}
	return true;
}

const GuidPrefix_t c_GuidPrefix_Unknown;

inline std::ostream& operator<<(std::ostream& output,const GuidPrefix_t& guiP){
	output << std::hex;
	for(uint8_t i =0;i<12;++i)
		output<<(int)guiP.value[i]<<".";
	return output<<std::dec;
}

#define ENTITYID_UNKNOWN 0x00000000
#define ENTITYID_PARTICIPANT  0x000001c1
#define ENTITYID_SEDP_BUILTIN_TOPIC_WRITER  0x000002c2
#define ENTITYID_SEDP_BUILTIN_TOPIC_READER 0x000002c7
#define ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER  0x000003c2
#define ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER  0x000003c7
#define ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER 0x000004c2
#define ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER  0x000004c7
#define ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER  0x000100c2
#define ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER  0x000100c7
#define ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER  0x000200C2
#define ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER  0x000200C7

//!@brief Structure EntityId_t, entity id part of GUID_t.
typedef struct EntityId_t{
	octet value[4];
	EntityId_t(){
		*this = ENTITYID_UNKNOWN;
	}
	EntityId_t(uint32_t id)
	{
		uint32_t* aux = (uint32_t*)(value);
		*aux = id;
		reverse();
	}
	EntityId_t& operator=(const EntityId_t& id)
	{
		value[0] = id.value[0];
		value[1] = id.value[1];
		value[2] = id.value[2];
		value[3] = id.value[3];
		return *this;
	}
	EntityId_t& operator=(uint32_t id){
		uint32_t* aux = (uint32_t*)(value);
		*aux = id;
		if(DEFAULT_ENDIAN == LITTLEEND)
			reverse();
		return *this;
		//return id;
	}
	void reverse(){
		octet oaux;
		oaux = value[3];
		value[3] = value[0];
		value[0] = oaux;
		oaux = value[2];
		value[2] = value[1];
		value[1] = oaux;
	}
}EntityId_t;

inline bool operator==(EntityId_t& eid,const uint32_t id2)
{
	if(DEFAULT_ENDIAN == LITTLEEND)
		eid.reverse();
	uint32_t* aux1 = (uint32_t*)(eid.value);
	bool result = true;
	if(*aux1 == id2)
		result = true;
	else
		result = false;
	if(DEFAULT_ENDIAN == LITTLEEND)
		eid.reverse();
	return result;
}
inline bool operator==(const EntityId_t& id1,const EntityId_t& id2){
	uint32_t* aux1 = (uint32_t*)(id1.value);
	uint32_t* aux2 = (uint32_t*)(id2.value);
	if(*aux1 == *aux2)
		return true;
	return false;
}




inline std::ostream& operator<<(std::ostream& output,const EntityId_t& enI){
	output << std::hex;
	for(uint8_t i =0;i<4;++i)
		output<<(int)enI.value[i]<<".";
	return output << std::dec;
}


const EntityId_t c_EntityId_Unknown = ENTITYID_UNKNOWN;
const EntityId_t c_EntityId_SPDPReader = ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER;
const EntityId_t c_EntityId_SPDPWriter = ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER;

const EntityId_t c_EntityId_SEDPPubWriter = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER;
const EntityId_t c_EntityId_SEDPPubReader = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER;
const EntityId_t c_EntityId_SEDPSubWriter = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER;
const EntityId_t c_EntityId_SEDPSubReader = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER;

const EntityId_t c_EntityId_Participant = ENTITYID_PARTICIPANT;



//!@brief Structure GUID_t, entity identifier, unique in DDS Domain.
typedef struct GUID_t{
	GuidPrefix_t guidPrefix;
	EntityId_t entityId;
	GUID_t& operator=(const GUID_t& guid)
	{
		guidPrefix = guid.guidPrefix;
		entityId = guid.entityId;
		return *this;
	}

	GUID_t(){};
	GUID_t(const GuidPrefix_t& guidP,uint32_t id):
		guidPrefix(guidP),entityId(id) {}
	GUID_t(const GuidPrefix_t& guidP,const EntityId_t& entId):
		guidPrefix(guidP),entityId(entId) {}
}GUID_t;

inline bool operator==(const GUID_t& g1,const GUID_t& g2){
	if(g1.guidPrefix == g2.guidPrefix && g1.entityId==g2.entityId)
		return true;
	else
		return false;
}

#define GUID_UNKNOWN(gui) {GUIDPREFIX_UNKNOWN(gui.guidPrefix); gui.entityId = ENTITYID_UNKNOWN;}


inline std::ostream& operator<<(std::ostream& output,const GUID_t& guid)
{
	output<<guid.guidPrefix<<"|"<<guid.entityId;
	return output;
}


}
}


#endif /* RTPS_GUID_H_ */
