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

ThreadSend::ThreadSend() : send_socket(sendService) {
	// TODO Auto-generated constructor stub
	//Create socket


}

bool ThreadSend::initSend(Locator_t loc)
{
	boost::asio::ip::address addr;
	sendLocator = loc;
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

		sendLocator.address[12] = addr.to_v4().to_bytes()[0];
		sendLocator.address[13] = addr.to_v4().to_bytes()[1];
		sendLocator.address[14] = addr.to_v4().to_bytes()[2];
		sendLocator.address[15] = addr.to_v4().to_bytes()[3];
	}
	catch (std::exception& e)
	{
		std::cerr << "Could not deal with socket. Exception: " << e.what() << std::endl;
		sendLocator = loc;
		sendLocator.address[12] = 127;
		sendLocator.address[13] = 0;
		sendLocator.address[14] = 0;
		sendLocator.address[15] = 1;
	}

	//udp::endpoint send_endpoint = udp::endpoint(boost::asio::ip::address_v4(),sendLocator.port);
	udp::endpoint send_endpoint = udp::endpoint(addr.to_v4(),sendLocator.port);
	//boost::asio::ip::udp::socket s(sendService,send_endpoint);
	send_socket.open(boost::asio::ip::udp::v4());
	send_socket.bind(send_endpoint);
	pInfo ( YELLOW<<"Sending through default address " << send_socket.local_endpoint()<<" Socket state: " << send_socket.is_open() << DEF<<endl);

	//boost::asio::io_service::work work(sendService);
	return true;
}


ThreadSend::~ThreadSend() {
	// TODO Auto-generated destructor stub
	//sendService.stop();
}

void ThreadSend::sendSync(CDRMessage_t* msg, Locator_t* loc)
{
	boost::lock_guard<ThreadSend> guard(*this);
	udp::endpoint send_endpoint;
	if(loc->kind == LOCATOR_KIND_UDPv4)
	{
		boost::asio::ip::address_v4::bytes_type addr;
		for(uint8_t i=0;i<4;i++)
			addr[i] = loc->address[12+i];
		send_endpoint = udp::endpoint(boost::asio::ip::address_v4(addr),loc->port);
	}
	else if(loc->kind == LOCATOR_KIND_UDPv6)
	{
		boost::asio::ip::address_v6::bytes_type addr;
		for(uint8_t i=0;i<16;i++)
			addr[i] = loc->address[i];
		send_endpoint = udp::endpoint(boost::asio::ip::address_v6(addr),loc->port);
	}


	pDebugInfo (YELLOW<< "Sending: " << msg->length << " bytes TO endpoint: " << send_endpoint << " FROM " << send_socket.local_endpoint()  << endl);
//	boost::posix_time::ptime t1,t2;
//	t1 = boost::posix_time::microsec_clock::local_time();
	size_t longitud = send_socket.send_to(boost::asio::buffer((void*)msg->buffer,msg->length),send_endpoint);
//	t2 = boost::posix_time::microsec_clock::local_time();
//	cout<< "TIME total send operation: " <<(t2-t1).total_microseconds()<< endl;
	pDebugInfo (YELLOW <<  "SENT " << longitud << DEF << endl);
}


} /* namespace rtps */
} /* namespace eprosima */
