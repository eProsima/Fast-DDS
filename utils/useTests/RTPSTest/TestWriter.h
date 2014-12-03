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

class TestWriter {
public:
	TestWriter();
	virtual ~TestWriter();
	RTPSParticipant* mp_participant;
	RTPSWriter* mp_writer;
	WriterHistory* mp_history;
	bool init();
	void run();
};

#endif /* TESTWRITER_H_ */
