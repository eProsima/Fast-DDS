/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ResourceListen.h
 *  ResourceListen class.
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *      		grcanosa@gmail.com
 */

#ifndef RESOURCELISTEN_H_
#define RESOURCELISTEN_H_


#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

#include "eprosimartps/common/types/Locator.h"
#include "eprosimartps/MessageReceiver.h"




namespace eprosima {
namespace rtps {

class RTPSReader;
class RTPSWriter;
class Endpoint;
class ParticipantImpl;

/**
 * Class ResourceListen, used to listen to a specific socket for RTPS messages. Each instance, when initialized, launches
 * a new thread that listen to a specific port (all possible IP addresses in this machine.). Multiple writers and readers can be associated
 * with the same ResourceListen. The MessageReceiver instance interprets where the messages need to be forwarded (which Writer or Reader, or both).
 * @ingroup MANAGEMENTMODULE
 */
class ResourceListen
{
	friend class MessageReceiver; // Here is justifies since each ResourceListen has only one MessageReceiver and viceversa
public:
	ResourceListen(ParticipantImpl* p, bool isMulti,bool isMetatraffic);
	virtual ~ResourceListen();

	void removeEndpointFromAssociated(Endpoint* endp);

	bool addAssociatedEndpoint(Endpoint* endp);

	bool hasAssociatedEndpoints(){return !(m_assoc_writers.empty() && m_assoc_readers.empty());};


	bool isListeningTo(const Locator_t& loc);

	/**
	 * Method to initialize the thread.
	 */
	bool init_thread(Locator_t& loc);
private:

	//! Vector of pointers to the associated writers.
	std::vector<RTPSWriter*> m_assoc_writers;
	//! Vector of pointer to the associated readers.
	std::vector<RTPSReader*> m_assoc_readers;
	//! Pointer to the participant.
	ParticipantImpl* mp_participantImpl;
	//!Vector containing the locators that are being listened with this thread. Currently the thread only listens to one Locator.
	LocatorList_t m_locList;
	//!Pointer to the thread.
	boost::thread* mp_thread;
	boost::asio::io_service m_io_service;
	//!Listen socket.
	boost::asio::ip::udp::socket m_listen_socket;
	//!MessageReceiver used in the thread.
	MessageReceiver m_MessageReceiver;
	bool m_first;
	//boost::asio::ip::udp::resolver resolver;
	Locator_t m_send_locator;
	boost::asio::ip::udp::endpoint m_sender_endpoint;
	//! Variable indicating whether the listen thread is Multicast.
	bool m_isMulticast;

	bool m_isMetatraffic;
	/**
	 * Callback to be executed when a new Message is received in the socket.
	 * @param error Error code associated with the operation.
	 * @param size NUmber of bytes received
	 */
	void newCDRMessage(const boost::system::error_code& error, std::size_t size);
	//! Method to run the io_service.
	void run_io_service();




};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RESOURCELISTEN_H_ */
