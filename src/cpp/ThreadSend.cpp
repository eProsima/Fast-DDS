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

#include "eprosimartps/ThreadSend.h"

using boost::asio::ip::udp;

namespace eprosima {
namespace rtps {

ThreadSend::ThreadSend() : send_socket(sendService) {
	// TODO Auto-generated constructor stub
	//Create socket

	//Found out about my IP
	//Fake IP for now.
	sendLocator.kind = LOCATOR_KIND_UDPv4;
	sendLocator.port = 14242;
	LOCATOR_ADDRESS_INVALID(sendLocator.address);
	sendLocator.address[12] = 127;
	sendLocator.address[13] = 0;
	sendLocator.address[14] = 0;
	sendLocator.address[15] = 1;

	udp::endpoint send_endpoint = udp::endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"),sendLocator.port);
	//boost::asio::ip::udp::socket s(sendService,send_endpoint);
	send_socket.open(boost::asio::ip::udp::v4());
	send_socket.bind(send_endpoint);
	cout << YELLOW<<"Sending through address " << send_socket.local_endpoint() << endl;
	cout << "Socket state: " << send_socket.is_open() << DEF << endl;
	//boost::asio::io_service::work work(sendService);
}

ThreadSend::~ThreadSend() {
	// TODO Auto-generated destructor stub
	//sendService.stop();
}

void ThreadSend::sendSync(CDRMessage_t* msg, Locator_t loc) {
	udp::endpoint send_endpoint = udp::endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"),loc.port);
	cout <<YELLOW<< "Sending: " << msg->length << " bytes TO endpoint: " << send_endpoint << " FROM " << DEF;
	cout << YELLOW << send_socket.local_endpoint()  <<DEF <<endl;
	size_t longitud = send_socket.send_to(boost::asio::buffer((void*)msg->buffer,msg->length),send_endpoint);
	cout <<YELLOW <<  "Mandado mensaje de longitud: " << longitud << DEF << endl;
}


} /* namespace rtps */
} /* namespace eprosima */
