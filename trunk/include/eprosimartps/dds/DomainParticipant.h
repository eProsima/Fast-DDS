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

#include "eprosimartps/common/types/common_types.h"
#include "eprosimartps/dds/attributes/all_attributes.h"



namespace eprosima{

namespace rtps{
class Participant;
class ParticipantImpl;

typedef std::pair<Participant*,ParticipantImpl*> ParticipantPair;

}


using namespace rtps;

namespace dds {

class DDSTopicDataType;
class Publisher;
class Subscriber;
class Publisher;
class PublisherImpl;
class Subscriber;
class SubscriberImpl;
class PublisherListener;
class SubscriberListener;

typedef std::pair<Subscriber*,SubscriberImpl*> SubscriberPair;
typedef std::pair<Publisher*,PublisherImpl*> PublisherPair;

/**
 * Class DomainParticipant, contains the static functions to create Publishers and Subscribers, as well as to register types. 
 * It can be directly accessed by the user. Is a singleton, so only one instance is ever created.
  * @ingroup DDSMODULE
 */
class DomainParticipantImpl
{
private:
	  DomainParticipantImpl();
    /**
     * DomainParticipant destructor
     */
    ~DomainParticipantImpl();

    bool getParticipantImpl(Participant*,ParticipantImpl**);

public:
    /**
     * Method to shut down all participants, readers, writers, etc.
     * It must be called at the end of the process to avoid memory leaks.
     * It also shut downs the DomainParticipant.
     */
    void stopAll();
	/**
	 * @brief Create a Publisher in the given Participant. 
	 * @param p Pointer to the Participant.
	 * @param WParam Writer Parameters to create a Publisher.
	 * @return Pointer to the publisher. 
	 */
    Publisher* createPublisher(Participant* p, PublisherAttributes& WParam,PublisherListener* plisten=NULL);
	/**
	 * @brief Create a Subscriber in the given Participant. 
	 * @param p Pointer to the Participant.
	 * @param RParam Reader Parameters to create a Publisher.
	 * @return Pointer to the subscriber. 
	 */
    Subscriber* createSubscriber(Participant* p, SubscriberAttributes& RParam,SubscriberListener* slisten=NULL);

    /**
     * @brief Create a Participant.
     * @snippet dds_example.cpp ex_ParticipantCreation
     * @param PParam Participant Parameters.
     * @return Pointer to the participant.
     */
    Participant* createParticipant(const ParticipantAttributes& PParam);

    /**
     * Remove a participant and delete all its associated Writers, Readers, resources, etc.
     * @param[in] p Pointer to the Participant;
     * @return True if correct.
     */
    bool removeParticipant(Participant* p);
    /**
     * Remove a publisher from the Participant.
     */
    bool removePublisher(Participant* p,Publisher* pub);
    /**
     * Remove a subscriber from a participant.
     */
    bool removeSubscriber(Participant* p,Subscriber* sub);

    /**
     * Register a type in the domain.
     * The name must be unique, the size > 0.
     * @param[in] type Pointer to the Data Type Object.
     * @return True if correct.
     */
    bool registerType(DDSTopicDataType* type);

    /**
     * Get a pointer to a registered type.
     * @param[in] type_name Name of the type to add.
     * @param[out] type_ptr Pointer to pointer of the type, is used to return the object.
     * @return True if the type is found.
     */
    bool getRegisteredType(std::string type_name,DDSTopicDataType** type_ptr);


