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
		MyListener():n_received(0),n_matched(0){};
		~MyListener(){};
		void onNewCacheChangeAdded(RTPSReader* reader,const CacheChange_t* const change);
		void onReaderMatched(RTPSReader* reader,MatchingInfo& info)
		{
			if(info.status == MATCHED_MATCHING) n_matched++;
		};
		uint32_t n_received;
		uint32_t n_matched;
	}m_listener;
};

#endif /* TESTREADER_H_ */
