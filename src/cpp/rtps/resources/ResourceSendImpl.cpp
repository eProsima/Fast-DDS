/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * ResourceSendImpl.cpp
 *
 */

#include "ResourceSendImpl.h"
#include <fastrtps/rtps/common/CDRMessage_t.h>
#include <fastrtps/utils/RTPSLog.h>

#include <fastrtps/utils/IPFinder.h>

using boost::asio::ip::udp;

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "SendResource";

static const int MAX_BIND_TRIES = 100;

ResourceSendImpl::ResourceSendImpl() :
										m_useIP4(true),
										m_useIP6(true),
										//m_send_socket_v4(m_send_service),
										//m_send_socket_v6(m_send_service),
										m_bytes_sent(0),
										m_send_next(true),
										mp_RTPSParticipant(nullptr),
										mp_mutex(new boost::recursive_mutex())
{

}

bool ResourceSendImpl::initSend(RTPSParticipantImpl* /*pimpl*/, const Locator_t& loc, uint32_t sendsockBuffer, bool useIP4, bool useIP6)
{
	const char* const METHOD_NAME = "initSend";
	m_useIP4 = useIP4;
	m_useIP6 = useIP6;

	std::vector<IPFinder::info_IP> locNames;
	IPFinder::getIPs(&locNames);
	boost::asio::socket_base::send_buffer_size option;
	bool not_bind = true;
	bool initialized = false;
	int bind_tries = 0;
	for (auto ipit = locNames.begin(); ipit != locNames.end(); ++ipit)
	{
		if (ipit->type == IPFinder::IP4 && m_useIP4)
		{
			mv_sendLocator_v4.push_back(loc);
			auto sendLocv4 = mv_sendLocator_v4.back();
			sendLocv4.port = loc.port;
			//OPEN SOCKETS:
			mv_send_socket_v4.push_back(new boost::asio::ip::udp::socket(m_send_service));
			auto sendSocketv4 = mv_send_socket_v4.back();
			sendSocketv4->open(boost::asio::ip::udp::v4());
			sendSocketv4->set_option(boost::asio::socket_base::send_buffer_size(sendsockBuffer));
			bind_tries = 0;
			udp::endpoint send_endpoint;
			while (not_bind && bind_tries < MAX_BIND_TRIES)
			{
				send_endpoint = udp::endpoint(boost::asio::ip::address_v4::from_string(ipit->name), (uint16_t)sendLocv4.port);
				try{
					sendSocketv4->bind(send_endpoint);
					not_bind = false;
				}
				#pragma warning(disable:4101)
				catch (boost::system::system_error const& e)
				{
					logInfo(RTPS_MSG_OUT, "UDPv4 Error binding endpoint: (" << send_endpoint << ")" << " with boost msg: "<<e.what() , C_YELLOW);
					sendLocv4.port++;
				}
				++bind_tries;
			}
			if(!not_bind)
			{
				sendSocketv4->get_option(option);
				logInfo(RTPS_MSG_OUT, "UDPv4: " << sendSocketv4->local_endpoint() << "|| State: " << sendSocketv4->is_open() <<
						" || buffer size: " << option.value(), C_YELLOW);
				initialized = true;
			}
			else
			{
				logWarning(RTPS_MSG_OUT,"UDPv4: Maxmimum Number of tries while binding in this interface: "<<send_endpoint,C_YELLOW)
				mv_sendLocator_v4.erase(mv_sendLocator_v4.end()-1);
				delete(*(mv_send_socket_v4.end()-1));
				mv_send_socket_v4.erase(mv_send_socket_v4.end()-1);
			}
			not_bind = true;

		}
		else if (ipit->type == IPFinder::IP6 && m_useIP6)
		{
			mv_sendLocator_v6.push_back(loc);
			auto sendLocv6 = mv_sendLocator_v6.back();
			sendLocv6.port = loc.port;
			//OPEN SOCKETS:
			mv_send_socket_v6.push_back(new boost::asio::ip::udp::socket(m_send_service));
			auto sendSocketv6 = mv_send_socket_v6.back();
			sendSocketv6->open(boost::asio::ip::udp::v6());
			sendSocketv6->set_option(boost::asio::socket_base::send_buffer_size(sendsockBuffer));
			bind_tries = 0;
			udp::endpoint send_endpoint;
			while (not_bind && bind_tries < MAX_BIND_TRIES)
			{
				boost::asio::ip::address_v6::bytes_type bt;
				for (uint8_t i = 0; i < 16;++i)
					bt[i] = ipit->locator.address[i];
				boost::asio::ip::address_v6 addr = boost::asio::ip::address_v6(bt);
				addr.scope_id(ipit->scope_id);
				send_endpoint = udp::endpoint(addr, (uint16_t)sendLocv6.port);
				//cout << "IP6 ADDRESS: "<< send_endpoint << endl;
				try{
					sendSocketv6->bind(send_endpoint);
					not_bind = false;
				}
                #pragma warning(disable:4101)
				catch (boost::system::system_error const& e)
				{
					logInfo(RTPS_MSG_OUT, "UDPv6 Error binding endpoint: (" << send_endpoint << ")"<< " with boost msg: "<<e.what() , C_YELLOW);
					sendLocv6.port++;
				}
				++bind_tries;
			}
			if(!not_bind)
			{
				sendSocketv6->get_option(option);
				logInfo(RTPS_MSG_OUT, "UDPv6: " << sendSocketv6->local_endpoint() << "|| State: " << sendSocketv6->is_open() <<
						" || buffer size: " << option.value(), C_YELLOW);
				initialized = true;
			}
			else
			{
				logWarning(RTPS_MSG_OUT,"UDPv6: Maxmimum Number of tries while binding in this endpoint: "<<send_endpoint,C_YELLOW);
				mv_sendLocator_v6.erase(mv_sendLocator_v6.end()-1);
				delete(*(mv_send_socket_v6.end()-1));
				mv_send_socket_v6.erase(mv_send_socket_v6.end()-1);
			}
			not_bind = true;
		}
	}

	return initialized;
}


