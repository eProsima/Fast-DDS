/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * ThreadSend.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *      		grcanosa@gmail.com
 */

#include "eprosimartps/threadtype/ThreadSend.h"

using boost::asio::ip::udp;

namespace eprosima {
namespace rtps {

ThreadSend::ThreadSend() :
	m_send_socket(m_send_service)
{

}

bool ThreadSend::initSend(const Locator_t& loc)
{
	boost::asio::ip::address addr;
	m_sendLocator = loc;
	try {
		boost::asio::io_service netService;
		udp::resolver   resolver(netService);
		udp::resolver::query query(udp::v4(), "google.com", "");
		udp::resolver::iterator endpoints = resolver.resolve(query);
		udp::endpoint ep = *endpoints;
		udp::socket socket(netService);
		socket.connect(ep);
		addr = socket.local_endpoint().address();

		pInfo("My IP according to google is: " << addr.to_string() << endl);

		m_sendLocator.address[12] = addr.to_v4().to_bytes()[0];
		m_sendLocator.address[13] = addr.to_v4().to_bytes()[1];
		m_sendLocator.address[14] = addr.to_v4().to_bytes()[2];
		m_sendLocator.address[15] = addr.to_v4().to_bytes()[3];
	}
	catch (std::exception& e)
	{
		std::cerr << "Could not deal with socket. Exception: " << e.what() << std::endl;
		m_sendLocator = loc;
		m_sendLocator.address[12] = 127;
		m_sendLocator.address[13] = 0;
		m_sendLocator.address[14] = 0;
		m_sendLocator.address[15] = 1;
	}

	//udp::endpoint send_endpoint = udp::endpoint(boost::asio::ip::address_v4(),sendLocator.port);
	udp::endpoint send_endpoint = udp::endpoint(addr.to_v4(),m_sendLocator.port);
	//boost::asio::ip::udp::socket s(sendService,send_endpoint);
	m_send_socket.open(boost::asio::ip::udp::v4());
	m_send_socket.bind(send_endpoint);
	pInfo ( YELLOW<<"Sending through default address " << m_send_socket.local_endpoint()<<" Socket state: " << m_send_socket.is_open() << DEF<<endl);

	//boost::asio::io_service::work work(sendService);
	return true;
}


ThreadSend::~ThreadSend()
{
	pDebugInfo("Send Thread destructor"<<endl;);
}

void ThreadSend::sendSync(CDRMessage_t* msg, Locator_t* loc)
{
	boost::lock_guard<ThreadSend> guard(*this);

	if(loc->kind == LOCATOR_KIND_UDPv4)
	{
		boost::asio::ip::address_v4::bytes_type addr;
		for(uint8_t i=0;i<4;i++)
			addr[i] = loc->address[12+i];
		m_send_endpoint = udp::endpoint(boost::asio::ip::address_v4(addr),loc->port);
	}
	else if(loc->kind == LOCATOR_KIND_UDPv6)
	{
		boost::asio::ip::address_v6::bytes_type addr;
		for(uint8_t i=0;i<16;i++)
			addr[i] = loc->address[i];
		m_send_endpoint = udp::endpoint(boost::asio::ip::address_v6(addr),loc->port);
	}


	pDebugInfo (YELLOW<< "Sending: " << msg->length << " bytes TO endpoint: " << m_send_endpoint << " FROM " << m_send_socket.local_endpoint()  << endl);
	//	boost::posix_time::ptime t1,t2;
	//t1 = boost::posix_time::microsec_clock::local_time();
	size_t longitud = m_send_socket.send_to(boost::asio::buffer((void*)msg->buffer,msg->length),m_send_endpoint);
	//t2 = boost::posix_time::microsec_clock::local_time();
	//cout<< "TIME total send operation: " <<(t2-t1).total_microseconds()<< endl;
	pDebugInfo (YELLOW <<  "SENT " << longitud << DEF << endl);
}


} /* namespace rtps */
} /* namespace eprosima */
