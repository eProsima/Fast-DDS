/*************************************************************************
 * Copyright (c) 2014 eProsima. All rights reserved.
 *
 * This copy of eProsima Fast RTPS is licensed to you under the terms described in the
 * FASTRTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/**
 * @file LatencyPublisher.cpp
 *
 */

#include "LatencyTestPublisher.h"
#include "fastrtps/utils/RTPSLog.h"
#include <numeric>

#define TIME_LIMIT_US 10000

using namespace eprosima;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

uint32_t dataspub[] = {12,28,60,124,252,508,1020,2044,4092,8188,16380};

std::vector<uint32_t> data_size_pub (dataspub, dataspub + sizeof(dataspub) / sizeof(uint32_t) );

static const char * const CLASS_NAME = "LatencyTestPublisher";

LatencyTestPublisher::LatencyTestPublisher():
												mp_participant(nullptr),
												mp_datapub(nullptr),
												mp_commandpub(nullptr),
												mp_datasub(nullptr),
												mp_commandsub(nullptr),
												mp_latency_in(nullptr),
												mp_latency_out(nullptr),
												t_overhead_(0.0),
												n_subscribers(0),
												n_samples(0),
												m_disc_sema(0),
												m_comm_sema(0),
												m_data_sema(0),
												m_status(0),
												n_received(0),
												m_datapublistener(nullptr),
												m_datasublistener(nullptr),
												m_commandpublistener(nullptr),
												m_commandsublistener(nullptr)
{
	m_datapublistener.mp_up = this;
	m_datasublistener.mp_up = this;
	m_commandpublistener.mp_up = this;
	m_commandsublistener.mp_up = this;
}

LatencyTestPublisher::~LatencyTestPublisher()
{
	Domain::removeParticipant(mp_participant);
}


