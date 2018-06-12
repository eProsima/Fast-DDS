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
 * @file LatencyPublisher.cpp
 *
 */

#include "LatencyTestPublisher.h"
#include "fastrtps/log/Log.h"
#include "fastrtps/log/Colors.h"
#include <numeric>
#include <cmath>
#include <fstream>
#include <inttypes.h>

#define TIME_LIMIT_US 10000

using namespace eprosima;
using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

uint32_t dataspub[] = {12, 28, 60, 124, 252, 508, 1020, 2044, 4092, 8188, 16380};
uint32_t dataspub_large[] = {63996, 131068};

std::vector<uint32_t> data_size_pub;


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
    disc_count_(0),
    comm_count_(0),
    data_count_(0),
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


bool LatencyTestPublisher::init(int n_sub, int n_sam, bool reliable, uint32_t pid, bool hostname, bool export_csv,
        const PropertyPolicy& part_property_policy, const PropertyPolicy& property_policy, bool large_data)
{
    n_samples = n_sam;
    n_subscribers = n_sub;
    n_export_csv = export_csv;
    reliable_ = reliable;

    if(!large_data)
    {
        data_size_pub.assign(dataspub, dataspub + sizeof(dataspub) / sizeof(uint32_t) );
    }
    else
    {
        data_size_pub.assign(dataspub_large, dataspub_large + sizeof(dataspub_large) / sizeof(uint32_t) );
    }

    //////////////////////////////
    /*
       char date_buffer[9];
       char time_buffer[7];
       time_t t = time(0);   // get time now
       struct tm * now = localtime(&t);
       strftime(date_buffer, 9, "%Y%m%d", now);
       strftime(time_buffer, 7, "%H%M%S", now);
       */
    for (std::vector<uint32_t>::iterator it = data_size_pub.begin(); it != data_size_pub.end(); ++it)
    {
        output_file_minimum << "\"" << n_samples << " samples of " << *it + 4 << " bytes (us)\"";
        output_file_average << "\"" << n_samples << " samples of " << *it + 4 << " bytes (us)\"";
        if (it != data_size_pub.end() - 1)
        {
            output_file_minimum << ",";
            output_file_average << ",";
        }

        std::string str_reliable = "besteffort";
        if(reliable_)
        {
            str_reliable = "reliable";
        }

        switch (*it + 4)
        {
            case 16:
                output_file_16 << "\"Minimum of " << n_samples << " samples (" << str_reliable << ")\",";
                output_file_16 << "\"Average of " << n_samples << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 32:
                output_file_32 << "\"Minimum of " << n_samples << " samples (" << str_reliable << ")\",";
                output_file_32 << "\"Average of " << n_samples << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 64:
                output_file_64 << "\"Minimum of " << n_samples << " samples (" << str_reliable << ")\",";
                output_file_64 << "\"Average of " << n_samples << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 128:
                output_file_128 << "\"Minimum of " << n_samples << " samples (" << str_reliable << ")\",";
                output_file_128 << "\"Average of " << n_samples << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 256:
                output_file_256 << "\"Minimum of " << n_samples << " samples (" << str_reliable << ")\",";
                output_file_256 << "\"Average of " << n_samples << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 512:
                output_file_512 << "\"Minimum of " << n_samples << " samples (" << str_reliable << ")\",";
                output_file_512 << "\"Average of " << n_samples << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 1024:
                output_file_1024 << "\"Minimum of " << n_samples << " samples (" << str_reliable << ")\",";
                output_file_1024 << "\"Average of " << n_samples << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 2048:
                output_file_2048 << "\"Minimum of " << n_samples << " samples (" << str_reliable << ")\",";
                output_file_2048 << "\"Average of " << n_samples << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 4096:
                output_file_4096 << "\"Minimum of " << n_samples << " samples (" << str_reliable << ")\",";
                output_file_4096 << "\"Average of " << n_samples << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 8192:
                output_file_8192 << "\"Minimum of " << n_samples << " samples (" << str_reliable << ")\",";
                output_file_8192 << "\"Average of " << n_samples << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 16384:
                output_file_16384 << "\"Minimum of " << n_samples << " samples (" << str_reliable << ")\",";
                output_file_16384 << "\"Average of " << n_samples << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 64000:
                output_file_64000 << "\"Minimum of " << n_samples << " samples (" << str_reliable << ")\",";
                output_file_64000 << "\"Average of " << n_samples << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            case 131072:
                output_file_131072 << "\"Minimum of " << n_samples << " samples (" << str_reliable << ")\",";
                output_file_131072 << "\"Average of " << n_samples << " samples (" << str_reliable << ")\"" << std::endl;
                break;
            default:
                break;
        }
    }
    output_file_minimum << std::endl;
    output_file_average << std::endl;
    //////////////////////////////

    ParticipantAttributes PParam;
    PParam.rtps.defaultSendPort = 10042;
    PParam.rtps.builtin.domainId = pid % 230;
    PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.properties = part_property_policy;

    PParam.rtps.sendSocketBufferSize = 65536;
    PParam.rtps.listenSocketBufferSize = 2*65536;
    PParam.rtps.setName("Participant_pub");
    mp_participant = Domain::createParticipant(PParam);
    if(mp_participant == nullptr)
        return false;
    Domain::registerType(mp_participant,(TopicDataType*)&latency_t);
    Domain::registerType(mp_participant,(TopicDataType*)&command_t);

    // Calculate overhead
    t_start_ = std::chrono::steady_clock::now();
    for(int i= 0; i < 1000; ++i)
        t_end_ = std::chrono::steady_clock::now();
    t_overhead_ = std::chrono::duration<double, std::micro>(t_end_ - t_start_) / 1001;
    cout << "Overhead " << t_overhead_.count() << " ns"  << endl;

    // DATA PUBLISHER
    PublisherAttributes PubDataparam;
    PubDataparam.topic.topicDataType = "LatencyType";
    PubDataparam.topic.topicKind = NO_KEY;
    std::ostringstream pt;
    pt << "LatencyTest_";
    if(hostname)
        pt << asio::ip::host_name() << "_";
    pt << pid << "_PUB2SUB";
    PubDataparam.topic.topicName = pt.str();
    PubDataparam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    PubDataparam.topic.historyQos.depth = n_samples;
    PubDataparam.topic.resourceLimitsQos.max_samples = n_samples + 1;
    PubDataparam.topic.resourceLimitsQos.allocated_samples = n_samples + 1;
    PubDataparam.times.heartbeatPeriod.seconds = 0;
    PubDataparam.times.heartbeatPeriod.fraction = 4294967 * 100;
    if(!reliable)
        PubDataparam.qos.m_reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    Locator_t loc;
    loc.port = 15000;
    PubDataparam.unicastLocatorList.push_back(loc);
    PubDataparam.properties = property_policy;
    if(large_data)
    {
        PubDataparam.historyMemoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        PubDataparam.qos.m_publishMode.kind = eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE;
    }
    mp_datapub = Domain::createPublisher(mp_participant,PubDataparam,(PublisherListener*)&this->m_datapublistener);
    if(mp_datapub == nullptr)
        return false;
    //DATA SUBSCRIBER
    SubscriberAttributes SubDataparam;
    SubDataparam.topic.topicDataType = "LatencyType";
    SubDataparam.topic.topicKind = NO_KEY;
    std::ostringstream st;
    st << "LatencyTest_";
    if(hostname)
        st << asio::ip::host_name() << "_";
    st << pid << "_SUB2PUB";
    SubDataparam.topic.topicName = st.str();
    SubDataparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    SubDataparam.topic.historyQos.depth = 1;
    if(reliable)
        SubDataparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    loc.port = 15001;
    SubDataparam.unicastLocatorList.push_back(loc);
    SubDataparam.properties = property_policy;
    if(large_data)
    {
        SubDataparam.historyMemoryPolicy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    }



    mp_datasub = Domain::createSubscriber(mp_participant,SubDataparam,&this->m_datasublistener);
    if(mp_datasub == nullptr)
        return false;
    //COMMAND PUBLISHER
    PublisherAttributes PubCommandParam;
    PubCommandParam.topic.topicDataType = "TestCommandType";
    PubCommandParam.topic.topicKind = NO_KEY;
    std::ostringstream pct;
    pct << "LatencyTest_Command_";
    if(hostname)
        pct << asio::ip::host_name() << "_";
    pct << pid << "_PUB2SUB";
    PubCommandParam.topic.topicName = pct.str();
    PubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    PubCommandParam.topic.historyQos.depth = 100;
    PubCommandParam.topic.resourceLimitsQos.max_samples = 101;
    PubCommandParam.topic.resourceLimitsQos.allocated_samples = 101;
    PubCommandParam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    mp_commandpub = Domain::createPublisher(mp_participant,PubCommandParam,&this->m_commandpublistener);
    if(mp_commandpub == nullptr)
        return false;
    SubscriberAttributes SubCommandParam;
    SubCommandParam.topic.topicDataType = "TestCommandType";
    SubCommandParam.topic.topicKind = NO_KEY;
    std::ostringstream sct;
    sct << "LatencyTest_Command_";
    if(hostname)
        sct << asio::ip::host_name() << "_";
    sct << pid << "_SUB2PUB";
    SubCommandParam.topic.topicName = sct.str();
    SubCommandParam.topic.historyQos.kind = KEEP_ALL_HISTORY_QOS;
    SubCommandParam.topic.historyQos.depth = 100;
    SubCommandParam.topic.resourceLimitsQos.allocated_samples = 101;
    SubCommandParam.topic.resourceLimitsQos.allocated_samples = 101;
    SubCommandParam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    SubCommandParam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    mp_commandsub = Domain::createSubscriber(mp_participant,SubCommandParam,&this->m_commandsublistener);
    if(mp_commandsub == nullptr)
        return false;
    return true;
}