ResourceSendImpl::~ResourceSendImpl()
{
	const char* const METHOD_NAME = "~SendResource";
	logInfo(RTPS_MSG_OUT,"",C_YELLOW);
	for (auto it = mv_send_socket_v4.begin(); it != mv_send_socket_v4.end(); ++it)
		(*it)->close();
	for (auto it = mv_send_socket_v6.begin(); it != mv_send_socket_v6.end(); ++it)
		(*it)->close();
	m_send_service.stop();
	for (auto it = mv_send_socket_v4.begin(); it != mv_send_socket_v4.end(); ++it)
		delete(*it);
	for (auto it = mv_send_socket_v6.begin(); it != mv_send_socket_v6.end(); ++it)
		delete(*it);
	delete(mp_mutex);
}

void ResourceSendImpl::sendSync(CDRMessage_t* msg, const Locator_t& loc)
{
	const char* const METHOD_NAME = "sendSync";
	boost::lock_guard<boost::recursive_mutex> guard(*this->mp_mutex);
	if(loc.port == 0)
		return;
	if(loc.kind == LOCATOR_KIND_UDPv4 && m_useIP4)
	{
		boost::asio::ip::address_v4::bytes_type addr;
		for(uint8_t i = 0; i < 4; ++i)
			addr[i] = loc.address[12 + i];
        boost::asio::ip::udp::endpoint send_endpoint_v4 = udp::endpoint(boost::asio::ip::address_v4(addr), (uint16_t)loc.port);
		for (auto sockit = mv_send_socket_v4.begin(); sockit != mv_send_socket_v4.end(); ++sockit)
		{
			logInfo(RTPS_MSG_OUT,"UDPv4: " << msg->length << " bytes TO endpoint: " << send_endpoint_v4
					<< " FROM " << (*sockit)->local_endpoint(), C_YELLOW);
			if(send_endpoint_v4.port()>0)
			{
				m_bytes_sent = 0;
				if(m_send_next)
				{
					try {
						m_bytes_sent = (*sockit)->send_to(boost::asio::buffer((void*)msg->buffer, msg->length), send_endpoint_v4);
					}
					catch (const std::exception& error) {
						// Should print the actual error message
						logWarning(RTPS_MSG_OUT, "Error: " << error.what(), C_YELLOW);
					}

				}
				else
				{
					m_send_next = true;
				}
				logInfo (RTPS_MSG_OUT,"SENT " << m_bytes_sent,C_YELLOW);
			}
			else
				logWarning(RTPS_MSG_OUT,"Port invalid",C_YELLOW);
		}
	}
	else if(loc.kind == LOCATOR_KIND_UDPv6 && m_useIP6)
	{
		boost::asio::ip::address_v6::bytes_type addr;
		for(uint8_t i = 0; i < 16; i++)
			addr[i] = loc.address[i];
        boost::asio::ip::udp::endpoint send_endpoint_v6 = udp::endpoint(boost::asio::ip::address_v6(addr), (uint16_t)loc.port);
		for (auto sockit = mv_send_socket_v6.begin(); sockit != mv_send_socket_v6.end(); ++sockit)
		{
			logInfo(RTPS_MSG_OUT, "UDPv6: " << msg->length << " bytes TO endpoint: "
					<< send_endpoint_v6 << " FROM " << (*sockit)->local_endpoint(), C_YELLOW);
			if (send_endpoint_v6.port()>0)
			{
				m_bytes_sent = 0;
				if (m_send_next)
				{
					try {
						m_bytes_sent = (*sockit)->send_to(boost::asio::buffer((void*)msg->buffer, msg->length), send_endpoint_v6);
					}
					catch (const std::exception& error) {
						// Should print the actual error message
						logWarning(RTPS_MSG_OUT, "Error: " << error.what(), C_YELLOW);
					}

				}
				else
				{
					m_send_next = true;
				}
				logInfo(RTPS_MSG_OUT, "SENT " << m_bytes_sent, C_YELLOW);
			}
			else
				logWarning(RTPS_MSG_OUT, "Port invalid", C_YELLOW);
		}
	}
	else
	{
		logInfo(RTPS_MSG_OUT,"Destination "<< loc << " not valid for this ListenResource",C_YELLOW);
	}

}

boost::recursive_mutex* ResourceSendImpl::getMutex() {return mp_mutex;}

}
} /* namespace rtps */
} /* namespace eprosima */
