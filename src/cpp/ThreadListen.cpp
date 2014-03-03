/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * ThreadListen.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 *      		grcanosa@gmail.com
 */

#include "eprosimartps/ThreadListen.h"

#include "eprosimartps/RTPSWriter.h"
#include "eprosimartps/RTPSReader.h"
#include "eprosimartps/Participant.h"

using boost::asio::ip::udp;


namespace eprosima {
namespace rtps {

ThreadListen::ThreadListen() : listen_socket(io_service) {
	// TODO Auto-generated constructor stub

}

ThreadListen::~ThreadListen() {
	// TODO Auto-generated destructor stub
	cout << "Removing thread " << b_thread->get_id() << endl;
	b_thread->interrupt();

}

//inline std::string IPString(Locator_t loc)
//{
//	std::stringstream ip_str;
//	if(loc.kind == 1) //IP4
//	{
//		for(int i=12;i<15;i++)
//			ip_str << (int)loc.address[i] <<".";
//		ip_str << (int)loc.address[15];
//	}
//	else if(loc.kind == 2) //IP6
//	{
//		for(int i=0;i<15;i++)
//			ip_str << (int)loc.address[i] <<".";
//		ip_str << (int)loc.address[15];
//	}
//	//cout << "IP str: " << ip_str.str().c_str() << endl;
//	return ip_str.str();
//}

void ThreadListen::listen() {
	//Initialize socket
	boost::asio::ip::udp::endpoint sender_endpoint;
	while(1){
		cout << RED << "Thread: " << b_thread->get_id() << " listening in IP: " << DEF ;
		cout << RED << listen_socket.local_endpoint() << DEF <<endl;
		CDRMessage_t msg;
		std::size_t lengthbytes = listen_socket.receive_from(boost::asio::buffer((void*)msg.buffer, msg.max_size), sender_endpoint);
		msg.length = lengthbytes;
		cout << RED << "Message received of length: " << msg.length << " from endpoint: " << sender_endpoint << DEF << endl;
		//Get addrress
		Locator_t send_loc;
		send_loc.port = sender_endpoint.port();
		LOCATOR_ADDRESS_INVALID(send_loc.address);
		for(int i=0;i<4;i++)
		{
			send_loc.address[i+12] = sender_endpoint.address().to_v4().to_bytes()[i];
			cout << (int)send_loc.address[i+12] << ".";
		}
		cout << endl;
		cout << "Before processing the message" << endl;
		MR.reset();
		cout << "reset exit ok";
		cout << " length: " << msg.length << endl;
		MR.processMsg(participant->guid.guidPrefix,send_loc,msg.buffer,msg.length);
		cout << "Message processed " << endl;
	}
}

void ThreadListen::init_thread() {
	if(!locList.empty()){
		MR.reset();
		cout << "message receiver reseted " << endl;
		udp::endpoint listen_endpoint(boost::asio::ip::udp::v4(),locList[0].port);
		listen_socket.open(boost::asio::ip::udp::v4());
		listen_socket.bind(listen_endpoint);
		cout << "Listen socket open?: " << listen_socket.is_open() <<endl;


		b_thread = new boost::thread(&ThreadListen::listen,this);
	}
}


} /* namespace rtps */
} /* namespace eprosima */

