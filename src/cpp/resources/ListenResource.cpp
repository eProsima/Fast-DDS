/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file ListenResource.cpp
 *
 */

#include "eprosimartps/resources/ListenResource.h"

#include "eprosimartps/writer/RTPSWriter.h"
#include "eprosimartps/reader/RTPSReader.h"
#include "eprosimartps/Endpoint.h"
#include "eprosimartps/Participant.h"

#include "eprosimartps/utils/IPFinder.h"
#include "eprosimartps/utils/RTPSLog.h"


using boost::asio::ip::udp;

namespace eprosima {
namespace rtps {

static const char* const CLASS_NAME = "ListenResource";

typedef std::vector<RTPSWriter*>::iterator Wit;
typedef std::vector<RTPSReader*>::iterator Rit;

ListenResource::ListenResource(ParticipantImpl*p):
		mp_participantImpl(p),
		mp_thread(NULL),
		m_listen_socket(m_io_service),
		m_MessageReceiver(p->getListenSocketBufferSize())
{
	m_MessageReceiver.mp_threadListen = this;
}

ListenResource::~ListenResource()
{
	const char* const METHOD_NAME = "~ListenResource";
	logWarning(RTPS_MSG_IN,"Removing listening thread " << mp_thread->get_id() << " locator: " << m_listenLoc,EPRO_BLUE);
	m_listen_socket.close();
	m_io_service.stop();
	logInfo(RTPS_MSG_IN,"Joining with thread",EPRO_BLUE);
	mp_thread->join();
	delete(mp_thread);
	logInfo(RTPS_MSG_IN,"Listening thread closed succesfully",EPRO_BLUE);
}

bool ListenResource::removeAssociatedEndpoint(Endpoint* endp)
{
	boost::lock_guard<ListenResource> guard(*this);
	if(endp->getEndpointKind() == WRITER)
	{
		for(Wit wit = m_assocWriters.begin();
				wit!=m_assocWriters.end();++wit)
		{
			if((*wit)->getGuid().entityId == endp->getGuid().entityId)
			{
				m_assocWriters.erase(wit);
				return true;
			}
		}
	}
	else if(endp->getEndpointKind() == READER)
	{
		for(Rit rit = m_assocReaders.begin();rit!=m_assocReaders.end();++rit)
		{
			if((*rit)->getGuid().entityId == endp->getGuid().entityId)
			{
				m_assocReaders.erase(rit);
				return true;
			}
		}
	}
	return false;
}

bool ListenResource::addAssociatedEndpoint(Endpoint* endp)
{
	const char* const METHOD_NAME = "addAssociatedEndpoint";
	boost::lock_guard<ListenResource> guard(*this);
	bool found = false;
	if(endp->getEndpointKind() == WRITER)
	{
		for(std::vector<RTPSWriter*>::iterator wit = m_assocWriters.begin();
				wit!=m_assocWriters.end();++wit)
		{
			if((*wit)->getGuid().entityId == endp->getGuid().entityId)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			m_assocWriters.push_back((RTPSWriter*)endp);
			logInfo(RTPS_MSG_IN,"Endpoint (" << endp->getGuid().entityId << ") added to listen Resource: "<< m_listenLoc,EPRO_BLUE);
			return true;
		}
	}
	else if(endp->getEndpointKind() == READER)
	{
		for(std::vector<RTPSReader*>::iterator rit = m_assocReaders.begin();rit!=m_assocReaders.end();++rit)
		{
			if((*rit)->getGuid().entityId == endp->getGuid().entityId)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			m_assocReaders.push_back((RTPSReader*)endp);
			logInfo(RTPS_MSG_IN,"Endpoint (" << endp->getGuid().entityId << ") added to listen Resource: "<< m_listenLoc,EPRO_BLUE);
			return true;
		}
	}
	return false;
}
bool ListenResource::isListeningTo(const Locator_t& loc)
{

	if(m_listenLoc == loc)
		return true;
	else
		return false;
}



void ListenResource::newCDRMessage(const boost::system::error_code& err, std::size_t msg_size)
{
	const char* const METHOD_NAME = "newCDRMessage";
	if(err == boost::system::errc::success)
	{
		boost::lock_guard<ListenResource> guard(*this);
		m_MessageReceiver.m_rec_msg.length = (uint16_t)msg_size;

		if(m_MessageReceiver.m_rec_msg.length == 0)
		{
			return;
		}
		logInfo(RTPS_MSG_IN,"Msg of length: " << m_MessageReceiver.m_rec_msg.length
				<< " FROM: " << m_sender_endpoint << " TO: " << m_listenLoc,EPRO_BLUE);

		//Get address into Locator
		m_senderLocator.port = m_sender_endpoint.port();
		LOCATOR_ADDRESS_INVALID(m_senderLocator.address);
		for(int i=0;i<4;i++)
		{
			m_senderLocator.address[i+12] = m_sender_endpoint.address().to_v4().to_bytes()[i];
		}
		try
		{
			m_MessageReceiver.processCDRMsg(mp_participantImpl->getGuid().guidPrefix,&m_senderLocator,&m_MessageReceiver.m_rec_msg);
		}
		catch(int e)
		{
			logError(RTPS_MSG_IN, "Error processing message: " << e,EPRO_BLUE);

		}
		//CDRMessage_t msg;
	//	pInfo(BLUE<< "Socket async receive put again to listen "<<DEF<< endl);
		CDRMessage::initCDRMsg(&m_MessageReceiver.m_rec_msg);
		m_listen_socket.async_receive_from(
				boost::asio::buffer((void*)m_MessageReceiver.m_rec_msg.buffer, m_MessageReceiver.m_rec_msg.max_size),
				m_sender_endpoint,
				boost::bind(&ListenResource::newCDRMessage, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
	}
	else if(err == boost::asio::error::operation_aborted)
	{
		logInfo(RTPS_MSG_IN,"Operation in listening socket aborted...",EPRO_BLUE);
		return;
	}
	else
	{
		//CDRMessage_t msg;
		logInfo(RTPS_MSG_IN,"Msg processed, Socket async receive put again to listen ",EPRO_BLUE);
		CDRMessage::initCDRMsg(&m_MessageReceiver.m_rec_msg);
		m_listen_socket.async_receive_from(
				boost::asio::buffer((void*)m_MessageReceiver.m_rec_msg.buffer, m_MessageReceiver.m_rec_msg.max_size),
				m_sender_endpoint,
				boost::bind(&ListenResource::newCDRMessage, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
	}
}

Locator_t ListenResource::init_thread(Locator_t& loc, bool isMulti, bool isFixed)
{
	const char* const METHOD_NAME = "init_thread";
	m_listenLoc = loc;
	boost::asio::ip::address address = boost::asio::ip::address::from_string(m_listenLoc.to_IP4_string());
	if(m_listenLoc.address[12]==0 && m_listenLoc.address[13]==0 && m_listenLoc.address[14]==0 && m_listenLoc.address[15]==0) //LISTEN IN ALL INTERFACES
	{
		logInfo(RTPS_MSG_IN,"Defined Locator IP with 0s (listen to all interfaces), setting first interface as value",EPRO_BLUE);
		LocatorList_t myIP;
		IPFinder::getIPAddress(&myIP);
		m_listenLoc= *myIP.begin();
		m_listenLoc.port = loc.port;
	}
	logInfo(RTPS_MSG_IN,"Initializing in : "<<m_listenLoc,EPRO_BLUE);
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
	m_listen_socket.set_option(boost::asio::socket_base::receive_buffer_size(this->mp_participantImpl->getListenSocketBufferSize()));
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
			logError(RTPS_MSG_IN,"Error: " << e.what() << " : " << m_listen_endpoint,EPRO_BLUE);
			m_listenLoc.kind = -1;
			return m_listenLoc;
		}
	}
	else
	{
		bool binded = false;
		for(uint8_t i =0;i<100;++i)
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
				logInfo(RTPS_MSG_IN,"Tried port "<< m_listen_endpoint.port() << ", trying next...",EPRO_BLUE);
			}
		}
		if(!binded)
		{
			logError(RTPS_MSG_IN,"Tried 100 ports and none was working",EPRO_BLUE);
			m_listenLoc.kind = -1;
			return m_listenLoc;
		}
	}
	boost::asio::socket_base::receive_buffer_size option;
	m_listen_socket.get_option(option);
	logInfo(RTPS_MSG_IN,"Created: " << m_listen_endpoint<< " || Listen buffer size: " << option.value(),EPRO_BLUE);
	if(isMulti)
	{
		logInfo(RTPS_MSG_IN,"Joining group: "<<m_listenLoc.to_IP4_string(),EPRO_BLUE);
		LocatorList_t loclist;
		IPFinder::getIPAddress(&loclist);
		for(LocatorListIterator it=loclist.begin();it!=loclist.end();++it)
			m_listen_socket.set_option( boost::asio::ip::multicast::join_group(address.to_v4(),boost::asio::ip::address_v4::from_string(it->to_IP4_string())) );
	}
	CDRMessage::initCDRMsg(&m_MessageReceiver.m_rec_msg);
	m_listen_socket.async_receive_from(
			boost::asio::buffer((void*)m_MessageReceiver.m_rec_msg.buffer, m_MessageReceiver.m_rec_msg.max_size),
			m_sender_endpoint,
			boost::bind(&ListenResource::newCDRMessage, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));

	mp_thread = new boost::thread(&ListenResource::run_io_service,this);
	mp_participantImpl->ResourceSemaphoreWait();
	return m_listenLoc;

}

void ListenResource::run_io_service()
{
	const char* const METHOD_NAME = "run_io_service";
	logInfo(RTPS_MSG_IN,"Thread: " << mp_thread->get_id() << " listening in IP: " << m_listen_socket.local_endpoint(),EPRO_BLUE) ;

	mp_participantImpl->ResourceSemaphorePost();

	this->m_io_service.run();
}

} /* namespace rtps */
} /* namespace eprosima */
