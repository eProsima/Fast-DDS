/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ResourceSendImpl.h
 */



#ifndef RESOURCESENDIMPL_H_
#define RESOURCESENDIMPL_H_
#include <boost/asio.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/thread.hpp>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "eprosimartps/common/types/Locator.h"

namespace eprosima {
namespace rtps {

struct CDRMessage_t;
class ParticipantImpl;

/**
 * Class ResourceSend, used to manage the send operation. In future version it will contain the grouping
 *  logic for merge different CDRMessages into a single RTPSMessages (HB piggybacking, for example).
 * @ingroup MANAGEMENTMODULE
 */
class ResourceSendImpl
{
public:
	ResourceSendImpl();
	virtual ~ResourceSendImpl();
	/**
	 * Send a CDR message syncrhonously. No waiting is required.
	 * @param msg Pointer to the message.
	 * @param loc Locator where to send the message.
	 */
	void sendSync(CDRMessage_t* msg,const Locator_t& loc);

	/**
	 * Initialize the sending socket.
	 * @param loc Locator of hte address from where to start the sending socket.
	 * @return True if correct
	 */
	bool initSend(ParticipantImpl*,const Locator_t& loc,bool useIP4, bool useIP6);

	//!FOR TESTING ONLY!!!!
	void loose_next(){m_send_next = false;};
	boost::recursive_mutex* getMutex();
private:
	bool m_useIP4;
	bool m_useIP6;
	Locator_t m_sendLocator_v4;
	Locator_t m_sendLocator_v6;
	boost::asio::io_service m_send_service;
	boost::asio::ip::udp::socket m_send_socket_v4;
	boost::asio::ip::udp::socket m_send_socket_v6;
	boost::asio::ip::udp::endpoint m_send_endpoint_v4;
	boost::asio::ip::udp::endpoint m_send_endpoint_v6;
	size_t m_bytes_sent;
	bool m_send_next;
	ParticipantImpl* mp_participant;
	boost::recursive_mutex* mp_mutex;

};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* RESOURCESEND_H_ */
