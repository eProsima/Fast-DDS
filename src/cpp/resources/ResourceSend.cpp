/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * ResourceSend.cpp
 *
 */

#include "eprosimartps/resources/ResourceSend.h"
#include "eprosimartps/common/types/CDRMessage_t.h"
#include "eprosimartps/utils/RTPSLog.h"

#include "eprosimartps/Participant.h"

using boost::asio::ip::udp;

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "SendResource";

ResourceSend::ResourceSend(ParticipantImpl* par) :
											m_send_socket_v4(m_send_service),
											m_send_socket_v6(m_send_service),
											m_bytes_sent(0),
											m_send_next(true),
											mp_participant(par)
{

}

bool ResourceSend::initSend(const Locator_t& loc)
{
	const char* const METHOD_NAME = "initSend";
	//	boost::asio::ip::address addr;
	if(loc.kind == LOCATOR_KIND_UDPv4)
	{
		m_sendLocator_v4 = loc;
		m_sendLocator_v6.port = loc.port;
	}
//	else if(loc.kind == LOCATOR_KIND_UDPv6)
//	{
//		m_sendLocator_v6 = loc;
//		m_sendLocator_v6.port = loc.port;
//	}

	//OPEN SOCKETS:
	m_send_socket_v4.open(boost::asio::ip::udp::v4());
	m_send_socket_v4.set_option(boost::asio::socket_base::send_buffer_size(this->mp_participant->getSendSocketBufferSize()));
//	m_send_socket_v6.open(boost::asio::ip::udp::v6());
//	m_send_socket_v6.set_option(boost::asio::socket_base::send_buffer_size(this->mp_participant->getSendSocketBufferSize()));
	//m_send_socket.set_option( boost::asio::ip::enable_loopback( true ) );
	//BINDING
	bool not_bind = true;
	while(not_bind)
	{
		udp::endpoint send_endpoint = udp::endpoint(boost::asio::ip::udp::v4(),m_sendLocator_v4.port);
		try{
			m_send_socket_v4.bind(send_endpoint);
			not_bind = false;
		}
		catch (boost::system::system_error const& e)
		{
			logWarning(RTPS_MSG_OUT,"UDPv4 Error binding: ("<<e.what()<< ") with socket: " << send_endpoint,EPRO_YELLOW);
			m_sendLocator_v4.port++;
		}
	}
	boost::asio::socket_base::send_buffer_size option;
	m_send_socket_v4.get_option(option);
//<<<<<<< HEAD
//
//	pInfo (RTPS_YELLOW<<"ResourceSend: initSend UDPv4: " << m_send_socket_v4.local_endpoint()<<"|| State: " << m_send_socket_v4.is_open() <<
//			" || buffer size: " <<option.value()<< RTPS_DEF<<endl);
//	not_bind = true;
//
////	m_sendLocator_v6.port = m_sendLocator_v4.port +1;
////	while(not_bind)
////	{
//	//	udp::endpoint send_endpoint = udp::endpoint(boost::asio::ip::udp::v6(),m_sendLocator_v6.port);
//	//	try{
//	//		m_send_socket_v6.bind(send_endpoint);
////			not_bind = false;
////		}
////		catch (boost::system::system_error const& e)
////		{
////			pWarning("ResourceSend: "<<e.what()<< " with socket: " << send_endpoint << endl);
////			m_sendLocator_v6.port++;
////		}
////	}
//
//	//m_send_socket_v6.get_option(option);
//	//pInfo (RTPS_YELLOW<<"ResourceSend: initSend UDPv6: " << m_send_socket_v6.local_endpoint()<<"|| State: " << m_send_socket_v6.is_open() <<
//	//		" || buffer size: " <<option.value()<< RTPS_DEF<<endl);
//
////	while(not_bind)
////	{
////		udp::endpoint send_endpoint = udp::endpoint(boost::asio::ip::udp::v6(),m_sendLocator_v6.port);
////		try{
////			m_send_socket_v6.bind(send_endpoint);
////			not_bind = false;
////		}
////		catch (boost::system::system_error const& e)
////		{
////			pWarning("ResourceSend: "<<e.what()<< " with socket: " << send_endpoint << endl);
////			m_sendLocator_v6.port++;
////		}
////	}
//	boost::asio::socket_base::send_buffer_size option2;
//	m_send_socket_v4.get_option(option2);
//
//	pInfo (RTPS_YELLOW<<"ResourceSend: initSend UDPv4: " << m_send_socket_v4.local_endpoint()<<"|| State: " << m_send_socket_v4.is_open() <<
//			" || buffer size: " <<option2.value()<< RTPS_DEF<<endl);
////	m_send_socket_v6.get_option(option);
////	pInfo (RTPS_YELLOW<<"ResourceSend: initSend UDPv6: " << m_send_socket_v6.local_endpoint()<<"|| State: " << m_send_socket_v6.is_open() <<
////			" || buffer size: " <<option.value()<< RTPS_DEF<<endl);
//
//=======
	logInfo (RTPS_MSG_OUT,"UDPv4: " << m_send_socket_v4.local_endpoint()<<"|| State: " << m_send_socket_v4.is_open() <<
				" || buffer size: " <<option.value(),EPRO_YELLOW);
	not_bind = true;
	m_sendLocator_v6.port = m_sendLocator_v4.port+1;
	while(not_bind)
	{
		udp::endpoint send_endpoint = udp::endpoint(boost::asio::ip::udp::v6(),m_sendLocator_v6.port);
		try{
			m_send_socket_v6.bind(send_endpoint);
			not_bind = false;
		}
		catch (boost::system::system_error const& e)
		{
			logWarning(RTPS_MSG_OUT,"UDPv6 Error binding: ("<<e.what()<< ") in socket: " << send_endpoint,EPRO_YELLOW);
			m_sendLocator_v6.port++;
		}
	}

	m_send_socket_v6.get_option(option);
	logInfo (RTPS_MSG_OUT,"UDPv6: " << m_send_socket_v6.local_endpoint()<<"|| State: " << m_send_socket_v6.is_open() <<
			" || buffer size: " <<option.value(),EPRO_YELLOW);
//>>>>>>> origin/feature/newLog

	//boost::asio::io_service::work work(sendService);
	return true;
}


