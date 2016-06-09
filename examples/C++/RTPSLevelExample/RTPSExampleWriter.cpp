#include "RTPSExampleWriter.h"

	my_WriterListener::my_WriterListener():n_matched(0){};
	my_WriterListener::~my_WriterListener(){};
	void my_WriterListener::onWriterMatched(RTPSWriter* writer, MatchingInfo& info)
	{
		if(info.status == MATCHED_MATCHING)
			++n_matched;
	}

	void RTPSExampleWriter::waitformatching(){
	   while(m_listener.n_matched == 0){}
		return;
	}
   	

	RTPSExampleWriter::RTPSExampleWriter(const std::string& topic_name) : my_participant(nullptr), my_writer(nullptr),
        topic_name_(topic_name), initialized_(false), matched_(0), receiving_(false), current_received_count_(0),
        number_samples_expected_(0)
        {
        }

	RTPSExampleWriter::~RTPSExampleWriter(){
            if(participant_ != nullptr)
                RTPSDomain::removeParticipant(my_participant);
        }
	
        void RTPSExampleWriter::init()
        {
	    //Creation of the participant
	    pattr.ParticipantID=2;
	    my_participant = RTPSDomain::createParticipant(pattr);
	    
	    //Creation of the Reader 
	    auto customTransport = std::make_shared<UDPv4Transport::TransportDescriptor>();
    	    customTransport->sendBufferSize = 65536;
    	    customTransport->receiveBufferSize = 65536;
    	    customTransport->granularMode = false;
    	    wattr.disable_builtin_transport();
    	    wattr.add_user_transport_to_pparams(testTransport);
            
            my_writer= RTPSDomain::createRTPSReader(my_participant, watt, my_history, &my_listener);

	    // Register type
	    tattr.topicKind = NO_KEY;
	    tattr.topicDataType = "string";
	    tattr.topicName = topic_name_.c_str();
	    my_participanti->registerWriter(my_writer,tattr, wqos);
            initialized_ = true;
        }

        bool RTPSExampleWriter::isInitialized() { return initialized_; }
	void RTPSExampleWriter::sendData(){
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
		mp_history->add_change(ch);


		}


	}
    
};


