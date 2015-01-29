/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file Guid.h 	
 */

#ifndef RTPS_GUID_H_
#define RTPS_GUID_H_
#include <cstdint>
#include "fastrtps/config/fastrtps_dll.h"
#include "fastrtps/rtps/common/Types.h"

namespace eprosima{
namespace fastrtps{
namespace rtps{



//!@brief Structure GuidPrefix_t, Guid Prefix of GUID_t.
//!@ingroup COMMON_MODULE
struct RTPS_DllAPI GuidPrefix_t{
	octet value[12];
	//!Default constructor. Set the Guid prefix to 0.
	GuidPrefix_t()
	{
		for(uint8_t i =0;i<12;i++)
			value[i] = 0;
	}
	/**
	* Guid prefix constructor
	* @param guid Guid prefix
	*/
	GuidPrefix_t(octet guid[12]){
		for(uint8_t i =0;i<12;i++)
			value[i] = guid[i];
	}
	/**
	* Guid prefix assignment operator
	* @param guidpre Guid prefix to copy the values from
	*/
	GuidPrefix_t& operator=(const GuidPrefix_t& guidpre)
	{
		for(uint8_t i =0;i<12;i++)
		{
			value[i] = guidpre.value[i];
		}
		return *this;
	}
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

	/**
	* Guid prefix comparison operator
	* @param guid1 First guid prefix to compare
	* @param guid2 Second guid prefix to compare
	* @return True if the guid prefixes are equal
	*/
inline bool operator==(const GuidPrefix_t& guid1,const GuidPrefix_t& guid2)
{
	for(uint8_t i =0;i<12;i++)
	{
		if(guid1.value[i] != guid2.value[i])
			return false;
	}
	return true;
}

	/**
	* Guid prefix comparison operator
	* @param guid1 First guid prefix to compare
	* @param guid2 Second guid prefix to compare
	* @return True if the guid prefixes are not equal
	*/
inline bool operator!=(const GuidPrefix_t& guid1,const GuidPrefix_t& guid2)
{
	for(uint8_t i =0;i<12;i++)
	{
		if(guid1.value[i] != guid2.value[i])
			return true;
	}
	return false;
}

#endif

const GuidPrefix_t c_GuidPrefix_Unknown;


inline std::ostream& operator<<(std::ostream& output,const GuidPrefix_t& guiP){
	output << std::hex;
	for(uint8_t i =0;i<11;++i)
		output<<(int)guiP.value[i]<<".";
	output << (int)guiP.value[11];
	return output<<std::dec;
}

#define ENTITYID_UNKNOWN 0x00000000
#define ENTITYID_RTPSParticipant  0x000001c1
#define ENTITYID_SEDP_BUILTIN_TOPIC_WRITER  0x000002c2
#define ENTITYID_SEDP_BUILTIN_TOPIC_READER 0x000002c7
#define ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER  0x000003c2
#define ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER  0x000003c7
#define ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER 0x000004c2
#define ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER  0x000004c7
#define ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER  0x000100c2
#define ENTITYID_SPDP_BUILTIN_RTPSParticipant_READER  0x000100c7
#define ENTITYID_P2P_BUILTIN_RTPSParticipant_MESSAGE_WRITER  0x000200C2
#define ENTITYID_P2P_BUILTIN_RTPSParticipant_MESSAGE_READER  0x000200C7

//!@brief Structure EntityId_t, entity id part of GUID_t.
//!@ingroup COMMON_MODULE
 struct RTPS_DllAPI EntityId_t{
	octet value[4];
	//! Default constructor. Uknown entity.
	EntityId_t(){
		*this = ENTITYID_UNKNOWN;
	}
	/**
	* Main constructor.
	* @param id Entity id
	*/
	EntityId_t(uint32_t id)
	{
		uint32_t* aux = (uint32_t*)(value);
		*aux = id;
		reverse();
	}
	/**
	* Assignment operator.
	* @param id Entity to copy values from
	*/
	EntityId_t& operator=(const EntityId_t& id)
	{
		value[0] = id.value[0];
		value[1] = id.value[1];
		value[2] = id.value[2];
		value[3] = id.value[3];
		return *this;
	}
	/**
	* Assignment operator.
	* @param id Entity id to copy
	*/
	EntityId_t& operator=(uint32_t id){
		uint32_t* aux = (uint32_t*)(value);
		*aux = id;
		if(DEFAULT_ENDIAN == LITTLEEND)
			reverse();
		return *this;
		//return id;
	}
	//! 
	void reverse(){
		octet oaux;
		oaux = value[3];
		value[3] = value[0];
		value[0] = oaux;
		oaux = value[2];
		value[2] = value[1];
		value[1] = oaux;
	}
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

