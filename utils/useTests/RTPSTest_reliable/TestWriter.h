/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima RTPS is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file TestWriter.h
 *
 */

#ifndef TESTWRITER_H_
#define TESTWRITER_H_

#include "fastrtps/rtps/rtps_fwd.h"
using namespace eprosima::fastrtps::rtps;

#include "fastrtps/rtps/writer/WriterListener.h"

class TestWriter {
public:
	TestWriter();
	virtual ~TestWriter();
	RTPSParticipant* mp_participant;
	RTPSWriter* mp_writer;
	WriterHistory* mp_history;
	bool init(); //Initialize writer
	bool reg(); //Register the Writer
	void run(); //Run the Writer
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
