#include <fastrtps/fastrtps_fwd.h>
#include <fastrtps/rtps/RTPSDomain.h>
#include <fastrtps/rtps/participant/RTPSParticipant.h>
#include <fastrtps/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastrtps/rtps/writer/RTPSWriter.h>
#include <fastrtps/rtps/writer/WriterListener.h>
#include <fastrtps/rtps/attributes/WriterAttributes.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/attributes/TopicAttributes.h>
#include <fastrtps/qos/WriterQos.h>

#include <string>
#include <list>
#include <condition_variable>
#include <gtest/gtest.h>
#include <stdio.h>

class my_WriterListener: public WriterListener
{
	public:
		my_WriterListener();
		~my_WriterListener();
		void onWriterMatched(RTPSWriter* writer, MatchingInfo& info);
		int n_matched;
};


class RTPSExampleWriter
{
	private:

	my_WriterListener *my_listener;
	void waitformatching();   	
	
	public:

        RTPSExampleWriter();
        ~RTPSExampleWriter();
        	
        void init();
        bool isInitialized();
	void sendData();

    private:
        RTPSParticipantAttributes pattr;
	RTPSParticipant *my_participant;
        WriterAttributes wattr;
	RTPSWriter *my_writer;
	HistoryAttributes hattr;
	WriterHistory *my_history;
	TopicAttributes tattr;
	WriterQos wqos;
	bool initialized_;
};


