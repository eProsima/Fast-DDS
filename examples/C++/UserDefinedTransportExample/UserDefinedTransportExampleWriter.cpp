#include "UserDefinedTransportExampleWriter.h"
#include <memory>
#include <fastrtps/transport/UDPv4TransportDescriptor.h>


	my_WriterListener::my_WriterListener():n_matched(0){};
	my_WriterListener::~my_WriterListener(){};

	void my_WriterListener::onWriterMatched(RTPSWriter* writer, MatchingInfo& info)
	{
		if(info.status == MATCHED_MATCHING)
			++n_matched;
	}

	void UserDefinedTransportExampleWriter::waitformatching(){
	   while(my_listener->n_matched == 0){}
		return;
	}
   	

	UserDefinedTransportExampleWriter::UserDefinedTransportExampleWriter() : my_participant(nullptr), my_writer(nullptr),
        initialized_(false)
        {
        }

	UserDefinedTransportExampleWriter::~UserDefinedTransportExampleWriter(){
            if(my_participant != nullptr)
                RTPSDomain::removeRTPSParticipant(my_participant);
        }
	
        void UserDefinedTransportExampleWriter::init()
        {
	    //Creation of the participant

	    auto customTransport = std::make_shared<UDPv4TransportDescriptor>();
    	    customTransport->sendBufferSize = 65536;
    	    customTransport->receiveBufferSize = 65536;
    	    customTransport->granularMode = false;

	    pattr.userTransports.push_back(customTransport);
	    pattr.useBuiltinTransports = false;
	    my_participant = RTPSDomain::createParticipant(pattr);
	    
	    //Creation of the Reader 
	    my_listener = new my_WriterListener();           
	    my_history = new WriterHistory(hattr); 
            my_writer= RTPSDomain::createRTPSWriter(my_participant, wattr, my_history, my_listener);

	    // Register type
	    tattr.topicKind = NO_KEY;
	    tattr.topicDataType = "string";
	    tattr.topicName = "ExampleTopic";
	    my_participant->registerWriter(my_writer,tattr, wqos);
            initialized_ = true;
        }

        bool UserDefinedTransportExampleWriter::isInitialized() { return initialized_; }
	
	
	void UserDefinedTransportExampleWriter::sendData(){
		waitformatching();
		for(int i=0;i<10;i++){
			CacheChange_t * ch = my_writer->new_change(ALIVE);
#if defined(_WIN32)
		ch->serializedPayload.length =
			sprintf_s((char*)ch->serializedPayload.data,255, "My example string %d", i)+1;
#else
		ch->serializedPayload.length =
			sprintf((char*)ch->serializedPayload.data,"My example string %d",i)+1;
#endif
		printf("Sending: %s\n",(char*)ch->serializedPayload.data);
		my_history->add_change(ch);


		}


	}
    


