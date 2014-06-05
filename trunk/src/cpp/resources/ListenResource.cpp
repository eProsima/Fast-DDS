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
 *  Created on: May 28, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *              grcanosa@gmail.com  	
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

typedef std::vector<RTPSWriter*>::iterator Wit;
typedef std::vector<RTPSReader*>::iterator Rit;

ListenResource::ListenResource(ParticipantImpl*p):
		mp_participantImpl(p),
		mp_thread(NULL),
		m_listen_socket(m_io_service)
{
	m_MessageReceiver.mp_threadListen = this;
}

ListenResource::~ListenResource()
{
	pWarning("Removing listening thread " << mp_thread->get_id() << std::endl);
	m_listen_socket.close();
	m_io_service.stop();
	pInfo("Joining with thread"<<endl);
	mp_thread->join();
	delete(mp_thread);

}

bool ListenResource::removeAssociatedEndpoint(Endpoint* endp)
{
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
			pInfo("ResourceListen: Endpoint (" << endp->getGuid().entityId << ") added to listen Resource: "<< m_listenLoc.printIP4Port() << endl);
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
			pInfo("ResourceListen: Endpoint (" << endp->getGuid().entityId << ") added to listen Resource: "<< m_listenLoc.printIP4Port() << endl);
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
	if(err == boost::system::errc::success)
	{
		m_MessageReceiver.m_rec_msg.length = msg_size;

		if(m_MessageReceiver.m_rec_msg.length == 0)
		{
			return;
		}
		pInfo (BLUE << "ResourceListen, msg of length: " << m_MessageReceiver.m_rec_msg.length << " FROM: " << m_sender_endpoint << " TO: " << m_listenLoc.printIP4Port()<<  DEF << endl);

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
			pError( "Error processing message of type: " << e << std::endl);

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
		pInfo("Operation in listening socket aborted..."<<endl);
		return;
	}
	else
	{
		//CDRMessage_t msg;
		pInfo(BLUE<< "Msg processed, Socket async receive put again to listen "<<DEF<< endl);
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
	pInfo(BLUE<<"Listen Resource initializing in : "<<loc.printIP4Port()<<DEF<< endl);
	m_listenLoc = loc;
	boost::asio::ip::address address = boost::asio::ip::address::from_string(m_listenLoc.to_IP4_string());
	if(isMulti)
	{
		m_listen_endpoint = udp::endpoint(boost::asio::ip::udp::v4(),m_listenLoc.port);
	}
	else
	{
		m_listen_endpoint = udp::endpoint(address,m_listenLoc.port);

	}
	//OPEN THE SOCKET:
	m_listen_socket.open(m_listen_endpoint.protocol());
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
			pError(e.what() << " : " << m_listen_endpoint <<endl);
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
				pDebugInfo("Tried port "<< m_listen_endpoint.port() << ", trying next..."<<endl);
			}
		}
		if(!binded)
		{
			pError("Tried 100 ports and none was working" <<endl);
			m_listenLoc.kind = -1;
			return m_listenLoc;
		}
	}
	pDebugInfo("Listen endpoint: " << m_listen_endpoint<< endl);
	if(isMulti)
	{
		pDebugInfo("Joining group: "<<m_listenLoc.to_IP4_string()<<endl);
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
	pInfo ( BLUE << "Thread: " << mp_thread->get_id() << " listening in IP: " << m_listen_socket.local_endpoint() << DEF << endl) ;

	mp_participantImpl->ResourceSemaphorePost();

	this->m_io_service.run();
}

} /* namespace rtps */
} /* namespace eprosima */
