#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/rtps/reader/RTPSReader.h>
#include <fastrtps/rtps/reader/ReaderListener.h>
#include <fastrtps/rtps/attributes/ReaderAttributes.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/qos/ReaderQos.h>
#include <fastrtps/rtps/RTPSDomain.h>

#include <string>
#include <list>
#include <condition_variable>
#include <boost/asio.hpp>

class my_ReaderListener: public ReaderListener 
{
        public:
	    int n_received;
            my_ReaderListener();
            ~my_ReaderListener();
            void onNewDataMessage(RTPSReader* reader, const CacheChange_t* const change);
            void onReaderMatched(RTPSReader* reader,MatchingInfo& info);
};

class RTPSExampleReader
{
    private:
    	my_ReaderListener *my_listener;
    public:
        RTPSExampleReader();
        ~RTPSExampleReader();
        void init();
        bool isInitialized();
	bool read();
    private:
        RTPSParticipantAttributes pattr;
	RTPSParticipant *my_participant;
        ReaderAttributes rattr;
	RTPSReader *my_reader;
	HistoryAttributes hattr;
	ReaderQos rqos;
	TopicAttributes tattr;
	ReaderHistory *my_history;
	bool initialized_;
};
