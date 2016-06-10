#include "RTPSExampleReader.h"
#include <memory>
#include <fastrtps/transport/UDPv4Transport.h>

	
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


	RTPSExampleReader::RTPSExampleReader() : my_participant(nullptr), my_reader(nullptr), initialized_(false)
        {
        }

        RTPSExampleReader::~RTPSExampleReader()
        {
                if(my_participant != nullptr)
			RTPSDomain::removeRTPSParticipant(my_participant);
        }

        void RTPSExampleReader::init()
        {
	    //Creation of the participant
	    auto customTransport = std::make_shared<UDPv4Transport::TransportDescriptor>();
    	    customTransport->sendBufferSize = 65536;
    	    customTransport->receiveBufferSize = 65536;
    	    customTransport->granularMode = false;

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
	bool RTPSExampleReader::read(){
	    while(my_listener->n_received < 9){}
	    return true;

	}
        bool RTPSExampleReader::isInitialized() { return initialized_; }
