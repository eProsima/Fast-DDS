/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TestReader.h
 *
 */

#ifndef TESTREADER_H_
#define TESTREADER_H_

#include "fastrtps/rtps/rtps_fwd.h"
using namespace eprosima::fastrtps::rtps;

#include "fastrtps/rtps/reader/ReaderListener.h"

class TestReader {
public:
	TestReader();
	virtual ~TestReader();
	RTPSParticipant* mp_participant;
	RTPSReader* mp_reader;
	ReaderHistory* mp_history;
	bool init();
	void run();
	class MyListener:public ReaderListener
	{
	public:
		MyListener(){};
		~MyListener(){};
		void onNewCacheChangeAdded(RTPSReader* reader,const CacheChange_t* const change);
	}m_listener;
};

#endif /* TESTREADER_H_ */
