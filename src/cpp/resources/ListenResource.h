/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ListenResource.h
 *
 *  Created on: May 28, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
 */

#ifndef LISTENRESOURCE_H_
#define LISTENRESOURCE_H_

#include "eprosimartps/common/types/Locator.h"
#include "eprosimartps/MessageReceiver.h"

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>
#include <boost/asio/ip/udp.hpp>

namespace eprosima {
namespace rtps {

class ParticipantImpl;
class RTPSWriter;
class RTPSReader;
class Endpoint;

class ListenResource {
	friend class MessageReceiver;
public:
	ListenResource(ParticipantImpl* p);
	virtual ~ListenResource();

	const Locator_t& init_thread(Locator_t& loc,bool isMulti,bool isFixed);

	bool addAssociatedEndpoint(Endpoint* end);
	bool removeAssociatedEndpoint(Endpoint* end);
	bool isListeningTo(const Locator_t& loc);

private:
	ParticipantImpl* mp_participantImpl;
	boost::thread* mp_thread;
	boost::asio::io_service m_io_service;
	boost::asio::ip::udp::socket m_listen_socket;
	boost::asio::ip::udp::endpoint m_sender_endpoint;

	Locator_t m_listenLoc;
	Locator_t m_senderLocator;

	std::vector<RTPSWriter*> m_assocWriters;
	std::vector<RTPSReader*> m_assocReaders;

	MessageReceiver m_MessageReceiver;

	/**
		 * Callback to be executed when a new Message is received in the socket.
		 * @param error Error code associated with the operation.
		 * @param size NUmber of bytes received
		 */
		void newCDRMessage(const boost::system::error_code& error, std::size_t size);

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* LISTENRESOURCE_H_ */
