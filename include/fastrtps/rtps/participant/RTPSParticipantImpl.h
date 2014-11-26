/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file RTPSParticipant.h
 */

#ifndef RTPSParticipantIMPL_H_
#define RTPSParticipantIMPL_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif


namespace boost
{
namespace interprocess{class interprocess_semaphore;}
namespace asio{class io_service;}
class recursive_mutex;
}

#include "fastrtps/rtps/attributes/RTPSParticipantAttributes.h"
#include "fastrtps/rtps/common/Guid.h"

namespace eprosima {
namespace fastrtps{

	class WriterQos;
	class ReaderQos;
	class TopicAttributes;

namespace rtps {

class RTPSParticipant;
class RTPSParticipantListener;


class ListenResource;
class ResourceSend;
class ResourceEvent;

class BuiltinProtocols;
struct CDRMessage_t;

class Endpoint;


class RTPSWriter;
class WriterAttributes;
class WriterHistory;
class WriterListener;

class RTPSReader;
class ReaderAttributes;
class ReaderHistory;
class ReaderListener;





/**
 * @brief Class RTPSParticipantImpl, it contains the private implementation of the RTPSParticipant functions and allows the creation and removal of writers and readers. It manages the send and receive threads.
 * @ingroup MANAGEMENTMODULE
 */
class RTPSParticipantImpl
{
public:
	RTPSParticipantImpl(const RTPSParticipantAttributes &param,
			const GuidPrefix_t& guidP,RTPSParticipant* part,RTPSParticipantListener* plisten= nullptr);
	virtual ~RTPSParticipantImpl();

	inline const GUID_t& getGuid() const {return m_guid;};

	//! Announce RTPSParticipantState (force the sending of a DPD message.)
	void announceRTPSParticipantState();
	//!Stop the RTPSParticipant Announcement (used in tests to avoid multiple packets being send)
	void stopRTPSParticipantAnnouncement();
	//!Reset to timer to make periodic RTPSParticipant Announcements.
	void resetRTPSParticipantAnnouncement();
	/**
	 * Activate a Remote Endpoint defined in the Static Discovery.
	 * @param pguid GUID_t of the endpoint.
	 * @param userDefinedId userDeinfed Id of the endpoint.
	 * @param kind kind of endpoint
	 * @return True if correct.
	 */
	bool newRemoteEndpointDiscovered(const GUID_t& pguid, int16_t userDefinedId,EndpointKind_t kind);
    //!Get the RTPSParticipant ID
    inline uint32_t getRTPSParticipantID() const { return (uint32_t)m_att.participantID;};
    //!Wait for the resource semaphore
    void ResourceSemaphorePost();
    //!Post to the resource semaphore
    void ResourceSemaphoreWait();
    //!Get Pointer to the IO Service.
    boost::asio::io_service* getIOService();
    //!Send Method
    void sendSync(CDRMessage_t* msg, const Locator_t& loc);
    //!Get Send Mutex
    boost::recursive_mutex* getSendMutex();

    inline RTPSParticipantListener* getListener(){return mp_participantListener;}

    inline RTPSParticipant* getUserRTPSParticipant(){return mp_userParticipant;};
private:
	//!Attributes of the RTPSParticipant
	RTPSParticipantAttributes m_att;
	//!Guid of the RTPSParticipant.
	const GUID_t m_guid;
	//! Sending resources.
	ResourceSend* mp_send_thr;
	//! Event Resource
	ResourceEvent* mp_event_thr;
	//! BuiltinProtocols of this RTPSParticipant
	BuiltinProtocols* mp_builtinProtocols;
	//!Semaphore to wait for the listen thread creation.
	boost::interprocess::interprocess_semaphore* mp_ResourceSemaphore;
	//!Id counter to correctly assign the ids to writers and readers.
	uint32_t IdCounter;
	//!Writer List.
	std::vector<RTPSWriter*> m_allWriterList;
	//!Reader List
	std::vector<RTPSReader*> m_allReaderList;
	//!Listen thread list.
	//!Writer List.
	std::vector<RTPSWriter*> m_userWriterList;
	//!Reader List
	std::vector<RTPSReader*> m_userReaderList;
	//!Listen Resource list
	std::vector<ListenResource*> m_listenResourceList;
	//!Participant Listener
	RTPSParticipantListener* mp_participantListener;
	//!Pointer to the user participant
	RTPSParticipant* mp_userParticipant;

