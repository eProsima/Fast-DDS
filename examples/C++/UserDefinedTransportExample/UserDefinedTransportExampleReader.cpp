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

#include "UserDefinedTransportExampleReader.h"
#include <memory>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>

	
        my_ReaderListener::my_ReaderListener() :n_received(0) {};
        my_ReaderListener::~my_ReaderListener(){};

        void my_ReaderListener::onNewDataMessage(RTPSReader* reader, const CacheChange_t* const change)
        {
		n_received++;
		std::cout << "Received " << n_received << " samples so far" << std::endl;
        }

        void my_ReaderListener::onReaderMatched(RTPSReader* reader,MatchingInfo& info)
        {
		std::cout << "Matched with a Writer" << std::endl;
		return;
    	}


	UserDefinedTransportExampleReader::UserDefinedTransportExampleReader() : my_participant(nullptr), my_reader(nullptr), initialized_(false)
        {
        }

        UserDefinedTransportExampleReader::~UserDefinedTransportExampleReader()
        {
                if(my_participant != nullptr)
			RTPSDomain::removeRTPSParticipant(my_participant);
        }

        void UserDefinedTransportExampleReader::init()
        {
	    //Creation of the participant
	    auto customTransport = std::make_shared<UDPv4TransportDescriptor>();
    	    customTransport->sendBufferSize = 65536;
    	    customTransport->receiveBufferSize = 65536;

	    pattr.userTransports.push_back(customTransport);
	    pattr.useBuiltinTransports = false;
	    my_participant = RTPSDomain::createParticipant(pattr);
	    
	    //Creation of the Reader 
            my_listener = new my_ReaderListener();
	    my_history = new ReaderHistory(hattr); 
            my_reader= RTPSDomain::createRTPSReader(my_participant, rattr, my_history, my_listener);

	    // Register type
	    tattr.topicKind = NO_KEY;
	    tattr.topicDataType = "string";
	    tattr.topicName = "ExampleTopic";
	    my_participant->registerReader(my_reader,tattr, rqos);            
            initialized_ = true;
        }
	bool UserDefinedTransportExampleReader::read(){
	    while(my_listener->n_received < 9){}
	    return true;

	}
        bool UserDefinedTransportExampleReader::isInitialized() { return initialized_; }
