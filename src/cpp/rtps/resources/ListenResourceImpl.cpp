/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * fastrtps_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ListenResourceImpl.cpp
 *
 */

#include "fastrtps/rtps/resources/ListenResourceImpl.h"
#include "fastrtps/rtps/resources/ListenResource.h"
#include "fastrtps/rtps/messages/MessageReceiver.h"
#include "fastrtps/rtps/participant/RTPSParticipantImpl.h"

#include "fastrtps/utils/IPFinder.h"
#include "fastrtps/utils/RTPSLog.h"


using boost::asio::ip::udp;

namespace eprosima {
namespace fastrtps{
namespace rtps {

static const char* const CLASS_NAME = "ListenResourceImpl";

typedef std::vector<RTPSWriter*>::iterator Wit;
typedef std::vector<RTPSReader*>::iterator Rit;

ListenResourceImpl::ListenResourceImpl(ListenResource* LR):
		mp_RTPSParticipantImpl(nullptr),
		mp_listenResource(LR),
		mp_thread(nullptr),
		m_listen_socket(m_io_service)

{

}

ListenResourceImpl::~ListenResourceImpl()
{
	const char* const METHOD_NAME = "~ListenResourceImpl";
	logWarning(RTPS_MSG_IN,"Removing listening thread " << mp_thread->get_id() << " locator: " << m_listenLoc,C_BLUE);
	m_listen_socket.close();
	m_io_service.stop();
	logInfo(RTPS_MSG_IN,"Joining with thread",C_BLUE);
	mp_thread->join();
	delete(mp_thread);
	logInfo(RTPS_MSG_IN,"Listening thread closed succesfully",C_BLUE);
}

bool ListenResourceImpl::isListeningTo(const Locator_t& loc)
{
	if(m_listenLoc == loc)
		return true;
	else
		return false;
}



void ListenResourceImpl::newCDRMessage(const boost::system::error_code& err, std::size_t msg_size)
{
	const char* const METHOD_NAME = "newCDRMessage";
	if(err == boost::system::errc::success)
	{
		boost::lock_guard<boost::recursive_mutex> guard(m_mutex);
		mp_listenResource->setMsgRecMsgLength((uint32_t)msg_size);
		if(msg_size == 0)
			return;

		logInfo(RTPS_MSG_IN,mp_listenResource->mp_receiver->m_rec_msg.length
				<< " bytes FROM: " << m_sender_endpoint << " TO: " << m_listenLoc,C_BLUE);

		//Get address into Locator
		m_senderLocator.port = m_sender_endpoint.port();
		LOCATOR_ADDRESS_INVALID(m_senderLocator.address);
		for(int i=0;i<4;i++)
		{
			m_senderLocator.address[i+12] = m_sender_endpoint.address().to_v4().to_bytes()[i];
		}
		try
		{
			mp_listenResource->mp_receiver->processCDRMsg(mp_RTPSParticipantImpl->getGuid().guidPrefix,
					&m_senderLocator,
					&mp_listenResource->mp_receiver->m_rec_msg);
		}
		catch(int e)
		{
			logError(RTPS_MSG_IN, "Error processing message: " << e,C_BLUE);

		}
		this->putToListen();
	}
	else if(err == boost::asio::error::operation_aborted)
	{
		logInfo(RTPS_MSG_IN,"Operation in listening socket aborted...",C_BLUE);
		return;
	}
	else
	{
		//CDRMessage_t msg;
		logInfo(RTPS_MSG_IN,"Msg processed, Socket async receive put again to listen ",C_BLUE);
		this->putToListen();
	}
}

Locator_t ListenResourceImpl::init_thread(RTPSParticipantImpl* pimpl,Locator_t& loc, uint32_t listenSocketSize, bool isMulti, bool isFixed)
{
	const char* const METHOD_NAME = "init_thread";
	this->mp_RTPSParticipantImpl = pimpl;
	m_listenLoc = loc;
	boost::asio::ip::address address = boost::asio::ip::address::from_string(m_listenLoc.to_IP4_string());
	if(m_listenLoc.address[12]==0 && m_listenLoc.address[13]==0 && m_listenLoc.address[14]==0 && m_listenLoc.address[15]==0) //LISTEN IN ALL INTERFACES
	{
		logInfo(RTPS_MSG_IN,"Defined Locator IP with 0s (listen to all interfaces), setting first interface as value",C_BLUE);
		LocatorList_t myIP;
		IPFinder::getIPAddress(&myIP);
		m_listenLoc= *myIP.begin();
		m_listenLoc.port = loc.port;
	}
	logInfo(RTPS_MSG_IN,"Initializing in : "<<m_listenLoc,C_BLUE);
	if(isMulti)
	{
		m_listen_endpoint = udp::endpoint(boost::asio::ip::udp::v4(),m_listenLoc.port);
	}
	else
	{
		//m_listen_endpoint = udp::endpoint(address,m_listenLoc.port);
		m_listen_endpoint = udp::endpoint(boost::asio::ip::udp::v4(),m_listenLoc.port);
	}
	//OPEN THE SOCKET:
	m_listen_socket.open(m_listen_endpoint.protocol());
	m_listen_socket.set_option(boost::asio::socket_base::receive_buffer_size(listenSocketSize));
	if(isMulti)
	{
		m_listen_socket.set_option( boost::asio::ip::udp::socket::reuse_address( true ) );
		m_listen_socket.set_option( boost::asio::ip::multicast::enable_loopback( true ) );
	}
	if(isFixed)
	{
		try
		{
			m_listen_socket.bind(m_listen_endpoint);
		}
		catch (boost::system::system_error const& e)
		{
			logError(RTPS_MSG_IN,"Error: " << e.what() << " : " << m_listen_endpoint,C_BLUE);
			m_listenLoc.kind = -1;
			return m_listenLoc;
		}
	}
	else
	{
		bool binded = false;
		for(uint8_t i =0;i<1000;++i)
		{
			m_listen_endpoint.port(m_listen_endpoint.port()+i);
			try
			{
				m_listen_socket.bind(m_listen_endpoint);
				binded = true;
				m_listenLoc.port = m_listen_endpoint.port();
				break;
			}
			catch(boost::system::system_error const& )
			{
				logInfo(RTPS_MSG_IN,"Tried port "<< m_listen_endpoint.port() << ", trying next...",C_BLUE);
			}
		}
		if(!binded)
		{
			logError(RTPS_MSG_IN,"Tried 1000 ports and none was working",C_BLUE);
			m_listenLoc.kind = -1;
			return m_listenLoc;
		}
	}
	boost::asio::socket_base::receive_buffer_size option;
	m_listen_socket.get_option(option);
	logInfo(RTPS_MSG_IN,"Created: " << m_listen_endpoint<< " || Listen buffer size: " << option.value(),C_BLUE);
	if(isMulti)
	{
		logInfo(RTPS_MSG_IN,"Joining group: "<<m_listenLoc.to_IP4_string(),C_BLUE);
		LocatorList_t loclist;
		IPFinder::getIPAddress(&loclist);
		for(LocatorListIterator it=loclist.begin();it!=loclist.end();++it)
			m_listen_socket.set_option( boost::asio::ip::multicast::join_group(address.to_v4(),boost::asio::ip::address_v4::from_string(it->to_IP4_string())) );
	}
	this->putToListen();

	mp_thread = new boost::thread(&ListenResourceImpl::run_io_service,this);
	mp_RTPSParticipantImpl->ResourceSemaphoreWait();
	return m_listenLoc;
}

void ListenResourceImpl::putToListen()
{
	CDRMessage::initCDRMsg(&mp_listenResource->mp_receiver->m_rec_msg);
		m_listen_socket.async_receive_from(
				boost::asio::buffer((void*)mp_listenResource->mp_receiver->m_rec_msg.buffer,
						mp_listenResource->mp_receiver->m_rec_msg.max_size),
				m_sender_endpoint,
				boost::bind(&ListenResourceImpl::newCDRMessage, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
}

void ListenResourceImpl::run_io_service()
{
	const char* const METHOD_NAME = "run_io_service";
	logInfo(RTPS_MSG_IN,"Thread: " << mp_thread->get_id() << " listening in IP: " << m_listen_socket.local_endpoint(),C_BLUE) ;

	mp_RTPSParticipantImpl->ResourceSemaphorePost();

	this->m_io_service.run();
}
}
} /* namespace rtps */
} /* namespace eprosima */