bool LatencyTestPublisher::init(int n_sub, int n_sam, bool reliable, uint32_t pid)
{
	n_samples = n_sam;
	n_subscribers = n_sub;

	//////////////////////////////
	/*
	char date_buffer[9];
	char time_buffer[7];
	time_t t = time(0);   // get time now
	struct tm * now = localtime(&t);
	strftime(date_buffer, 9, "%Y%m%d", now);
	strftime(time_buffer, 7, "%H%M%S", now);
	*/
	output_file_name << "perf_LatencyTest.csv";
	for (std::vector<uint32_t>::iterator it = data_size_pub.begin(); it != data_size_pub.end(); ++it) {
		output_file << "\"" << n_samples << " samples of " << *it + 4 << " bytes (us)\"";
		if (it != data_size_pub.end() - 1)
			output_file << ",";
	}
	output_file << std::endl;
	//////////////////////////////

	ParticipantAttributes PParam;
	PParam.rtps.defaultSendPort = 10042;
	PParam.rtps.builtin.domainId = pid % 230;
	PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
	PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
	PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
	PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
	PParam.rtps.builtin.leaseDuration = c_TimeInfinite;

	PParam.rtps.sendSocketBufferSize = 65536;
	PParam.rtps.listenSocketBufferSize = 2*65536;
	PParam.rtps.setName("Participant_pub");
	mp_participant = Domain::createParticipant(PParam);
	if(mp_participant == nullptr)
		return false;
	Domain::registerType(mp_participant,(TopicDataType*)&latency_t);
	Domain::registerType(mp_participant,(TopicDataType*)&command_t);

    // Calculate overhead
    t_start_ = boost::chrono::steady_clock::now();
	for(int i= 0; i < 1000; ++i)
        t_end_ = boost::chrono::steady_clock::now();
	t_overhead_ = boost::chrono::duration<double, boost::micro>(t_end_ - t_start_) / 1001;
	cout << "Overhead " << t_overhead_.count() << " ns"  << endl;

	// DATA PUBLISHER
	PublisherAttributes PubDataparam;
	PubDataparam.topic.topicDataType = "LatencyType";
	PubDataparam.topic.topicKind = NO_KEY;
    std::ostringstream pt;
    pt << "LatencyTest_" << boost::asio::ip::host_name() << "_" << pid << "_PUB2SUB";
    PubDataparam.topic.topicName = pt.str();
	PubDataparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	PubDataparam.topic.historyQos.depth = n_samples +100;
	PubDataparam.topic.resourceLimitsQos.max_samples = n_samples +100;
	PubDataparam.topic.resourceLimitsQos.allocated_samples = n_samples +100;//n_samples+100;
    if(!reliable)
        PubDataparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
	Locator_t loc;
	loc.port = 15000;
	PubDataparam.unicastLocatorList.push_back(loc);
	mp_datapub = Domain::createPublisher(mp_participant,PubDataparam,(PublisherListener*)&this->m_datapublistener);
	if(mp_datapub == nullptr)
		return false;
	//DATA SUBSCRIBER
	SubscriberAttributes SubDataparam;
	SubDataparam.topic.topicDataType = "LatencyType";
	SubDataparam.topic.topicKind = NO_KEY;
    std::ostringstream st;
    st << "LatencyTest_" << boost::asio::ip::host_name() << "_" << pid << "_SUB2PUB";
    SubDataparam.topic.topicName = st.str();
	SubDataparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
	SubDataparam.topic.historyQos.depth = 1;
	SubDataparam.topic.resourceLimitsQos.max_samples = 50;//n_samples+100;
	SubDataparam.topic.resourceLimitsQos.allocated_samples = 50;//n_samples+100;
    if(reliable)
        SubDataparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	loc.port = 15001;
	SubDataparam.unicastLocatorList.push_back(loc);



	mp_datasub = Domain::createSubscriber(mp_participant,SubDataparam,&this->m_datasublistener);
	if(mp_datasub == nullptr)
		return false;
	//COMMAND PUBLISHER
	PublisherAttributes PubCommandParam;
	PubCommandParam.topic.topicDataType = "TestCommandType";
	PubCommandParam.topic.topicKind = NO_KEY;
    std::ostringstream pct;
    pct << "LatencyTest_Command_" << boost::asio::ip::host_name() << "_" << pid << "_PUB2SUB";
    PubCommandParam.topic.topicName = pct.str();
	PubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	PubCommandParam.topic.historyQos.depth = 100;
	PubCommandParam.topic.resourceLimitsQos.max_samples = 50;
	PubCommandParam.topic.resourceLimitsQos.allocated_samples = 50;
	PubCommandParam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
	mp_commandpub = Domain::createPublisher(mp_participant,PubCommandParam,&this->m_commandpublistener);
	if(mp_commandpub == nullptr)
		return false;
	SubscriberAttributes SubCommandParam;
	SubCommandParam.topic.topicDataType = "TestCommandType";
	SubCommandParam.topic.topicKind = NO_KEY;
    std::ostringstream sct;
    sct << "LatencyTest_Command_" << boost::asio::ip::host_name() << "_" << pid << "_SUB2PUB";
    SubCommandParam.topic.topicName = sct.str();
	SubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
	SubCommandParam.topic.historyQos.depth = 100;
	SubCommandParam.topic.resourceLimitsQos.max_samples = 50;
	SubCommandParam.topic.resourceLimitsQos.allocated_samples = 50;
	SubCommandParam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
	SubCommandParam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
	mp_commandsub = Domain::createSubscriber(mp_participant,SubCommandParam,&this->m_commandsublistener);
	if(mp_commandsub == nullptr)
		return false;
	return true;
}

void LatencyTestPublisher::DataPubListener::onPublicationMatched(Publisher* /*pub*/, MatchingInfo& info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << C_MAGENTA << "Data Pub Matched "<<C_DEF<<endl;

		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			std::cout << "More matched subscribers than expected" << std::endl;
			mp_up->m_status = -1;
			mp_up->m_disc_sema.post();
		}
		else
			mp_up->m_disc_sema.post();
	}
}

void LatencyTestPublisher::DataSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << C_MAGENTA << "Data Sub Matched "<<C_DEF<<endl;

		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			std::cout << "More matched subscribers than expected" << std::endl;
			mp_up->m_status = -1;
			mp_up->m_disc_sema.post();
		}
		else
			mp_up->m_disc_sema.post();
	}
}

