/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TestReaderSocket.h
 *
 */

#ifndef TESTREADERSOCKET_H_
#define TESTREADERSOCKET_H_

#include "fastrtps/rtps/rtps_fwd.h"
using namespace eprosima::fastrtps::rtps;

#include "fastrtps/rtps/reader/ReaderListener.h"

class TestReaderSocket {
public:
	TestReaderSocket();
	virtual ~TestReaderSocket();
	RTPSParticipant* mp_participant;
	RTPSReader* mp_reader;
	ReaderHistory* mp_history;
	bool init(std::string ip,uint32_t port);
	void run();
	class MyListener:public ReaderListener
	{
	public:
		MyListener():m_received(0){};
		~MyListener(){};
		void onNewCacheChangeAdded(RTPSReader* reader,const CacheChange_t* const change);
		uint32_t m_received;
	}m_listener;

};

#endif /* TESTREADER_H_ */
