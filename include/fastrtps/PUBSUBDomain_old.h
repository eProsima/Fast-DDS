/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DomainRTPSParticipant.h 	
 */



#ifndef RTPSRTPSParticipant_H_
#define RTPSRTPSParticipant_H_

#include "fastrtps/rtps/common/Types.h"
#include "fastrtps/rtps/attributes/RTPSParticipantAttributes.h"
#include <set>


namespace eprosima{

namespace rtps{
class RTPSParticipant;
class RTPSParticipantImpl;

typedef std::pair<RTPSParticipant*,RTPSParticipantImpl*> RTPSParticipantPair;

}


using namespace rtps;

namespace pubsub
{

class TopicDataType;
class Publisher;
class Subscriber;
class Publisher;
class PublisherImpl;
class Subscriber;
class SubscriberImpl;
class PublisherListener;
class SubscriberListener;
class RTPSParticipantListener;

typedef std::pair<Subscriber*,SubscriberImpl*> SubscriberPair;
typedef std::pair<Publisher*,PublisherImpl*> PublisherPair;

/**
 * Class DomainRTPSParticipantImpl, singleton that performs all operations permitted by the DomainRTPSParticipant class. It also stores information
 * regarding all RTPSParticipants created in this Domain.
 */
class RTPSDomainImpl
{
private:
	RTPSDomainImpl();
    /**
     * DomainRTPSParticipant destructor
     */
    ~RTPSDomainImpl();

    bool getRTPSParticipantImpl(RTPSParticipant*,RTPSParticipantImpl**);

public:
    /**
     * Method to shut down all RTPSParticipants, readers, writers, etc.
     * It must be called at the end of the process to avoid memory leaks.
     * It also shut downs the DomainRTPSParticipant.
     */
    void stopAll();
	/**
	 * @brief Create a Publisher in the given RTPSParticipant. 
	 * @param p Pointer to the RTPSParticipant.
	 * @param WParam Writer Parameters to create a Publisher.
	 * @param plisten Pointer to the PublisherListener (optional).
	 * @return Pointer to the publisher. 
	 */
    Publisher* createPublisher(RTPSParticipant* p, PublisherAttributes& WParam,PublisherListener* plisten=NULL);
	/**
	 * @brief Create a Subscriber in the given RTPSParticipant. 
	 * @param p Pointer to the RTPSParticipant.
	 * @param RParam Reader Parameters to create a Publisher.
	 * @param slisten Pointer to the SubscriberListener (optional).
	 * @return Pointer to the subscriber. 
	 */
    Subscriber* createSubscriber(RTPSParticipant* p, SubscriberAttributes& RParam,SubscriberListener* slisten=NULL);

    /**
     * @brief Create a RTPSParticipant.
     * @snippet pubsub_example.cpp ex_RTPSParticipantCreation
     * @param PParam RTPSParticipant Parameters.
     * @return Pointer to the RTPSParticipant.
     */
    RTPSParticipant* createRTPSParticipant(const RTPSParticipantAttributes& PParam,RTPSParticipantListener* plisten = NULL);

    /**
     * Remove a RTPSParticipant and delete all its associated Writers, Readers, resources, etc.
     * @param[in] p Pointer to the RTPSParticipant;
     * @return True if correct.
     */
    bool removeRTPSParticipant(RTPSParticipant* p);
    /**
     * Remove a publisher from the RTPSParticipant.
     */
    bool removePublisher(RTPSParticipant* p,Publisher* pub);
    /**
     * Remove a subscriber from a RTPSParticipant.
     */
    bool removeSubscriber(RTPSParticipant* p,Subscriber* sub);

    /**
     * Register a type in the domain.
     * The name must be unique, the size > 0.
     * @param[in] type Pointer to the Data Type Object.
     * @return True if correct.
     */
    bool registerType(TopicDataType* type);

    /**
     * Get a pointer to a registered type.
     * @param[in] type_name Name of the type to add.
     * @param[out] type_ptr Pointer to pointer of the type, is used to return the object.
     * @return True if the type is found.
     */
    bool getRegisteredType(std::string type_name,TopicDataType** type_ptr);


