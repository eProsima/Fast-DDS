/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TestReaderRegistered.h
 *
 */

#ifndef TESTREADERREGISTERED_H_
#define TESTREADERREGISTERED_H_

#include "fastrtps/rtps/rtps_fwd.h"
using namespace eprosima::fastrtps::rtps;

#include "fastrtps/rtps/reader/ReaderListener.h"

class TestReaderRegistered {
public:
	TestReaderRegistered();
	virtual ~TestReaderRegistered();
	RTPSParticipant* mp_participant;
	RTPSReader* mp_reader;
	ReaderHistory* mp_history;
	bool init(); //Initialization
	bool reg(); //Register
	void run(); //Run
	class MyListener:public ReaderListener
	{
	public:
		MyListener(){};
		~MyListener(){};
		void onNewCacheChangeAdded(RTPSReader* reader,const CacheChange_t* const change);
	}m_listener;
};

#endif /* TESTREADER_H_ */
