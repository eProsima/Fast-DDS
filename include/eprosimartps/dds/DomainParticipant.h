/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DomainParticipant.h
 *	Singleton DomainParticipant.
 *  Created on: Feb 27, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#include <iostream>

#include "eprosimartps/rtps_all.h"

#include "eprosimartps/dds/Publisher.h"
#include "eprosimartps/dds/Subscriber.h"
#include "eprosimartps/Participant.h"

#ifndef DOMAINPARTICIPANT_H_
#define DOMAINPARTICIPANT_H_


namespace eprosima {
namespace dds {


/**
 * Class DomainParticipant, contains the static functions to create Publishers and Subscribers, as well as to register types. 
 * It can be directly accessed by the user. Is a singleton, so only one instance is ever created.
  * @ingroup DDSMODULE
 */
class RTPS_DllAPI DomainParticipant {
public:

private:
	uint32_t id;
    static bool instanceFlag;
    static DomainParticipant *single;
    DomainParticipant()
    {
        id = 0;//private constructor
    }
    std::vector<TypeReg_t> typesRegistered;
public:
	/**
	 * @brief Create a Publisher in the given Participant. 
	 * @param p Pointer to the Participant.
	 * @param WParam Writer Parameters to create a Publisher.
	 * @return Pointer to the publisher. 
	 */
    static Publisher* createPublisher(Participant* p,WriterParams_t WParam);
	/**
	 * @brief Create a Subscriber in the given Participant. 
	 * @param p Pointer to the Participant.
	 * @param RParam Reader Parameters to create a Publisher.
	 * @return Pointer to the subscriber. 
	 */
    static Subscriber* createSubscriber(Participant* p,ReaderParams_t RParam);

    /**
     * @brief Create a Participant.
     * @param PParam Participant Parameters.
     * @return Pointer to the participant.
     */
    static Participant* createParticipant(ParticipantParams_t PParam);


	/**
	 * @brief Register a type to be used in the communication. Serialized and deserialized functions must be provided, as well as the byte size of the type.
	 * @param in_str Type Name. Must be unique.
	 * @param serialize Pointer to serialization function.
	 * @param deserialize Pointer to deserialization function.
	 * @param getKey Pointer to getKeyhashFunction.
	 * @param size Size in bytes of the type.
	 * @return True if suceeded. 
	 */
    static bool registerType(std::string in_str, void (*serialize)(SerializedPayload_t*data,void*)
    		,void (*deserialize)(SerializedPayload_t*data,void*),
    		void (*getKey)(void*,InstanceHandle_t*),int32_t size);

	/**
	 * @brief Get pointer to the unique instance of this class.
	 * @return Pointer to the instance.
	 */
    static DomainParticipant* getInstance();
	/**
	 * @brief Get Id to create a participant. 
	 * @return Different ID for each call.
	 */
    uint32_t getNewId();
    ~DomainParticipant()
    {
    	instanceFlag = false;
    }



};

} /* namespace dds */
} /* namespace eprosima */

#endif /* DOMAINPARTICIPANT_H_ */