	/**
	 * @brief Get pointer to the unique instance of this class.
	 * @return Pointer to the instance.
	 */
    static RTPSDomainImpl* getInstance();



    /**
     * Set the parameters used to calculate the default ports in the discovery.
     */
    void setPortParameters(uint16_t PB,uint16_t DG,uint16_t PG,uint16_t d0,uint16_t d1,uint16_t d2,uint16_t d3)
    {
    	m_portBase = PB;
    	m_domainIdGain = DG;
    	m_RTPSParticipantIdGain = PG;
    	m_offsetd0 = d0;
    	m_offsetd1 = d1;
    	m_offsetd2 = d2;
    	m_offsetd3 = d3;
    }
    /** @name Methods to get the default Port numbers.
     */

    	///@{
	uint16_t getDomainIdGain() const {		return m_domainIdGain;	}

	uint16_t getOffsetd0() const {		return m_offsetd0;	}

	uint16_t getOffsetd1() const {		return m_offsetd1;	}

	uint16_t getOffsetd2() const {		return m_offsetd2;	}

	uint16_t getOffsetd3() const {		return m_offsetd3;	}

	uint16_t getRTPSParticipantIdGain() const {		return m_RTPSParticipantIdGain;	}

	uint16_t getPortBase() const {		return m_portBase;	}

	void setMaxRTPSParticipantId(uint32_t maxRTPSParticipantId) {
		m_maxRTPSParticipantID = maxRTPSParticipantId;
	}

	///@}

	uint32_t getMulticastPort(uint32_t domainId)
	{
		return getPortBase()+ getDomainIdGain() * domainId+ getOffsetd0();
	}
	uint32_t getUnicastPort(uint32_t domainId,uint32_t RTPSParticipantID)
	{
		return getPortBase()+ getDomainIdGain() * domainId	+ getOffsetd1()	+ getRTPSParticipantIdGain() * RTPSParticipantID;
	}


private:
	uint32_t m_maxRTPSParticipantID;
	static bool instanceFlag;
	static RTPSDomainImpl *single;
	std::vector<TopicDataType*> m_registeredTypes;

	uint16_t m_portBase;
	uint16_t m_domainIdGain;
	uint16_t m_RTPSParticipantIdGain;
	uint16_t m_offsetd0;
	uint16_t m_offsetd1;
	uint16_t m_offsetd2;
	uint16_t m_offsetd3;

	std::vector<PublisherPair> m_publisherList;
	std::vector<SubscriberPair> m_subscriberList;
	std::vector<RTPSParticipantPair> m_RTPSParticipants;

	uint16_t m_DomainId;

	/**
	 * @brief Get Id to create a RTPSParticipant.
	 * @return Different ID for each call.
	 */
	uint32_t getNewId();

