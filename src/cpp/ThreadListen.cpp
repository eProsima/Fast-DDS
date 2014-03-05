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

void ThreadListen::listen() {
	//Initialize socket
	MR.threadListen_ptr = this;
	boost::asio::ip::udp::endpoint sender_endpoint;
	if(first)
	{
		cout << BLUE << "Thread: " << b_thread->get_id() << " listening in IP: " << DEF ;
		cout << BLUE << listen_socket.local_endpoint() << DEF << endl;
		participant->endpointToListenThreadSemaphore->post();
		first = false;
	}
	while(1) //TODOG: Add more reasonable condition, something with boost::thread
	{
		CDRMessage_t msg;
		//Try to block all associated readers
		std::size_t lengthbytes = listen_socket.receive_from(boost::asio::buffer((void*)msg.buffer, msg.max_size), sender_endpoint);
		msg.length = lengthbytes;
		cout << BLUE << "Message received of length: " << msg.length << " from endpoint: " << sender_endpoint << DEF << endl;
		//Get address into Locator
		Locator_t send_loc;
		send_loc.port = sender_endpoint.port();
		LOCATOR_ADDRESS_INVALID(send_loc.address);
		for(int i=0;i<4;i++)
		{
			send_loc.address[i+12] = sender_endpoint.address().to_v4().to_bytes()[i];
		}
		MR.processCDRMsg(participant->guid.guidPrefix,send_loc,msg.buffer,msg.length);
		cout << "Message processed " << endl;
	}
}

void ThreadListen::init_thread() {
	if(!locList.empty()){
		first = true;
		udp::endpoint listen_endpoint(boost::asio::ip::udp::v4(),locList[0].port);
		listen_socket.open(boost::asio::ip::udp::v4());
		listen_socket.bind(listen_endpoint);
		b_thread = new boost::thread(&ThreadListen::listen,this);
	}
}



} /* namespace rtps */
} /* namespace eprosima */

