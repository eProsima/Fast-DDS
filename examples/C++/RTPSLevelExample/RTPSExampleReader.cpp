#include "RTPSExampleReader.h"
	
        my_ReaderListener::my_ReaderListener() : reader_(reader), n_received(0) {};
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


	RTPSExampleReader::RTPSExampleReader(const std::string& topic_name) : my_participant(nullptr), my_reader(nullptr),
        topic_name_(topic_name), initialized_(false), matched_(0), receiving_(false), current_received_count_(0),
        number_samples_expected_(0)
        {
        }

        RTPSExampleReader::~RTPSExampleReader()
        {
            if(participant_ != nullptr)
                RTPSDomain::removeParticipant(my_participant);
        }

        void RTPSExampleReader::init()
        {
	    //Creation of the participant
            pattr.rtps.builtin.domainId = (uint32_t)boost::interprocess::ipcdetail::get_current_process_id() % 230;
	    pattr.ParticipantID=1;
	    my_participant = RTPSDomain::createParticipant(pattr);
	    
	    //Creation of the Reader 
	    auto customTransport = std::make_shared<UDPv4Transport::TransportDescriptor>();
    	    customTransport->sendBufferSize = 65536;
    	    customTransport->receiveBufferSize = 65536;
    	    customTransport->granularMode = false;
    	    rattr.writer.disable_builtin_transport();
    	    rattr.add_user_transport_to_pparams(testTransport);
            
            my_reader= RTPSDomain::createRTPSReader(my_participant, ratt, my_history, &my_listener);

	    // Register type
	    tattr.topicKind = NO_KEY;
	    tattr.topicDataType = "string";
	    tattr.topicName = topic_name_.c_str();
	    my_participanti->registerReader(my_reader,tattr, rqos);            
            initialized_ = true;
        }

        bool RTPSExampleReader::isInitialized() { return initialized_; }