	/**
	 * Method to check if a specific entityId already exists in this RTPSParticipant
	 * @param ent EnityId to check
	 * @param kind Endpoint Kind.
	 * @return True if exists.
	 */
	bool existsEntityId(const EntityId_t& ent,EndpointKind_t kind) const;
	/**
	 * Assign an endpoint to the listenResources.
	 * @param endp Pointer to the endpoint.
	 * @param isBuiltin Boolean indicating if it is builtin.
	 * @return True if correct.
	 */
	bool assignEndpointListenResources(Endpoint* endp,bool isBuiltin);

	/** Assign an endpoint to a specific listen locator
	 * @param pend Pointer to the endpoint.
	 * @param lit Locator list iterator.
	 * @param isMulticast Boolean indicating that is multicast.
	 * @param isFixed Boolean indicating that is a fixed listenresource.
	 * @return True if assigned.
	 */
	bool assignEndpoint2Locator(Endpoint* pend,LocatorListIterator lit,bool isMulticast,bool isFixed);


public:
	/**
	 * Create a Writer in this RTPSParticipant.
	 * @param Writer Pointer to pointer of the Writer, used as output. Only valid if return==true.
	 * @param param WriterAttributes to define the Writer.
	 * @param entityId EntityId assigned to the Writer.
	 *  * @param isBuiltin Bool value indicating if the Writer is builtin (Discovery or Liveliness protocol) or is created for the end user.
	 * @return True if the Writer was correctly created.
	 */
	bool createWriter(RTPSWriter** Writer, WriterAttributes& param,WriterHistory* hist,WriterListener* listen,
			const EntityId_t& entityId = c_EntityId_Unknown,bool isBuiltin = false);

	bool createReader(RTPSReader** Reader, ReaderAttributes& param,ReaderHistory* hist,ReaderListener* listen,
				const EntityId_t& entityId = c_EntityId_Unknown,bool isBuiltin = false);


	bool registerWriter(RTPSWriter* Writer,TopicAttributes& topicAtt,WriterQos& wqos);

	bool registerReader(RTPSReader* Reader,TopicAttributes& topicAtt,ReaderQos& wqos);

	inline uint32_t getParticipantID() {return (uint32_t)this->m_att.participantID;};

	inline RTPSParticipantAttributes& getAttributes() {return m_att;};

	bool deleteUserEndpoint(Endpoint*);

	std::vector<RTPSReader*>::iterator userReadersListBegin(){return m_userReaderList.begin();};

	std::vector<RTPSReader*>::iterator userReadersListEnd(){return m_userReaderList.end();};

	std::vector<RTPSWriter*>::iterator userWritersListBegin(){return m_userWriterList.begin();};

	std::vector<RTPSWriter*>::iterator userWritersListEnd(){return m_userWriterList.end();};

	//
	//	/**
	//	 * Create a Reader in this RTPSParticipant.
	//	 * @param Reader Pointer to pointer of the Reader, used as output. Only valid if return==true.
	//	 * @param RParam SubscriberAttributes to define the Reader.
	//	 * @param payload_size Maximum payload size.
	//	 * @param isBuiltin Bool value indicating if the Reader is builtin (Discovery or Liveliness protocol) or is created for the end user.
	//	 * @param kind STATEFUL or STATELESS.
	//	 * @param ptype Pointer to the TOpicDataType object (optional).
	//	 * @param slisten Pointer to the SubscriberListener object (optional).
	//	 * @param entityId EntityId assigned to the Reader.
	//	 * @return True if the Reader was correctly created.
	//	 */
	//	bool createReader(RTPSReader** Reader,SubscriberAttributes& RParam,uint32_t payload_size,bool isBuiltin,StateKind_t kind,
	//			TopicDataType* ptype = NULL,SubscriberListener* slisten=NULL,const EntityId_t& entityId = c_EntityId_Unknown);
	//
	//	/**

	//	/**
	//	 * Register a reader in the builtin protocols.
	//	 * @param Reader Pointer to the RTPSReader to register.
	//	 */
	//	void registerReader(RTPSReader* Reader);

	//	/**

	//
	//
	//	/**
	//	 * Remove Endpoint from the RTPSParticipant. It closes all entities related to them that are no longer in use.
	//	 * For example, if a ResourceListen is not useful anymore the thread is closed and the instance removed.
	//	 * @param[in] p_endpoint Pointer to the Endpoint that is going to be removed.
	//	 * @param[in] type Char indicating if it is Reader ('R') or Writer ('W')
	//	 * @return True if correct.
	//	 */
	//	bool deleteUserEndpoint(Endpoint* p_endpoint,char type);
	//
	//

