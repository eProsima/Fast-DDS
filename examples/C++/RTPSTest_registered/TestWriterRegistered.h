// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
		void onWriterMatched(RTPSWriter* writer, MatchingInfo& info)
		{
			if (info.status == MATCHED_MATCHING)
				++n_matched;
		}
		int n_matched;
	}m_listener;
};

#endif /* TESTWRITER_H_ */