	/**
	* Guid prefix comparison operator
	* @param id1 EntityId to compare
	* @param id2 ID prefix to compare
	* @return True if equal
	*/
inline bool operator==(EntityId_t& id1,const uint32_t id2)
{
	if(DEFAULT_ENDIAN == LITTLEEND)
		id1.reverse();
	uint32_t* aux1 = (uint32_t*)(id1.value);
	bool result = true;
	if(*aux1 == id2)
		result = true;
	else
		result = false;
	if(DEFAULT_ENDIAN == LITTLEEND)
		id1.reverse();
	return result;
}
	/**
	* Guid prefix comparison operator
	* @param id1 First EntityId to compare
	* @param id2 Second EntityId to compare
	* @return True if equal
	*/
inline bool operator==(const EntityId_t& id1,const EntityId_t& id2)
{
	for(uint8_t i =0;i<4;++i)
	{
		if(id1.value[i] != id2.value[i])
			return false;
	}
	return true;
}

	/**
	* Guid prefix comparison operator
	* @param id1 First EntityId to compare
	* @param id2 Second EntityId to compare
	* @return True if not equal
	*/
inline bool operator!=(const EntityId_t& id1,const EntityId_t& id2)
{
	for(uint8_t i =0;i<4;++i)
	{
		if(id1.value[i] != id2.value[i])
			return true;
	}
	return false;
}

#endif

inline std::ostream& operator<<(std::ostream& output,const EntityId_t& enI){
	output << std::hex;
	output<<(int)enI.value[0]<<"."<<(int)enI.value[1]<<"."<<(int)enI.value[2]<<"."<<(int)enI.value[3];
	return output << std::dec;
}


const EntityId_t c_EntityId_Unknown = ENTITYID_UNKNOWN;
const EntityId_t c_EntityId_SPDPReader = ENTITYID_SPDP_BUILTIN_RTPSParticipant_READER;
const EntityId_t c_EntityId_SPDPWriter = ENTITYID_SPDP_BUILTIN_RTPSParticipant_WRITER;

const EntityId_t c_EntityId_SEDPPubWriter = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER;
const EntityId_t c_EntityId_SEDPPubReader = ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER;
const EntityId_t c_EntityId_SEDPSubWriter = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER;
const EntityId_t c_EntityId_SEDPSubReader = ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER;

const EntityId_t c_EntityId_RTPSParticipant = ENTITYID_RTPSParticipant;

const EntityId_t c_EntityId_WriterLiveliness = ENTITYID_P2P_BUILTIN_RTPSParticipant_MESSAGE_WRITER;
const EntityId_t c_EntityId_ReaderLiveliness = ENTITYID_P2P_BUILTIN_RTPSParticipant_MESSAGE_READER;



//!@brief Structure GUID_t, entity identifier, unique in DDS-RTPS Domain.
//!@ingroup COMMON_MODULE
 struct RTPS_DllAPI GUID_t{
	//!Guid prefix
	GuidPrefix_t guidPrefix;
	//!Entity id
	EntityId_t entityId;
	/**
	* Assignment operator
	* @param guid GUID to copy the data from.
	*/
	GUID_t& operator=(const GUID_t& guid)
	{
		guidPrefix = guid.guidPrefix;
		entityId = guid.entityId;
		return *this;
	}
	
	//! Default constructor
	GUID_t(){};
	
	/**
	* @param guidP Guid prefix
	* @param id Entity id
	*/	
	GUID_t(const GuidPrefix_t& guidP,uint32_t id):
		guidPrefix(guidP),entityId(id) {}
	
	/**
	* @param guidP Guid prefix
	* @param entId Entity id
	*/	
	GUID_t(const GuidPrefix_t& guidP,const EntityId_t& entId):
		guidPrefix(guidP),entityId(entId) {}
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

	/**
	* GUID comparison operator
	* @param g1 First GUID to compare
	* @param g2 Second GUID to compare
	* @return True if equal
	*/
inline bool operator==(const GUID_t& g1,const GUID_t& g2){
	if(g1.guidPrefix == g2.guidPrefix && g1.entityId==g2.entityId)
		return true;
	else
		return false;
}

	/**
	* GUID comparison operator
	* @param g1 First GUID to compare
	* @param g2 Second GUID to compare
	* @return True if not equal
	*/
inline bool operator!=(const GUID_t& g1,const GUID_t& g2){
	if(g1.guidPrefix != g2.guidPrefix || g1.entityId!=g2.entityId)
		return true;
	else
		return false;
}

inline bool operator<(const GUID_t& g1, const GUID_t& g2){
	for (uint8_t i = 0; i < 12; ++i)
	{
		if (g1.guidPrefix.value[i] < g2.guidPrefix.value[i])
			return true;
	}
	for (uint8_t i = 0; i < 4; ++i)
	{
		if (g1.entityId.value[i] < g2.entityId.value[i])
			return true;
	}
	return false;
}
#endif

const GUID_t c_Guid_Unknown;

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

	/**
	* Stream operator, prints a GUID.
	* @param output Output stream.
	* @param guid GUID_t to print.
	* @return Stream operator.
	*/
inline std::ostream& operator<<(std::ostream& output,const GUID_t& guid)
{
	if(guid !=c_Guid_Unknown)
		output<<guid.guidPrefix<<"|"<<guid.entityId;
	else
		output << "|GUID UNKNOWN|";
	return output;
}

#endif

}
}
}


#endif /* RTPS_GUID_H_ */
