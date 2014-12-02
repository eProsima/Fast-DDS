/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file EDPStatic.h
 *
 */

#ifndef EDPSTATIC_H_
#define EDPSTATIC_H_


#include "fastrtps/rtps/builtin/discovery/endpoint/EDP.h"


namespace eprosima {
namespace fastrtps{
namespace rtps {

class EDPStaticXML;

/**
 * Class EDPStaticProperty, used to read and write the strings from the properties used to transmit the EntityId_t.
 * @ingroup DISCOVERYMODULE
 */
class EDPStaticProperty
{
public:
	EDPStaticProperty():m_userId(0){};
	~EDPStaticProperty(){};
	//!Endpoint type
	std::string m_endpointType;
	//!Status
	std::string m_status;
	//!User ID as string
	std::string m_userIdStr;
	//!User ID
	uint16_t m_userId;
	//!Entity ID
	EntityId_t m_entityId;
	/**
	* @param type
	* @param status
	* @param id
	* @param end
	* @return
	*/
	static std::pair<std::string,std::string> toProperty(std::string type,std::string status,uint16_t id,const EntityId_t& ent);
	/**
	* @param property
	* @return
	*/
	bool fromProperty(std::pair<std::string,std::string> property);
};

/**
 * Class EDPStatic, implements a static endpoint discovery module.
 * @ingroup DISCOVERYMODULE
 */
class EDPStatic : public EDP {
public:
	/**
	* @param p
	* @param part
	*/
	EDPStatic(PDPSimple* p,RTPSParticipantImpl* part);
	virtual ~EDPStatic();
	/**
	 * Abstract method to initialize the EDP.
	 * @param attributes DiscoveryAttributes structure.
	 * @return True if correct.
	 */
	bool initEDP(BuiltinAttributes& attributes);
	/**
	 * Abstract method that assigns remote endpoints when a new RTPSParticipantProxyData is discovered.
	 * @param pdata
	 */
	void assignRemoteEndpoints(ParticipantProxyData* pdata);
	/**
	 * Abstract method that removes a local Reader from the discovery method
	 * @param R Pointer to the Reader to remove.
	 * @return True if correctly removed.
	 */
	bool removeLocalReader(RTPSReader* R);
	/**
	 * Abstract method that removes a local Writer from the discovery method
	 * @param W Pointer to the Writer to remove.
	 * @return True if correctly removed.
	 */
	bool removeLocalWriter(RTPSWriter*W);

	/**
	 * After a new local ReaderProxyData has been created some processing is needed (depends on the implementation).
	 * @param rdata Pointer to the ReaderProxyData object.
	 * @return True if correct.
	 */
	bool processLocalReaderProxyData(ReaderProxyData* rdata);
	/**
	 * After a new local WriterProxyData has been created some processing is needed (depends on the implementation).
	 * @param wdata Pointer to the Writer ProxyData object.
	 * @return True if correct.
	 */
	bool processLocalWriterProxyData(WriterProxyData* wdata);

	/**
	 * New Remote Writer has been found and this method process it and calls the pairing methods.
	 * @param pdata Pointer to the RTPSParticipantProxyData object.
	 * @param userId UserId.
	 * @param entId EntityId.
	 * @return True if correct.
	 */
	bool newRemoteWriter(ParticipantProxyData*pdata,uint16_t userId, EntityId_t entId=c_EntityId_Unknown);
	/**
	 * New Remote Reader has been found and this method process it and calls the pairing methods.
	 * @param pdata Pointer to the RTPSParticipantProxyData object.
	 * @param userId UserId.
	 * @param entId EntityId.
	 * @return true if correct.
	 */
	bool newRemoteReader(ParticipantProxyData*pdata,uint16_t userId, EntityId_t entId=c_EntityId_Unknown);
	/**
	* This method checks the provided entityId against the topic type to see if it matches
	* @param rdata Pointer to the readerProxyData
	* @return True if its correct. 
	**/
	bool checkEntityId(ReaderProxyData* rdata);
	/**
	* This method checks the provided entityId against the topic type to see if it matches
	* @param rdata Pointer to the writerProxyData
	* @return True if its correct. 
	**/
	bool checkEntityId(WriterProxyData* wdata);
private:
	EDPStaticXML* mp_edpXML;
	BuiltinAttributes m_attributes;
};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* EDPSTATIC_H_ */