void LatencyTestPublisher::DataPubListener::onPublicationMatched(Publisher* /*pub*/, MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        cout << C_MAGENTA << "Data Pub Matched "<<C_DEF<<endl;

        n_matched++;
        if(n_matched > mp_up->n_subscribers)
        {
            std::cout << "More matched subscribers than expected" << std::endl;
            mp_up->m_status = -1;
        }

        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void LatencyTestPublisher::DataSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        cout << C_MAGENTA << "Data Sub Matched "<<C_DEF<<endl;

        n_matched++;
        if(n_matched > mp_up->n_subscribers)
        {
            std::cout << "More matched subscribers than expected" << std::endl;
            mp_up->m_status = -1;
        }

        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void LatencyTestPublisher::CommandPubListener::onPublicationMatched(Publisher* /*pub*/, MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        cout << C_MAGENTA << "Command Pub Matched "<<C_DEF<<endl;

        n_matched++;
        if(n_matched > mp_up->n_subscribers)
        {
            std::cout << "More matched subscribers than expected" << std::endl;
            mp_up->m_status = -1;
        }

        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void LatencyTestPublisher::CommandSubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    std::unique_lock<std::mutex> lock(mp_up->mutex_);

    if(info.status == MATCHED_MATCHING)
    {
        cout << C_MAGENTA << "Command Sub Matched "<<C_DEF<<endl;

        n_matched++;
        if(n_matched > mp_up->n_subscribers)
        {
            std::cout << "More matched subscribers than expected" << std::endl;
            mp_up->m_status = -1;
        }

        ++mp_up->disc_count_;
    }
    else
    {
        --mp_up->disc_count_;
    }

    lock.unlock();
    mp_up->disc_cond_.notify_one();
}

void LatencyTestPublisher::CommandSubListener::onNewDataMessage(Subscriber* subscriber)
{
    TestCommandType command;
    SampleInfo_t info;
    //	cout << "COMMAND RECEIVED"<<endl;
    if(subscriber->takeNextData((void*)&command,&info))
    {
        if(info.sampleKind == ALIVE)
        {
            //cout << "ALIVE "<<command.m_command<<endl;
            if(command.m_command == BEGIN)
            {
                //	cout << "POSTING"<<endl;
                mp_up->mutex_.lock();
                ++mp_up->comm_count_;
                mp_up->mutex_.unlock();
                mp_up->comm_cond_.notify_one();
            }
        }
    }
    else
        cout<< "Problem reading"<<endl;
}

void LatencyTestPublisher::DataSubListener::onNewDataMessage(Subscriber* subscriber)
{
    subscriber->takeNextData((void*)mp_up->mp_latency_in,&mp_up->m_sampleinfo);

    if(mp_up->mp_latency_in->seqnum == mp_up->mp_latency_out->seqnum)
    {
        mp_up->t_end_ = std::chrono::steady_clock::now();
        mp_up->times_.push_back(std::chrono::duration<double, std::micro>(mp_up->t_end_ - mp_up->t_start_) - mp_up->t_overhead_);
        mp_up->n_received++;

        // Reset seqnum from out data
        mp_up->mp_latency_out->seqnum = 0;

        mp_up->mutex_.lock();
        if(mp_up->data_count_ == 0)
        {
            ++mp_up->data_count_;
            mp_up->data_cond_.notify_one();
        }
        mp_up->mutex_.unlock();
    }
}

void LatencyTestPublisher::run()
{
    //WAIT FOR THE DISCOVERY PROCESS FO FINISH:
    //EACH SUBSCRIBER NEEDS 4 Matchings (2 publishers and 2 subscribers)
    std::unique_lock<std::mutex> disc_lock(mutex_);
    while(disc_count_ != (n_subscribers * 4)) disc_cond_.wait(disc_lock);
    disc_lock.unlock();

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
        {
            output_file_minimum << ",";
            output_file_average << ",";
        }
    }
    cout << "REMOVING PUBLISHER"<<endl;
    Domain::removePublisher(this->mp_commandpub);
    cout << "REMOVING SUBSCRIBER"<<endl;
    Domain::removeSubscriber(mp_commandsub);

    std::string str_reliable = "besteffort";
    if(reliable_)
        str_reliable = "reliable";

    if (n_export_csv)
    {
        std::ofstream outFile;
        outFile.open("perf_LatencyTest_minimum_" + str_reliable + ".csv");
        outFile << output_file_minimum.str();
        outFile.close();
        outFile.open("perf_LatencyTest_average_" + str_reliable + ".csv");
        outFile << output_file_average.str();
        outFile.close();
        outFile.open("perf_LatencyTest_16_" + str_reliable + ".csv");
        outFile << output_file_16.str();
        outFile.close();
        outFile.open("perf_LatencyTest_32_" + str_reliable + ".csv");
        outFile << output_file_32.str();
        outFile.close();
        outFile.open("perf_LatencyTest_64_" + str_reliable + ".csv");
        outFile << output_file_64.str();
        outFile.close();
        outFile.open("perf_LatencyTest_128_" + str_reliable + ".csv");
        outFile << output_file_128.str();
        outFile.close();
        outFile.open("perf_LatencyTest_256_" + str_reliable + ".csv");
        outFile << output_file_256.str();
        outFile.close();
        outFile.open("perf_LatencyTest_512_" + str_reliable + ".csv");
        outFile << output_file_512.str();
        outFile.close();
        outFile.open("perf_LatencyTest_1024_" + str_reliable + ".csv");
        outFile << output_file_1024.str();
        outFile.close();
        outFile.open("perf_LatencyTest_2048_" + str_reliable + ".csv");
        outFile << output_file_2048.str();
        outFile.close();
        outFile.open("perf_LatencyTest_4096_" + str_reliable + ".csv");
        outFile << output_file_4096.str();
        outFile.close();
        outFile.open("perf_LatencyTest_8192_" + str_reliable + ".csv");
        outFile << output_file_8192.str();
        outFile.close();
        outFile.open("perf_LatencyTest_16384_" + str_reliable + ".csv");
        outFile << output_file_16384.str();
        outFile.close();
    }
}

