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
 */

#include "eprosimartps/ThreadListen.h"

#include "RTPSWriter.h"
#include "RTPSReader.h"

namespace eprosima {
namespace rtps {

ThreadListen::ThreadListen() {
	// TODO Auto-generated constructor stub

}

ThreadListen::~ThreadListen() {
	// TODO Auto-generated destructor stub
}

void ThreadListen::listen() {
	while(1){
		cout << "Thread: " << b_thread->get_id() << " listening" << endl;
		boost::this_thread::sleep(1);
	}
}

void ThreadListen::init_thread() {
	b_thread = new boost::thread(&ThreadListen::listen,this);
}


} /* namespace rtps */
} /* namespace eprosima */

