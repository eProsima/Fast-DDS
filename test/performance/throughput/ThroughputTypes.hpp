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
 * @file ThroughputTypes.h
 *
 */

#ifndef THROUGHPUTTYPES_H_
#define THROUGHPUTTYPES_H_

#include <fastrtps/TopicDataType.h>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/log/Colors.hpp>

#include <chrono>

struct TroughputResults
{
    uint32_t payload_size;
    uint32_t recovery_time_ms;
    uint32_t demand;

    struct PublisherResults
    {
        std::chrono::duration<double, std::micro>  totaltime_us;
        uint64_t send_samples;
        double MBitssec;
        double Packssec;
    }
    publisher;

    struct SubscriberResults
    {
        std::chrono::duration<double, std::micro> totaltime_us;
        uint64_t recv_samples;
        uint32_t lost_samples;
        double MBitssec;
        double Packssec;
    }
    subscriber;

    void compute()
    {
        publisher.MBitssec = (double)publisher.send_samples * payload_size * 8 / publisher.totaltime_us.count();
        publisher.Packssec = (double)publisher.send_samples * 1000000 / publisher.totaltime_us.count();
        subscriber.MBitssec = (double)subscriber.recv_samples * payload_size * 8 / subscriber.totaltime_us.count();
        subscriber.Packssec = (double)subscriber.recv_samples * 1000000 / subscriber.totaltime_us.count();
    }

};

inline void print_results(
        std::vector<TroughputResults> results)
{
    printf("\n");
    printf(
        "[            TEST           ][                    PUBLISHER                      ][                            SUBSCRIBER                        ]\n");
    printf(
        "[ Bytes,Demand,Recovery Time][Sent Samples,Send Time(us),   Packs/sec,  MBits/sec][Rec Samples,Lost Samples,Rec Time(us),   Packs/sec,  MBits/sec]\n");
    printf(
        "[------,------,-------------][------------,-------------,------------,-----------][-----------,------------,------------,------------,-----------]\n");
    for (uint32_t i = 0; i < results.size(); i++)
    {
        printf("%7u,%6u,%13u,%13.0f,%13.0f,%12.3f,%11.3f,%12.0f,%12.0f,%12.0f,%12.3f,%11.3f\n",
                results[i].payload_size,
                results[i].demand,
                results[i].recovery_time_ms,
                (double)results[i].publisher.send_samples,
                results[i].publisher.totaltime_us.count(),
                results[i].publisher.Packssec,
                results[i].publisher.MBitssec,
                (double)results[i].subscriber.recv_samples,
                (double)results[i].subscriber.lost_samples,
                results[i].subscriber.totaltime_us.count(),
                (double)results[i].subscriber.Packssec,
                (double)results[i].subscriber.MBitssec);
    }
    printf("\n");
    fflush(stdout);
}

typedef struct ThroughputType
{
    uint32_t seqnum;
    std::vector<uint8_t> data;

    ThroughputType(
            uint32_t number)
        : seqnum(0)
        , data(number, 0)
    {
    }

} ThroughputType;

inline bool operator ==(
        const ThroughputType& lt1,
        const ThroughputType& lt2)
{
    if (lt1.seqnum != lt2.seqnum)
    {
        return false;
    }
    if (lt1.data.size() != lt2.data.size())
    {
        return false;
    }
    for (size_t i = 0; i < lt1.data.size(); ++i)
    {
        if (lt1.data.at(i) != lt2.data.at(i))
        {
            return false;
        }
    }
    return true;
}

class ThroughputDataType : public eprosima::fastrtps::TopicDataType
{
public:

    ThroughputDataType(
            uint32_t size)
    {
        setName("ThroughputType");
        m_typeSize = size + 4 + 4;
        m_isGetKeyDefined = false;
    }

    ~ThroughputDataType()
    {
    }

    bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

    bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;

    void* createData() override;

    void deleteData(
            void* data) override;

    bool getKey(
            void* /*data*/,
            eprosima::fastrtps::rtps::InstanceHandle_t* /*ihandle*/,
            bool force_md5 = false) override
    {
        (void)force_md5;
        return false;
    }

};

enum e_Command : uint32_t
{
    DEFAULT,
    READY_TO_START,
    BEGIN,
    TEST_STARTS,
    TEST_ENDS,
    ALL_STOPS,
    TEST_RESULTS
};

typedef struct ThroughputCommandType
{
    e_Command m_command;
    uint32_t m_size;
    uint32_t m_demand;
    uint32_t m_lostsamples;
    uint64_t m_lastrecsample;
    uint64_t m_totaltime;

    ThroughputCommandType()
    {
        m_command = DEFAULT;
        m_size = 0;
        m_demand = 0;
        m_lostsamples = 0;
        m_lastrecsample = 0;
        m_totaltime = 0;
    }

    ThroughputCommandType(
            e_Command com)
        : m_command(com)
        , m_size(0)
        , m_demand(0)
        , m_lostsamples(0)
        , m_lastrecsample(0)
        , m_totaltime(0)
    {
    }

} ThroughputCommandType;


inline std::ostream& operator <<(
        std::ostream& output,
        const ThroughputCommandType& com)
{
    switch (com.m_command)
    {
        case (DEFAULT): return output << "DEFAULT";
        case (READY_TO_START): return output << "READY_TO_START";
        case (BEGIN): return output << "BEGIN";
        case (TEST_STARTS): return output << "TEST_STARTS";
        case (TEST_ENDS): return output << "TEST_ENDS";
        case (ALL_STOPS): return output << "ALL_STOPS";
        case (TEST_RESULTS): return output << "TEST RESULTS";
        default: return output << C_B_RED << "UNKNOWN COMMAND" << C_DEF;
    }
    return output;
}

class ThroughputCommandDataType : public eprosima::fastrtps::TopicDataType
{
public:

    ThroughputCommandDataType()
    {
        setName("ThroughputCommand");
        m_typeSize = 4 * sizeof(uint32_t) + 2 * sizeof(uint64_t) + sizeof(double);
        m_isGetKeyDefined = false;
    }

    ~ThroughputCommandDataType()
    {
    }

    bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

    bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;

    void* createData() override;

    void deleteData(
            void* data) override;

    bool getKey(
            void* /*data*/,
            eprosima::fastrtps::rtps::InstanceHandle_t* /*ihandle*/,
            bool force_md5 = false) override
    {
        (void)force_md5;
        return false;
    }

};

#endif /* THROUGHPUTTYPES_H_ */