	/**
	 * @brief Get pointer to the unique instance of this class.
	 * @return Pointer to the instance.
	 */
    static DomainParticipantImpl* getInstance();



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
    /** @name Methods to get the default Port numbers.
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

	void setMaxParticipantId(uint32_t maxParticipantId) {
		m_maxParticipantID = maxParticipantId;
	}

	///@}

	uint32_t getMulticastPort(uint32_t domainId)
	{
		return getPortBase()+ getDomainIdGain() * domainId+ getOffsetd0();
	}
	uint32_t getUnicastPort(uint32_t domainId,uint32_t participantID)
	{
		return getPortBase()+ getDomainIdGain() * domainId	+ getOffsetd1()	+ getParticipantIdGain() * participantID;
	}


private:
	uint32_t m_maxParticipantID;
	static bool instanceFlag;
	static DomainParticipantImpl *single;
	std::vector<DDSTopicDataType*> m_registeredTypes;

	uint16_t m_portBase;
	uint16_t m_domainIdGain;
	uint16_t m_participantIdGain;
	uint16_t m_offsetd0;
	uint16_t m_offsetd1;
	uint16_t m_offsetd2;
	uint16_t m_offsetd3;

	std::vector<PublisherPair> m_publisherList;
	std::vector<SubscriberPair> m_subscriberList;
	std::vector<ParticipantPair> m_participants;

	uint16_t m_DomainId;

	/**
	 * @brief Get Id to create a participant.
	 * @return Different ID for each call.
	 */
	uint32_t getNewId();

};



class RTPS_DllAPI DomainParticipant
{
public:
	/**
	 * Method to shut down all participants, readers, writers, etc.
	 * It must be called at the end of the process to avoid memory leaks.
	 * It also shut downs the DomainParticipant.
	 */
	static void stopAll()
	{
		DomainParticipantImpl::getInstance()->stopAll();
	}
	/**
	 * @brief Create a Publisher in the given Participant.
	 * @param p Pointer to the Participant.
	 * @param WParam Writer Parameters to create a Publisher.
	 * @return Pointer to the publisher.
	 */
	static Publisher* createPublisher(Participant* p, PublisherAttributes& WParam,PublisherListener* plisten=NULL)
	{
		return (DomainParticipantImpl::getInstance()->createPublisher(p,WParam,plisten));
	}
	/**
	 * @brief Create a Subscriber in the given Participant.
	 * @param p Pointer to the Participant.
	 * @param RParam Reader Parameters to create a Publisher.
	 * @return Pointer to the subscriber.
	 */
	static Subscriber* createSubscriber(Participant* p, SubscriberAttributes& RParam,SubscriberListener* slisten=NULL)
	{
		return (DomainParticipantImpl::getInstance()->createSubscriber(p,RParam,slisten));
	}
	/**
	 * @brief Create a Participant.
	 * @snippet dds_example.cpp ex_ParticipantCreation
	 * @param PParam Participant Parameters.
	 * @return Pointer to the participant.
	 */
	static Participant* createParticipant(const ParticipantAttributes& PParam)
	{
		return (DomainParticipantImpl::getInstance()->createParticipant(PParam));
	}
	/**
	 * Remove a participant and delete all its associated Writers, Readers, resources, etc.
	 * @param[in] p Pointer to the Participant;
	 * @return True if correct.
	 */
	static bool removeParticipant(Participant* p)
	{
		return (DomainParticipantImpl::getInstance()->removeParticipant( p));
	}
	/**
	 * Remove a publisher from the Participant.
	 */
	static bool removePublisher(Participant* p,Publisher* pub)
	{
		return (DomainParticipantImpl::getInstance()->removePublisher( p, pub));
	}
	/**
	 * Remove a subscriber from a participant.
	 */
	static bool removeSubscriber(Participant* p,Subscriber* sub)
	{
		return (DomainParticipantImpl::getInstance()->removeSubscriber( p, sub));
	}

	/**
	 * Register a type in the domain.
	 * The name must be unique, the size > 0.
	 * @param[in] type Pointer to the Data Type Object.
	 * @return True if correct.
	 */
	static bool registerType(DDSTopicDataType* type)
	{
		return (DomainParticipantImpl::getInstance()->registerType( type));
	}

	/**
	 * Get a pointer to a registered type.
	 * @param[in] type_name Name of the type to add.
	 * @param[out] type_ptr Pointer to pointer of the type, is used to return the object.
	 * @return True if the type is found.
	 */
	static bool getRegisteredType(std::string type_name,DDSTopicDataType** type_ptr)
	{
		return (DomainParticipantImpl::getInstance()->getRegisteredType(type_name,type_ptr));
	}
};




} /* namespace dds */
} /* namespace eprosima */

#endif /* DOMAINPARTICIPANT_H_ */
