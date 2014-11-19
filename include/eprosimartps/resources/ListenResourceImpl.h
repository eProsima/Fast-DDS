/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ListenResourceImpl.h
 *
 */

#ifndef LISTENRESOURCEIMPL_H_
#define LISTENRESOURCEIMPL _H_

#include "eprosimartps/common/types/Locator.h"


#include <boost/asio.hpp>
#include <boost/asio/ip/udp.hpp>

#include <boost/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace eprosima {
namespace rtps {

class ParticipantImpl;

/**
 * Class ListenResourceImpl, used to control the listen sockets and the received messages.
 * @ingroup MANAGEMENTMODULE
 */
class ListenResourceImpl
{
	friend class MessageReceiver;
public:
	ListenResourceImpl();
	virtual ~ListenResourceImpl();
	/**
	 * Initialize the listening thread.
	 * @param loc Locator to open the socket.
	 * @param isMulti Boolean for when is multicast.
	 * @param isFixed Boolean to indicate whether another locator can be use in case the default is already being used.
	 * @return The locator that has been opennend.
	 */
	Locator_t init_thread(ParticipantImpl* pimpl,Locator_t& loc,uint32_t listenSocketSize,bool isMulti,bool isFixed);

	//!Returns true if the ListenResourceImpl is listenning to a specific locator.
	bool isListeningTo(const Locator_t& loc);
//	//!Returns trus if the ListenResourceImpl has any associated endpoints.
//	bool hasAssociatedEndpoints(){return !(m_assocWriters.empty() && m_assocReaders.empty());};
//	//!Get the pointer to the participant
//	ParticipantImpl* getParticipantImpl(){return mp_participantImpl;};

private:
	ParticipantImpl* mp_participantImpl;
	boost::thread* mp_thread;
	boost::asio::io_service m_io_service;
	boost::asio::ip::udp::socket m_listen_socket;
	boost::asio::ip::udp::endpoint m_sender_endpoint;
	boost::asio::ip::udp::endpoint m_listen_endpoint;

	Locator_t m_listenLoc;
	Locator_t m_senderLocator;


//
//	MessageReceiver m_MessageReceiver;

	/**
	 * Callback to be executed when a new Message is received in the socket.
	 * @param error Error code associated with the operation.
	 * @param size NUmber of bytes received
	 */
	void newCDRMessage(const boost::system::error_code& error, std::size_t size);

	//! Method to run the io_service.
	void run_io_service();

	boost::recursive_mutex m_mutex;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* LISTENRESOURCE_H_ */
