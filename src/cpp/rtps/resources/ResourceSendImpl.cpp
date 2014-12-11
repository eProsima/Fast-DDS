/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * ResourceSendImpl.cpp
 *
 */

#include "fastrtps/rtps/resources/ResourceSendImpl.h"
#include "fastrtps/rtps/common/CDRMessage_t.h"
#include "fastrtps/utils/RTPSLog.h"

#include "fastrtps/utils/IPFinder.h"

using boost::asio::ip::udp;

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "SendResource";

ResourceSendImpl::ResourceSendImpl() :
						m_useIP4(true),
						m_useIP6(true),
						//m_send_socket_v4(m_send_service),
						m_send_socket_v6(m_send_service),
						m_bytes_sent(0),
						m_send_next(true),
						mp_RTPSParticipant(nullptr),
						mp_mutex(new boost::recursive_mutex())
{

}

bool ResourceSendImpl::initSend(RTPSParticipantImpl* pimpl,const Locator_t& loc,uint32_t sendsockBuffer,bool useIP4,bool useIP6)
{
	const char* const METHOD_NAME = "initSend";
	m_useIP4 = useIP4;
	m_useIP6 = useIP6;

	LocatorList_t list;
	IPFinder::getIPAddress(&list);
	int index = 0;
	boost::asio::socket_base::send_buffer_size option;
	bool not_bind = true;
	for (auto lit = list.begin(); lit != list.end(); ++lit)
	{
		
		if (m_useIP4)
		{
			mv_sendLocator_v4.push_back(loc);
			mv_sendLocator_v4.at(index).port = loc.port;
			//OPEN SOCKETS:
			mv_send_socket_v4.push_back(new boost::asio::ip::udp::socket(m_send_service));
			mv_send_socket_v4.at(index)->open(boost::asio::ip::udp::v4());
			mv_send_socket_v4.at(index)->set_option(boost::asio::socket_base::send_buffer_size(sendsockBuffer));

			while (not_bind)
			{
				udp::endpoint send_endpoint = udp::endpoint(boost::asio::ip::address_v4(lit->to_IP4_long()), mv_sendLocator_v4.at(index).port);
				try{
					mv_send_socket_v4.at(index)->bind(send_endpoint);
					not_bind = false;
				}
				catch (boost::system::system_error const& e)
				{
					logWarning(RTPS_MSG_OUT, "UDPv4 Error binding: (" << e.what() << ") with socket: " << send_endpoint, C_YELLOW);
					mv_sendLocator_v4.at(index).port++;
				}
			}

			mv_send_socket_v4.at(index)->get_option(option);
			logInfo(RTPS_MSG_OUT, "UDPv4: " << mv_send_socket_v4.at(index)->local_endpoint() << "|| State: " << mv_send_socket_v4.at(index)->is_open() <<
				" || buffer size: " << option.value(), C_YELLOW);
			not_bind = true;
		}
		++index;
	}
	--index;
	if(m_useIP6)
	{
		m_sendLocator_v6 = loc;
		m_sendLocator_v6.port = mv_sendLocator_v4.at(index).port+1;
		//OPEN SOCKETS:
		m_send_socket_v6.open(boost::asio::ip::udp::v4());
		m_send_socket_v6.set_option(boost::asio::socket_base::send_buffer_size(sendsockBuffer));
		while(not_bind)
		{
			udp::endpoint send_endpoint = udp::endpoint(boost::asio::ip::udp::v6(),m_sendLocator_v6.port);
			try{
				m_send_socket_v6.bind(send_endpoint);
				not_bind = false;
			}
			catch (boost::system::system_error const& e)
			{
				logWarning(RTPS_MSG_OUT,"UDPv6 Error binding: ("<<e.what()<< ") in socket: " << send_endpoint,C_YELLOW);
				m_sendLocator_v6.port++;
			}
		}

		m_send_socket_v6.get_option(option);
		logInfo (RTPS_MSG_OUT,"UDPv6: " << m_send_socket_v6.local_endpoint()<<"|| State: " << m_send_socket_v6.is_open() <<
				" || buffer size: " <<option.value(),C_YELLOW);
	}
	return true;
}


ResourceSendImpl::~ResourceSendImpl()
{
	const char* const METHOD_NAME = "~SendResource";
	logInfo(RTPS_MSG_OUT,"",C_YELLOW);
	for (auto it = mv_send_socket_v4.begin(); it != mv_send_socket_v4.end(); ++it)
		(*it)->close();
	m_send_socket_v6.close();
	m_send_service.stop();
	for (auto it = mv_send_socket_v4.begin(); it != mv_send_socket_v4.end(); ++it)
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
		for(uint8_t i=0;i<4;i++)
			addr[i] = loc.address[12+i];
		m_send_endpoint_v4 = udp::endpoint(boost::asio::ip::address_v4(addr),loc.port);
		for (auto sockit = mv_send_socket_v4.begin(); sockit != mv_send_socket_v4.end(); ++sockit)
		{
		logInfo(RTPS_MSG_OUT,"UDPv4: " << msg->length << " bytes TO endpoint: " << m_send_endpoint_v4
			<< " FROM " << (*sockit)->local_endpoint(), C_YELLOW);
		if(m_send_endpoint_v4.port()>0)
		{
			m_bytes_sent = 0;
			if(m_send_next)
			{
				

					try {
						m_bytes_sent = (*sockit)->send_to(boost::asio::buffer((void*)msg->buffer, msg->length), m_send_endpoint_v4);
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
		else if(m_send_endpoint_v4.port()<=0)
		{
			logWarning(RTPS_MSG_OUT,"Port invalid",C_YELLOW);
		}
		else
			logError(RTPS_MSG_OUT,"Port error",C_YELLOW);
		}
	}
	else if(loc.kind == LOCATOR_KIND_UDPv6 && m_useIP6)
	{
		boost::asio::ip::address_v6::bytes_type addr;
		for(uint8_t i=0;i<16;i++)
			addr[i] = loc.address[i];
		m_send_endpoint_v6 = udp::endpoint(boost::asio::ip::address_v6(addr),loc.port);
		logInfo(RTPS_MSG_OUT,"UDPv6: " << msg->length << " bytes TO endpoint: "
				<< m_send_endpoint_v6 << " FROM " << m_send_socket_v6.local_endpoint(),C_YELLOW);
		if(m_send_endpoint_v6.port()>0)
		{
			m_bytes_sent = 0;
			if(m_send_next)
			{
				try {
					m_bytes_sent = m_send_socket_v6.send_to(boost::asio::buffer((void*)msg->buffer,msg->length),m_send_endpoint_v6);
				} catch (const std::exception& error) {
					// Should print the actual error message
					logWarning(RTPS_MSG_OUT,"Error: " <<error.what(),C_YELLOW);
				}

			}
			else
			{
				m_send_next = true;
			}
			logInfo (RTPS_MSG_OUT,"SENT " << m_bytes_sent,C_YELLOW);
		}
		else if(m_send_endpoint_v6.port()<=0)
		{
			logWarning(RTPS_MSG_OUT,"Port invalid",C_YELLOW);
		}
		else
			logError(RTPS_MSG_OUT,"Port error",C_YELLOW);
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
