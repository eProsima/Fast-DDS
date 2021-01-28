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
 * @file LatencyTestTypes.h
 *
 */

#include <stdint.h>
#include <vector>

#ifndef LATENCYTESTTYPES_H_
#define LATENCYTESTTYPES_H_

#include "fastrtps/fastrtps_all.h"

#define MAX_TYPE_SIZE 10 * 1024 * 1024

class LatencyDataSizes
{
public:

    LatencyDataSizes()
    {
        sample_sizes_ = {16 - 4, 1024 - 4, 64512 - 4, 1048576 - 4};
    }

    inline std::vector<uint32_t>& sample_sizes()
    {
        return sample_sizes_;
    }

private:

    std::vector<uint32_t> sample_sizes_;

};

/*
 * Loanable DataTypes should be flat (only used basic types and arrays).
 * Consequently the compiler cannot generate this type allocation code because the array member size is unknown at build
 * time. The LatencyDataType must allocate a suitable buffer for these objects based on the array member size given at
 * runtime. A beforehand knowledge of this type alignment in needed to calculate the right buffer size. The default
 * alignment turn out to be 4 in both msvc and gcc for x86 and x64 architectures. The alignas specifier is used to match
 * this default behaviour in other platforms.
 * This type does not define a comparison operator because the actual data size referenced is unknown. Use the
 * comparison method provided in LatencyDataType.
 * */
class alignas(4) LatencyType
{
public:

    uint32_t seqnum;
    uint8_t data[1];

    LatencyType()
        : seqnum(0)
    {
    }

    LatencyType(
            uint32_t number)
        : seqnum(number)            // TODO(jlbueno) Ask Miguel Barro
    {
    }

    ~LatencyType()
    {
    }

};

class LatencyDataType : public eprosima::fastdds::dds::TopicDataType
{
    // Buffer size for size management
    const uint32_t buffer_size_;
public:


    LatencyDataType()
        : buffer_size_(MAX_TYPE_SIZE - 4)
    {
        setName("LatencyType");
        m_typeSize = MAX_TYPE_SIZE;
        m_isGetKeyDefined = false;
    }

    LatencyDataType(
            const uint32_t& size)
        : buffer_size_(size)
    {
        setName("LatencyType");
        m_typeSize = size + 4;
        m_isGetKeyDefined = false;
    }

    ~LatencyDataType()
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

    void set_size(
            const uint32_t& size);

    bool compare_data(
            const LatencyType& lt1,
            const LatencyType& lt2) const;

    bool is_bounded() const override
    {
        // All plain types are bounded
        return is_plain();
    }

    bool is_plain() const override
    {
        // It is plain because the type has a fixed size
        return true;
    }
};

enum TESTCOMMAND : uint32_t
{
    DEFAULT,
    READY,
    BEGIN,
    STOP,
    STOP_ERROR
};

typedef struct TestCommandType
{
    TESTCOMMAND m_command;
    TestCommandType()
    {
        m_command = DEFAULT;
    }

    TestCommandType(
            TESTCOMMAND com)
        : m_command(com)
    {
    }

}TestCommandType;

class TestCommandDataType : public eprosima::fastdds::dds::TopicDataType
{
public:

    TestCommandDataType()
    {
        setName("TestCommandType");
        m_typeSize = 4;
        m_isGetKeyDefined = false;
    }

    ~TestCommandDataType()
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


#endif /* LATENCYTESTTYPES_H_ */