	//
	//
	//	//!Used for tests
	//	void loose_next_change(){m_send_thr.loose_next();};
	//	//! Announce RTPSParticipantState (force the sending of a DPD message.)
	//	void announceRTPSParticipantState();
	//	//!Stop the RTPSParticipant Announcement (used in tests to avoid multiple packets being send)
	//	void stopRTPSParticipantAnnouncement();
	//	//!Reset to timer to make periodic RTPSParticipant Announcements.
	//	void resetRTPSParticipantAnnouncement();
	//	/**
	//	 * Get the GUID_t of the RTPSParticipant.
	//	 * @return GUID_t of the RTPSParticipant.
	//	 */
	//	const GUID_t& getGuid() const {
	//		return m_guid;
	//	}
	//	/**
	//	 * Get the RTPSParticipant Name.
	//	 * @return String with the RTPSParticipant Name.
	//	 */
	//	const std::string& getRTPSParticipantName() const {
	//		return m_RTPSParticipantName;
	//	}
	//
	//
	//
	//	void ResourceSemaphorePost();
	//
	//	void ResourceSemaphoreWait();
	//
	//	const BuiltinAttributes& getBuiltinAttributes() const {
	//		return m_builtin;
	//	}
	//
	//	ResourceEvent* getEventResource()
	//	{
	//		return &m_event_thr;
	//	}
	//
	//	uint32_t getRTPSParticipantId() const {
	//		return m_RTPSParticipantID;
	//	}
	//
	//	void setRTPSParticipantId(uint32_t RTPSParticipantId) {
	//		m_RTPSParticipantID = RTPSParticipantId;
	//	}
	//
	//	uint32_t getListenSocketBufferSize() const {
	//		return m_listen_socket_buffer_size;
	//	}
	//
	//	uint32_t getSendSocketBufferSize() const {
	//		return m_send_socket_buffer_size;
	//	}
	//
	//	BuiltinProtocols* getBuiltinProtocols(){return &m_builtinProtocols;}
	//
	//	bool existsEntityId(const EntityId_t& ent,EndpointKind_t kind) const;
	//
	//	bool newRemoteEndpointDiscovered(const GUID_t& pguid, int16_t userDefinedId,EndpointKind_t kind);
	//
	//	void setListener(RTPSParticipantListener* lis) {mp_RTPSParticipantListener = lis;}
	//
	//	RTPSParticipantListener* getListener() const {return mp_RTPSParticipantListener;}
	//
	//	RTPSParticipant* getUserRTPSParticipant() const {return mp_userRTPSParticipant;}
	//
	//	std::vector<octet> getUserData() const {return m_userData;}
	//
	//	uint32_t getRTPSParticipantID() const{return m_RTPSParticipantID;}
	//private:
	//
	//
	//
	//
	//	/*!
	//	 * Assign a given Endpoint to one of the current listen thread or create a new one.
	//	 * @param[in] endpoint Pointer to the Endpoint to add.
	//	 * @param[in] type Type of the Endpoint (R or W)(Reader or Writer).
	//	 * @param[in] isBuiltin Indicates if the endpoint is Builtin or not.
	//	 * @return True if correct.
	//	 */
	//	bool assignEnpointToListenResources(Endpoint* endpoint,char type,bool isBuiltin);
	//	//	/*!
	//	//	 * Create a new listen thread in the specified locator.
	//	//	 * @param[in] loc Locator to use.
	//	//	 * @param[out] listenthread Pointer to pointer of this class to correctly initialize the listening recourse.
	//	//	 * @param[in] isMulticast To indicate whether the new lsited thread is multicast.
	//	//	 * @param[in] isBuiltin Indicates that the endpoint is builtin.
	//	//	 * @return True if correct.
	//	//	 */
	//	//	bool addNewListenResource(Locator_t& loc,ResourceListen** listenthread,bool isMulticast,bool isBuiltin);
	//
	//	//RTPSParticipantDiscoveryProtocol* mp_PDP;
	//
	//
	//
	//
	//
	//
	//	RTPSParticipantListener* mp_RTPSParticipantListener;
	//




};

}
} /* namespace rtps */
} /* namespace eprosima */

#endif /* RTPSParticipant_H_ */





