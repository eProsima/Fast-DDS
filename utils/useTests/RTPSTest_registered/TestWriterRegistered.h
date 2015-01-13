/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TestWriterRegistered.h
 *
 */

#ifndef TESTWRITERREGISTERED_H_
#define TESTWRITERREGISTERED_H_

#include "fastrtps/rtps/rtps_fwd.h"
using namespace eprosima::fastrtps::rtps;

#include "fastrtps/rtps/writer/WriterListener.h"

class TestWriterRegistered {
public:
	TestWriterRegistered();
	virtual ~TestWriterRegistered();
	RTPSParticipant* mp_participant;
	RTPSWriter* mp_writer;
	WriterHistory* mp_history;
	bool init(); //Initialize writer
	bool reg(); //Register the Writer
	void run(uint16_t samples); //Run the Writer
	class MyListener :public WriterListener
	{
	public:
		MyListener():n_matched(0){};
		~MyListener(){};
		void onWriterMatched(RTPSWriter* writer, MatchingInfo info)
		{
			if (info.status == MATCHED_MATCHING)
				++n_matched;
		}
		int n_matched;
	}m_listener;
};

#endif /* TESTWRITER_H_ */
