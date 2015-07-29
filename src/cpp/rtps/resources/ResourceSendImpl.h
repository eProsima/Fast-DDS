/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ResourceSendImpl.h
 */



#ifndef RESOURCESENDIMPL_H_
#define RESOURCESENDIMPL_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <boost/asio.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/thread.hpp>
#include <boost/thread/lockable_adapter.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <fastrtps/rtps/common/Locator.h>

namespace eprosima {
namespace fastrtps{
namespace rtps {

struct CDRMessage_t;
class RTPSParticipantImpl;

/**
 * Class ResourceSend, used to manage the send operation. In future version it will contain the grouping
 *  logic for merge different CDRMessages into a single RTPSMessages (HB piggybacking, for example).
 *@ingroup MANAGEMENT_MODULE
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
	*
	* @param pimpl
	* @param loc Locator of the address from where to start the sending socket.
	* @param sendsockBuffer
	* @param useIP4 Booleand telling whether to use IPv4
	* @param useIP6 Booleand telling whether to use IPv6
	* @return true on success
	*/
	bool initSend(RTPSParticipantImpl*,const Locator_t& loc,uint32_t sendseockBuffer,bool useIP4, bool useIP6);

	//!FOR TESTING ONLY!!!!
	void loose_next(){m_send_next = false;};
	
	/**
	* Get associated mutex
	* @return Associated mutex
	*/
	boost::recursive_mutex* getMutex();
private:
	bool m_useIP4;
	bool m_useIP6;
	std::vector<Locator_t> mv_sendLocator_v4;
	std::vector<Locator_t> mv_sendLocator_v6;
	boost::asio::io_service m_send_service;
	std::vector<boost::asio::ip::udp::socket*> mv_send_socket_v4;
	std::vector<boost::asio::ip::udp::socket*> mv_send_socket_v6;
	boost::asio::ip::udp::endpoint m_send_endpoint_v4;
	boost::asio::ip::udp::endpoint m_send_endpoint_v6;
	size_t m_bytes_sent;
	bool m_send_next;
	RTPSParticipantImpl* mp_RTPSParticipant;
	boost::recursive_mutex* mp_mutex;

};
}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* RESOURCESEND_H_ */
