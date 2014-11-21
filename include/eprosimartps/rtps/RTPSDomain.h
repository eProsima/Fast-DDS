/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file DomainRTPSParticipant.h 	
 */



#ifndef RTPSRTPSParticipant_H_
#define RTPSRTPSParticipant_H_

#include "eprosimartps/rtps/common/Types.h"

#include "eprosimartps/rtps/attributes/RTPSParticipantAttributes.h"

#include <set>


namespace eprosima{

namespace rtps{

class RTPSParticipantImpl;
class RTPSParticipant;
class RTPSParticipantListener;
class RTPSWriter;
class WriterAttributes;
class WriterHistory;
class WriterListener;


/**
 * Class DomainRTPSParticipantImpl, singleton that performs all operations permitted by the DomainRTPSParticipant class. It also stores information
 * regarding all RTPSParticipants created in this Domain.
 */
class RTPSDomain
{
	typedef std::pair<RTPSParticipant*,RTPSParticipantImpl*> t_p_RTPSParticipant;
private:
	RTPSDomain();
    /**
     * DomainRTPSParticipant destructor
     */
    ~RTPSDomain();

public:
    /**
     * Method to shut down all RTPSParticipants, readers, writers, etc.
     * It must be called at the end of the process to avoid memory leaks.
     * It also shut downs the DomainRTPSParticipant.
     */
    static void stopAll();

    /**
     * @brief Create a RTPSParticipant.
     * @snippet pubsub_example.cpp ex_RTPSParticipantCreation
     * @param PParam RTPSParticipant Parameters.
     * @return Pointer to the RTPSParticipant.
     */
    static RTPSParticipant* createParticipant(RTPSParticipantAttributes& PParam,RTPSParticipantListener* plisten = nullptr);

    /**
     *
     * @param watt
     * @param hist
     * @param listen
     * @return
     */
    static RTPSWriter* createRTPSWriter(RTPSParticipant* p, WriterAttributes& watt, WriterHistory* hist, WriterListener* listen = nullptr);


    static bool removeRTPSWriter(RTPSWriter* writer);

    /**
     * Remove a RTPSParticipant and delete all its associated Writers, Readers, resources, etc.
     * @param[in] p Pointer to the RTPSParticipant;
     * @return True if correct.
     */
    static bool removeRTPSParticipant(RTPSParticipant* p);

	/**
	 * @brief Get pointer to the unique instance of this class.
	 * @return Pointer to the instance.
	 */
    static RTPSDomain* getInstance();



   	static inline void setMaxRTPSParticipantId(uint32_t maxRTPSParticipantId) {
		m_maxRTPSParticipantID = maxRTPSParticipantId;
	}

//	static inline uint32_t getMulticastPort(uint32_t domainId)
//	{
//		return getPortBase()+ getDomainIdGain() * domainId+ getOffsetd0();
//	}
//	static inline uint32_t getUnicastPort(uint32_t domainId,uint32_t RTPSParticipantID)
//	{
//		return getPortBase()+ getDomainIdGain() * domainId	+ getOffsetd1()	+ getRTPSParticipantIdGain() * RTPSParticipantID;
//	}

private:
	static uint32_t m_maxRTPSParticipantID;
	static bool instanceFlag;
	static RTPSDomain *single;

	static std::vector<t_p_RTPSParticipant> m_RTPSParticipants;

	/**
	 * @brief Get Id to create a RTPSParticipant.
	 * @return Different ID for each call.
	 */
	static inline uint32_t getNewId() {return ++m_maxRTPSParticipantID;};

	static std::set<uint32_t> m_RTPSParticipantIDs;

};






} /* namespace pubsub */
} /* namespace eprosima */

#endif /* DOMAINRTPSParticipant_H_ */