bool LatencyTestPublisher::test(uint32_t datasize)
{
    //cout << "Beginning test of size: "<<datasize+4 <<endl;
    m_status = 0;
    n_received = 0;
    mp_latency_in = new LatencyType(datasize);
    mp_latency_out = new LatencyType(datasize);
    times_.clear();
    TestCommandType command;
    command.m_command = READY;
    mp_commandpub->write(&command);

    //cout << "WAITING FOR COMMAND RESPONSES "<<endl;;
    std::unique_lock<std::mutex> lock(mutex_);
    while(comm_count_ != n_subscribers) comm_cond_.wait(lock);
    --comm_count_;
    lock.unlock();
    //cout << endl;
    //BEGIN THE TEST:

    for(unsigned int count = 1; count <= n_samples; ++count)
    {
        mp_latency_in->seqnum = 0;
        mp_latency_out->seqnum = count;

        t_start_ = std::chrono::steady_clock::now();
        mp_datapub->write((void*)mp_latency_out);

        lock.lock();
        data_cond_.wait_for(lock, std::chrono::seconds(1), [&]() { return data_count_ > 0; });

        if(data_count_ > 0)
        {
            --data_count_;
        }
        lock.unlock();
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
    analyzeTimes(datasize);
    printStat(m_stats.back());

    delete(mp_latency_in);
    delete(mp_latency_out);

    return true;
}

void LatencyTestPublisher::analyzeTimes(uint32_t datasize)
{
    TimeStats TS;
    TS.nbytes = datasize+4;
    TS.received = n_received;
    TS.m_min = *std::min_element(times_.begin(), times_.end());
    TS.m_max = *std::max_element(times_.begin(), times_.end());
    TS.mean = std::accumulate(times_.begin(), times_.end(), std::chrono::duration<double, std::micro>(0)).count() / times_.size();

    double auxstdev=0;
    for(std::vector<std::chrono::duration<double, std::micro>>::iterator tit = times_.begin(); tit != times_.end(); ++tit)
    {
        auxstdev += pow(((*tit).count() - TS.mean), 2);
    }
    auxstdev = sqrt(auxstdev / times_.size());
    TS.stdev = static_cast<double>(round(auxstdev));

    std::sort(times_.begin(), times_.end());
    size_t elem = 0;

    elem = static_cast<size_t>(times_.size() * 0.5);
    if(elem > 0 && elem <= times_.size())
        TS.p50 = times_.at(--elem).count();
    else
        TS.p50 = NAN;

    elem = static_cast<size_t>(times_.size() * 0.9);
    if(elem > 0 && elem <= times_.size())
        TS.p90 = times_.at(--elem).count();
    else
        TS.p90 = NAN;

    elem = static_cast<size_t>(times_.size() * 0.99);
    if(elem > 0 && elem <= times_.size())
        TS.p99 = times_.at(--elem).count();
    else
        TS.p99 = NAN;

    elem = static_cast<size_t>(times_.size() * 0.9999);
    if(elem > 0 && elem <= times_.size())
        TS.p9999 = times_.at(--elem).count();
    else
        TS.p9999 = NAN;

    m_stats.push_back(TS);
}


void LatencyTestPublisher::printStat(TimeStats& TS)
{
    output_file_minimum << "\"" << TS.m_min.count() << "\"";
    output_file_average << "\"" << TS.mean << "\"";

    switch (TS.nbytes)
    {
        case 16:
            output_file_16 << "\"" << TS.m_min.count() << "\",\"" << TS.mean << "\"" << std::endl;
            break;
        case 32:
            output_file_32 << "\"" << TS.m_min.count() << "\",\"" << TS.mean << "\"" << std::endl;
            break;
        case 64:
            output_file_64 << "\"" << TS.m_min.count() << "\",\"" << TS.mean << "\"" << std::endl;
            break;
        case 128:
            output_file_128 << "\"" << TS.m_min.count() << "\",\"" << TS.mean << "\"" << std::endl;
            break;
        case 256:
            output_file_256 << "\"" << TS.m_min.count() << "\",\"" << TS.mean << "\"" << std::endl;
            break;
        case 512:
            output_file_512 << "\"" << TS.m_min.count() << "\",\"" << TS.mean << "\"" << std::endl;
            break;
        case 1024:
            output_file_1024 << "\"" << TS.m_min.count() << "\",\"" << TS.mean << "\"" << std::endl;
            break;
        case 2048:
            output_file_2048 << "\"" << TS.m_min.count() << "\",\"" << TS.mean << "\"" << std::endl;
            break;
        case 4096:
            output_file_4096 << "\"" << TS.m_min.count() << "\",\"" << TS.mean << "\"" << std::endl;
            break;
        case 8192:
            output_file_8192 << "\"" << TS.m_min.count() << "\",\"" << TS.mean << "\"" << std::endl;
            break;
        case 16384:
            output_file_16384 << "\"" << TS.m_min.count() << "\",\"" << TS.mean << "\"" << std::endl;
            break;
        case 64000:
            output_file_64000 << "\"" << TS.m_min.count() << "\",\"" << TS.mean << "\"" << std::endl;
            break;
        case 131072:
            output_file_131072 << "\"" << TS.m_min.count() << "\",\"" << TS.mean << "\"" << std::endl;
            break;
        default:
            break;
    }

#ifdef _WIN32
    printf("%8I64u,%8u,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f \n",
            TS.nbytes, TS.received, TS.stdev, TS.mean,
            TS.m_min.count(),
            TS.p50, TS.p90, TS.p99, TS.p9999,
            TS.m_max.count());
#else
    printf("%8" PRIu64 ",%8u,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f \n",
            TS.nbytes, TS.received, TS.stdev, TS.mean,
            TS.m_min.count(),
            TS.p50, TS.p90, TS.p99, TS.p9999,
            TS.m_max.count());
#endif
}
