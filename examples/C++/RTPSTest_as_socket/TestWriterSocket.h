/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TestWriterSocket.h
 *
 */

#ifndef TESTWRITERSOCKET_H_
#define TESTWRITERSOCKET_H_

#include "fastrtps/rtps/rtps_fwd.h"
using namespace eprosima::fastrtps::rtps;

#include <string>
#include <cstdio>
#include <cstdint>

class TestWriterSocket {
public:
	TestWriterSocket();
	virtual ~TestWriterSocket();
	RTPSParticipant* mp_participant;
	RTPSWriter* mp_writer;
	WriterHistory* mp_history;
	bool init(std::string ip,uint32_t port);
	void run(uint16_t nmsgs);
};

#endif /* TESTWRITER_H_ */
