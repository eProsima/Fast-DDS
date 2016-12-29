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
 * @file Publisher.cpp
 */

#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/publisher/PublisherListener.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>

#include <HelloWorldType.h>

using namespace eprosima::fastrtps;

class PubListener : public PublisherListener
{
    public:

        PubListener() : matched_(0) {};

        ~PubListener() {};

        void onPublicationMatched(Publisher* /*publisher*/, MatchingInfo& info)
        {
            if(info.status == MATCHED_MATCHING)
                matched_++;
            else
                matched_--;
        }

        unsigned int matched_;
};

int main()
{
	ParticipantAttributes participant_attributes;
	Participant* participant = Domain::createParticipant(participant_attributes);

	if(participant == nullptr)
		return 1;

    HelloWorldType type; 
	Domain::registerType(participant,&type);

    PubListener listener;

	//CREATE THE PUBLISHER
	PublisherAttributes publisher_attributes;
	publisher_attributes.topic.topicKind = NO_KEY;
	publisher_attributes.topic.topicDataType = type.getName();
	publisher_attributes.topic.topicName = "HelloWorldTopic";
	publisher_attributes.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	Publisher* publisher = Domain::createPublisher(participant, publisher_attributes, &listener);
	if(publisher == nullptr)
    {
        Domain::removeParticipant(participant);
		return 1;
    }

    while(listener.matched_ == 0)
		eClock::my_sleep(250);

    HelloWorld data;
    data.index(1);
    data.message("HelloWorld");

    while(1)
    {
        publisher->write((void*)&data);

        if(data.index() == 4)
            data.index() = 1;
        else
            ++data.index();

		eClock::my_sleep(250);
    };

	Domain::removeParticipant(participant);

	return 0;
}
