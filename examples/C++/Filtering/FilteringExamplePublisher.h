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

#ifndef _FILTERINGEXAMPLE_PUBLISHER_H_
#define _FILTERINGEXAMPLE_PUBLISHER_H_

#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/publisher/PublisherListener.h>

#include "FilteringExamplePubSubTypes.h"

using namespace eprosima::fastrtps;

class FilteringExamplePublisher 
{
public:
	FilteringExamplePublisher();
	virtual ~FilteringExamplePublisher();
	bool init();
	void run();
private:
	Participant *mp_participant;
	Publisher *mp_fast_publisher;
	Publisher *mp_slow_publisher;

	class PubListener : public PublisherListener
	{
	public:
		PubListener() : n_matched(0){};
		~PubListener(){};
		void onPublicationMatched(Publisher* pub,MatchingInfo& info);
		int n_matched;
	} m_listener;
	FilteringExamplePubSubType myType;
};

#endif // _FILTERINGEXAMPLE_PUBLISHER_H_