ResourceSend::~ResourceSend()
{
	const char* const METHOD_NAME = "~SendResource";
	logInfo(RTPS_MSG_OUT,"",EPRO_YELLOW);
	m_send_socket_v4.close();
	//m_send_socket_v6.close();
	m_send_service.stop();
}

void ResourceSend::sendSync(CDRMessage_t* msg, const Locator_t& loc)
{
	const char* const METHOD_NAME = "sendSync";
	boost::lock_guard<ResourceSend> guard(*this);
	if(loc.port == 0)
		return;
	if(loc.kind == LOCATOR_KIND_UDPv4)
	{
		boost::asio::ip::address_v4::bytes_type addr;
		for(uint8_t i=0;i<4;i++)
			addr[i] = loc.address[12+i];
		m_send_endpoint_v4 = udp::endpoint(boost::asio::ip::address_v4(addr),loc.port);
		logInfo(RTPS_MSG_OUT,"UDPv4: " << msg->length << " bytes TO endpoint: " << m_send_endpoint_v4
				<< " FROM " << m_send_socket_v4.local_endpoint(),EPRO_YELLOW);
		if(m_send_endpoint_v4.port()>0)
		{
			m_bytes_sent = 0;
			if(m_send_next)
			{
				try {
					m_bytes_sent = m_send_socket_v4.send_to(boost::asio::buffer((void*)msg->buffer,msg->length),m_send_endpoint_v4);
				} catch (const std::exception& error) {
					// Should print the actual error message
					logWarning(RTPS_MSG_OUT,"Error: " <<error.what(),EPRO_YELLOW);
				}
			}
			else
			{
				m_send_next = true;
			}
			logInfo (RTPS_MSG_OUT,"SENT " << m_bytes_sent,EPRO_YELLOW);
		}
		else if(m_send_endpoint_v4.port()<=0)
		{
			logWarning(RTPS_MSG_OUT,"Port invalid",EPRO_YELLOW);
		}
		else
			logError(RTPS_MSG_OUT,"Port error",EPRO_YELLOW);
	}
//<<<<<<< HEAD
//	else if(loc.kind == LOCATOR_KIND_UDPv6)
//	{
//		boost::asio::ip::address_v6::bytes_type addr;
//		for(uint8_t i=0;i<16;i++)
//			addr[i] = loc.address[i];
//		m_send_endpoint_v6 = udp::endpoint(boost::asio::ip::address_v6(addr),loc.port);
//		pInfo(RTPS_YELLOW<< "ResourceSend: sendSync UDPv6: " << msg->length << " bytes TO endpoint: " << m_send_endpoint_v6 << " FROM " << m_send_socket_v6.local_endpoint()  << endl);
//		if(m_send_endpoint_v6.port()>0)
//		{
//			m_bytes_sent = 0;
//			if(m_send_next)
//			{
//				try {
//					m_bytes_sent = m_send_socket_v6.send_to(boost::asio::buffer((void*)msg->buffer,msg->length),m_send_endpoint_v6);
//				} catch (const std::exception& error) {
//					// Should print the actual error message
//					pWarning(error.what() << std::endl);
//				}
//
//			}
//			else
//			{
//				m_send_next = true;
//			}
//			pInfo (RTPS_YELLOW <<  "SENT " << m_bytes_sent << RTPS_DEF << endl);
//		}
//		else if(m_send_endpoint_v6.port()<=0)
//		{
//			pWarning("ResourceSend: sendSync: port invalid"<<endl);
//		}
//		else
//			pError("ResourceSend: sendSync: port error"<<endl);
	//}
//=======
//	else if(loc.kind == LOCATOR_KIND_UDPv6)
//	{
//		boost::asio::ip::address_v6::bytes_type addr;
//		for(uint8_t i=0;i<16;i++)
//			addr[i] = loc.address[i];
//		m_send_endpoint_v6 = udp::endpoint(boost::asio::ip::address_v6(addr),loc.port);
//		logInfo(RTPS_MSG_OUT,"UDPv6: " << msg->length << " bytes TO endpoint: "
//				<< m_send_endpoint_v6 << " FROM " << m_send_socket_v6.local_endpoint(),EPRO_YELLOW);
//		if(m_send_endpoint_v6.port()>0)
//		{
//			m_bytes_sent = 0;
//			if(m_send_next)
//			{
//				try {
//					m_bytes_sent = m_send_socket_v6.send_to(boost::asio::buffer((void*)msg->buffer,msg->length),m_send_endpoint_v6);
//				} catch (const std::exception& error) {
//					// Should print the actual error message
//					logWarning(RTPS_MSG_OUT,"Error: " <<error.what(),EPRO_YELLOW);
//				}
//
//			}
//			else
//			{
//				m_send_next = true;
//			}
//			logInfo (RTPS_MSG_OUT,"SENT " << m_bytes_sent,EPRO_YELLOW);
//		}
//		else if(m_send_endpoint_v6.port()<=0)
//		{
//			logWarning(RTPS_MSG_OUT,"Port invalid",EPRO_YELLOW);
//		}
//		else
//			logError(RTPS_MSG_OUT,"Port error",EPRO_YELLOW);
//	}
//>>>>>>> origin/feature/newLog

}


} /* namespace rtps */
} /* namespace eprosima */