void LatencyTestPublisher::CommandPubListener::onPublicationMatched(Publisher* /*pub*/, MatchingInfo& info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << C_MAGENTA << "Command Pub Matched "<<C_DEF<<endl;

		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			std::cout << "More matched subscribers than expected" << std::endl;
			mp_up->m_status = -1;
			mp_up->m_disc_sema.post();
		}
		else
			mp_up->m_disc_sema.post();
	}
}

void LatencyTestPublisher::CommandSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
	if(info.status == MATCHED_MATCHING)
	{
		cout << C_MAGENTA << "Command Sub Matched "<<C_DEF<<endl;

		n_matched++;
		if(n_matched > mp_up->n_subscribers)
		{
			std::cout << "More matched subscribers than expected" << std::endl;
			mp_up->m_status = -1;
			mp_up->m_disc_sema.post();
		}
		else
			mp_up->m_disc_sema.post();
	}
}

void LatencyTestPublisher::CommandSubListener::onNewDataMessage(Subscriber* /*sub*/)
{
	TestCommandType command;
	SampleInfo_t info;
	//	cout << "COMMAND RECEIVED"<<endl;
	if(mp_up->mp_commandsub->takeNextData((void*)&command,&info))
	{
		if(info.sampleKind == ALIVE)
		{
			//cout << "ALIVE "<<command.m_command<<endl;
			if(command.m_command == BEGIN)
			{
				//	cout << "POSTING"<<endl;
				eClock::my_sleep(50);
				mp_up->m_comm_sema.post();
			}
		}
	}
	else
		cout<< "Problem reading"<<endl;
}

void LatencyTestPublisher::DataSubListener::onNewDataMessage(Subscriber* /*sub*/)
{
	mp_up->mp_datasub->takeNextData((void*)mp_up->mp_latency_in,&mp_up->m_sampleinfo);

	if(mp_up->mp_latency_in->seqnum == mp_up->mp_latency_out->seqnum)
	{
        mp_up->t_end_ = boost::chrono::steady_clock::now();
        mp_up->times_.push_back(boost::chrono::duration<double, boost::micro>(mp_up->t_end_ - mp_up->t_start_) - mp_up->t_overhead_);
		mp_up->n_received++;
	}

    mp_up->m_data_sema.post();
}

void LatencyTestPublisher::run()
{
	//WAIT FOR THE DISCOVERY PROCESS FO FINISH:
	//EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
	for(uint8_t i = 0;i<n_subscribers*4;++i)
	{
		m_disc_sema.wait();
		if(m_status == -1)
			return;
	}
	cout << C_B_MAGENTA << "DISCOVERY COMPLETE "<<C_DEF<<endl;
	printf("Printing round-trip times in us, statistics for %d samples\n",n_samples);
	printf("   Bytes, Samples,   stdev,    mean,     min,     50%%,     90%%,     99%%,  99.99%%,     max\n");
	printf("--------,--------,--------,--------,--------,--------,--------,--------,--------,--------,\n");
	//int aux;
	for(std::vector<uint32_t>::iterator ndata = data_size_pub.begin(); ndata != data_size_pub.end(); ++ndata)
	{
		if(!this->test(*ndata))
			break;
		eClock::my_sleep(100);
		if (ndata != data_size_pub.end() - 1)
			output_file << ",";
	}
	cout << "REMOVING PUBLISHER"<<endl;
	Domain::removePublisher(this->mp_commandpub);
	cout << "REMOVING SUBSCRIBER"<<endl;
	Domain::removeSubscriber(mp_commandsub);

	std::ofstream outFile;
	outFile.open(output_file_name.str());
	outFile << output_file.str();
	outFile.close();
}

