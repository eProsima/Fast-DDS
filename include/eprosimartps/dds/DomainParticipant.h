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



#ifndef DOMAINPARTICIPANT_H_
#define DOMAINPARTICIPANT_H_

#include <iostream>

#include "eprosimartps/rtps_all.h"

#include "eprosimartps/dds/Publisher.h"
#include "eprosimartps/dds/Subscriber.h"
//#include "eprosimartps/Participant.h"
#include "eprosimartps/utils/IPFinder.h"
#include "eprosimartps/dds/DDSTopicDataType.h"

namespace eprosima{
namespace rtps{
class Participant;
}
}

using namespace eprosima::rtps;

namespace eprosima {
namespace dds {


/**
 * Class DomainParticipant, contains the static functions to create Publishers and Subscribers, as well as to register types. 
 * It can be directly accessed by the user. Is a singleton, so only one instance is ever created.
  * @ingroup DDSMODULE
 */
class RTPS_DllAPI DomainParticipant {

private:
	uint32_t id;
    static bool instanceFlag;
    static DomainParticipant *single;
    DomainParticipant();
    std::vector<DDSTopicDataType*> m_registeredTypes;
    std::vector<Participant*> m_participants;
public:
    /**
     * Method to shut down all participants, readers, writers, etc.
     * It must be called at the end of the process to avoid memory leaks.
     */
    static void stopAll();
	/**
	 * @brief Create a Publisher in the given Participant. 
	 * @param p Pointer to the Participant.
	 * @param WParam Writer Parameters to create a Publisher.
	 * @return Pointer to the publisher. 
	 */
    static Publisher* createPublisher(Participant* p,const PublisherAttributes& WParam);
	/**
	 * @brief Create a Subscriber in the given Participant. 
	 * @param p Pointer to the Participant.
	 * @param RParam Reader Parameters to create a Publisher.
	 * @return Pointer to the subscriber. 
	 */
    static Subscriber* createSubscriber(Participant* p,const SubscriberAttributes& RParam);

    /**
     * @brief Create a Participant.
     * @param PParam Participant Parameters.
     * @return Pointer to the participant.
     */
    static Participant* createParticipant(const ParticipantAttributes& PParam);

    /**
     * Remove a participant and delete all its associated Writers, Readers, threads, etc.
     * @param[in] p Pointer to the Participant;
     * @return True if correct.
     */
    static bool removeParticipant(Participant* p);
    /**
     * Remove a publisher from the Participant.
     */
    static bool removePublisher(Participant* p,Publisher* pub);
    /**
     * Remove a subscriber from a participant.
     */
    static bool removeSubscriber(Participant* p,Subscriber* sub);

    /**
     * Register a type in the domain.
     * The name must be unique, the size > 0.
     * @param[in] type Pointer to the Data Type Object.
     * @return True if correct.
     */
    static bool registerType(DDSTopicDataType* type);

    /**
     * Get a pointer to a registered type.
     * @param[in] type_name Name of the type to add.
     * @param[out] type_ptr Pointer to pointer of the type, is used to return the object.
     * @return True if the type is found.
     */
    static bool getRegisteredType(std::string type_name,DDSTopicDataType** type_ptr);


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
    ~DomainParticipant();

    /**
     * Set the parameters used to calculate the default ports in the discovery.
     */
    void setPortParameters(uint16_t PB,uint16_t DG,uint16_t PG,uint16_t d0,uint16_t d1,uint16_t d2,uint16_t d3)
    {
    	m_portBase = PB;
    	m_domainIdGain = DG;
    	m_participantIdGain = PG;
    	m_offsetd0 = d0;
    	m_offsetd1 = d1;
    	m_offsetd2 = d2;
    	m_offsetd3 = d3;
    }
    /** @name Methods to get the Port parameters.
     */

    	///@{
	uint16_t getDomainIdGain() const {
		return m_domainIdGain;
	}

	uint16_t getOffsetd0() const {
		return m_offsetd0;
	}

	uint16_t getOffsetd1() const {
		return m_offsetd1;
	}

	uint16_t getOffsetd2() const {
		return m_offsetd2;
	}

	uint16_t getOffsetd3() const {
		return m_offsetd3;
	}

	uint16_t getParticipantIdGain() const {
		return m_participantIdGain;
	}

	uint16_t getPortBase() const {
		return m_portBase;
	}
	///@}

	/**
	 * Get IP addresses of the machine
	 * @param[out] locators Pointer to vector of locators used to return the different IPs of the machine.
	 *
	 */
	static void getIPAddress(std::vector<Locator_t>* locators);

private:
    uint16_t m_portBase;
    uint16_t m_domainIdGain;
    uint16_t m_participantIdGain;
    uint16_t m_offsetd0;
    uint16_t m_offsetd1;
    uint16_t m_offsetd2;
    uint16_t m_offsetd3;
    IPFinder m_IPFinder;

    std::vector<Publisher*> m_publisherList;
    std::vector<Subscriber*> m_subscriberList;

    uint16_t m_DomainId;


};

} /* namespace dds */
} /* namespace eprosima */

#endif /* DOMAINPARTICIPANT_H_ */