	std::set<uint32_t> m_RTPSParticipantIDs;

};



/**
 * Class DomainRTPSParticipant, contains the static functions to create Publishers and Subscribers, as well as to register types.
 * It can be directly accessed by the user, though only by static methods.
  * @ingroup PUBSUBMODULE
 */
class RTPS_DllAPI RTPSDomain
{
public:
	/**
	 * Method to shut down all RTPSParticipants, readers, writers, etc.
	 * It must be called at the end of the process to avoid memory leaks.
	 * It also shut downs the DomainRTPSParticipant.
	 */
	static void stopAll()
	{
		RTPSDomainImpl::getInstance()->stopAll();
	}
	/**
	 * @brief Create a Publisher in the given RTPSParticipant.
	 * @param p Pointer to the RTPSParticipant.
	 * @param WParam Writer Parameters to create a Publisher.
	 * @param plisten Pointer to the PublisherListener (optional).
	 * @return Pointer to the publisher.
	 */
	static Publisher* createPublisher(RTPSParticipant* p, PublisherAttributes& WParam,PublisherListener* plisten=NULL)
	{
		return (RTPSDomainImpl::getInstance()->createPublisher(p,WParam,plisten));
	}
	/**
	 * @brief Create a Subscriber in the given RTPSParticipant.
	 * @param p Pointer to the RTPSParticipant.
	 * @param RParam Reader Parameters to create a Publisher.
	 * @param slisten Pointer to the SubscriberListener (optional).
	 * @return Pointer to the subscriber.
	 */
	static Subscriber* createSubscriber(RTPSParticipant* p, SubscriberAttributes& RParam,SubscriberListener* slisten=NULL)
	{
		return (RTPSDomainImpl::getInstance()->createSubscriber(p,RParam,slisten));
	}
	/**
	 * @brief Create a RTPSParticipant.
	 * @snippet pubsub_example.cpp ex_RTPSParticipantCreation
	 * @param PParam RTPSParticipant Parameters.
	 * @return Pointer to the RTPSParticipant.
	 */
	static RTPSParticipant* createRTPSParticipant(const RTPSParticipantAttributes& PParam,RTPSParticipantListener* plisten = NULL)
	{
		return (RTPSDomainImpl::getInstance()->createRTPSParticipant(PParam,plisten));
	}
	/**
	 * Remove a RTPSParticipant and delete all its associated Writers, Readers, resources, etc.
	 * @param[in] p Pointer to the RTPSParticipant;
	 * @return True if correct.
	 */
	static bool removeRTPSParticipant(RTPSParticipant* p)
	{
		return (RTPSDomainImpl::getInstance()->removeRTPSParticipant( p));
	}
	/**
	 * Remove a publisher from the RTPSParticipant.
	 */
	static bool removePublisher(RTPSParticipant* p,Publisher* pub)
	{
		return (RTPSDomainImpl::getInstance()->removePublisher( p, pub));
	}
	/**
	 * Remove a subscriber from a RTPSParticipant.
	 */
	static bool removeSubscriber(RTPSParticipant* p,Subscriber* sub)
	{
		return (RTPSDomainImpl::getInstance()->removeSubscriber( p, sub));
	}

	/**
	 * Register a type in the domain.
	 * The name must be unique, the size > 0.
	 * @param[in] type Pointer to the Data Type Object.
	 * @return True if correct.
	 */
	static bool registerType(TopicDataType* type)
	{
		return (RTPSDomainImpl::getInstance()->registerType( type));
	}

	/**
	 * Get a pointer to a registered type.
	 * @param[in] type_name Name of the type to add.
	 * @param[out] type_ptr Pointer to pointer of the type, is used to return the object.
	 * @return True if the type is found.
	 */
	static bool getRegisteredType(std::string type_name,TopicDataType** type_ptr)
	{
		return (RTPSDomainImpl::getInstance()->getRegisteredType(type_name,type_ptr));
	}

	  /**
     * Set the parameters used to calculate the default ports in the discovery.
     */
    static void setPortParameters(uint16_t PB,uint16_t DG,uint16_t PG,uint16_t d0,uint16_t d1,uint16_t d2,uint16_t d3)
    {
    	return (RTPSDomainImpl::getInstance()->setPortParameters( PB, DG, PG, d0, d1, d2, d3));
    }
    /** @name Methods to get the default Port numbers.
     */

    	///@{
	static uint16_t getDomainIdGain()  {		return (RTPSDomainImpl::getInstance()->getDomainIdGain());	}

	static uint16_t getOffsetd0()  {		return (RTPSDomainImpl::getInstance()-> getOffsetd0());	}

	static uint16_t getOffsetd1()  {		return (RTPSDomainImpl::getInstance()->getOffsetd1());	}

	static uint16_t getOffsetd2() {		return (RTPSDomainImpl::getInstance()->getOffsetd2());	}

	static uint16_t getOffsetd3()  {		return (RTPSDomainImpl::getInstance()->getOffsetd3());	}

	static uint16_t getRTPSParticipantIdGain()  {		return (RTPSDomainImpl::getInstance()->getRTPSParticipantIdGain());	}

	static uint16_t getPortBase()  {		return (RTPSDomainImpl::getInstance()->getPortBase());	}

//	static void setPortBase(uint16_t pb)  {return Doma}

private:
	RTPSDomainImpl* mp_impl;
};




} /* namespace pubsub */
} /* namespace eprosima */

#endif /* DOMAINRTPSParticipant_H_ */