bool LatencyTestPublisher::test(uint32_t datasize)
{
	//cout << "Beginning test of size: "<<datasize+4 <<endl;
	m_status = 0;
	n_received = 0;
	mp_latency_in = new LatencyType((uint16_t)datasize);
	mp_latency_out = new LatencyType((uint16_t)datasize);
	times_.clear();
	TestCommandType command;
	command.m_command = READY;
	mp_commandpub->write(&command);
	//cout << "WAITING FOR COMMAND RESPONSES "<<endl;;
	for(uint8_t i = 0;i<n_subscribers;++i)
	{
		//cout << "WAITING"<<endl;
		m_comm_sema.wait();
		//cout << (int)i << " "<<std::flush;
	}
	//cout << endl;
	//BEGIN THE TEST:
    
    for(unsigned int count = 0; count < n_samples; ++count)
    {
        mp_latency_in->seqnum = 0;
        mp_latency_out->seqnum = count;
        
        boost::system_time timeout = boost::get_system_time() + boost::posix_time::seconds(1);
        t_start_ = boost::chrono::steady_clock::now();

        mp_datapub->write((void*)mp_latency_out);

        m_data_sema.timed_wait(timeout);
    }

	command.m_command = STOP;
	mp_commandpub->write(&command);

	if(m_status !=0)
	{
		cout << "Error in test "<<endl;
		return false;
	}
	//TEST FINISHED:
	size_t removed=0;
	mp_datapub->removeAllChange(&removed);
	//cout << "   REMOVED: "<< removed<<endl;
	analizeTimes(datasize);
	printStat(m_stats.back());

    delete(mp_latency_in);
    delete(mp_latency_out);

	return true;
}

void LatencyTestPublisher::analizeTimes(uint32_t datasize)
{
	TimeStats TS;
	TS.nbytes = datasize+4;
    TS.received = n_received;
	TS.m_min = *std::min_element(times_.begin(), times_.end());
	TS.m_max = *std::max_element(times_.begin(), times_.end());
    TS.mean = std::accumulate(times_.begin(), times_.end(), boost::chrono::duration<double, boost::micro>(0)).count() / times_.size();

	double auxstdev=0;
	for(std::vector<boost::chrono::duration<double, boost::micro>>::iterator tit = times_.begin(); tit != times_.end(); ++tit)
	{
		auxstdev += pow(((*tit).count() - TS.mean), 2);
	}
	auxstdev = sqrt(auxstdev / times_.size());
	TS.stdev = (double)round(auxstdev);

	std::sort(times_.begin(), times_.end());
	double x= 0;
	double elem, dec;
	x = times_.size() * 0.5;
	dec = modf(x, &elem);
    if(elem != times_.size())
    {
        if(dec == 0.0)
			TS.p50 = ((times_.at((uint64_t)elem - 1) + times_.at((uint64_t)elem)).count() / 2.0);
        else
			TS.p50 = times_.at((uint64_t)elem).count();
    }
    else
		TS.p50 = times_.at((uint64_t)elem - 1).count();

	x = times_.size() * 0.9;
	dec = modf(x,&elem);
    if(elem != times_.size())
    {
        if(dec == 0.0)
			TS.p90 = ((times_.at((uint64_t)elem - 1) + times_.at((uint64_t)elem)).count() / 2.0);
        else
			TS.p90 = times_.at((uint64_t)elem).count();
    }
    else
		TS.p90 = times_.at((uint64_t)elem - 1).count();

	x = times_.size()*0.99;
	dec = modf(x,&elem);
    if(elem != times_.size())
    {
        if(dec == 0.0)
			TS.p99 = ((times_.at((uint64_t)elem - 1) + times_.at((uint64_t)elem)).count() / 2.0);
        else
			TS.p99 = times_.at((uint64_t)elem).count();
    }
    else
		TS.p99 = times_.at((uint64_t)elem - 1).count();

	x = times_.size()*0.9999;
	dec = modf(x,&elem);
    if(elem != times_.size())
    {
        if(dec == 0.0)
			TS.p9999 = ((times_.at((uint64_t)elem - 1) + times_.at((uint64_t)elem)).count() / 2.0);
        else
			TS.p9999 = times_.at((uint64_t)elem).count();
    }
    else
		TS.p9999 = times_.at((uint64_t)elem - 1).count();

	m_stats.push_back(TS);
}


void LatencyTestPublisher::printStat(TimeStats& TS)
{
	output_file << "\"" << TS.mean << "\"";

	printf("%8lu,%8u,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f \n",
			TS.nbytes, TS.received, TS.stdev, TS.mean,
			TS.m_min.count(),
			TS.p50, TS.p90, TS.p99, TS.p9999,
			TS.m_max.count());
}
