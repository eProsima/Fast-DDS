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
 * @file BaseReader.hpp
 */

#ifndef RTPS_READER__BASEREADER_HPP
#define RTPS_READER__BASEREADER_HPP

#include <gmock/gmock.h>

#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/rtps/attributes/ReaderAttributes.h>
#include <fastdds/rtps/attributes/WriterAttributes.h>
#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/Endpoint.h>
#include <fastdds/rtps/history/ReaderHistory.h>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/reader/ReaderListener.h>

namespace eprosima {

namespace fastrtps {
namespace rtps {

struct CacheChange_t;
class IDataSharingListener;
struct ReaderHistoryState;
class ReaderListener;
class RTPSParticipantImpl;
class WriterProxy;
class WriterProxyData;

} // namespace rtps
} // namespace fastrtps

namespace fastdds {
namespace rtps {

class BaseReader : public fastrtps::rtps::RTPSReader
{
public:

    BaseReader()
        : fastrtps::rtps::RTPSReader()
    {
    }

    BaseReader(
            fastrtps::rtps::ReaderHistory* history,
            fastrtps::RecursiveTimedMutex* mutex)
        : fastrtps::rtps::RTPSReader(history, mutex)
    {
    }

    virtual ~BaseReader() = default;

    static BaseReader* downcast(
            fastrtps::rtps::RTPSReader* reader)
    {
        return static_cast<BaseReader*>(reader);
    }

    static BaseReader* downcast(
            fastrtps::rtps::Endpoint* endpoint)
    {
        return static_cast<BaseReader*>(endpoint);
    }

    fastrtps::rtps::ReaderListener* get_listener() const override
    {
        return listener_;
    }

    virtual bool set_listener(
            fastrtps::rtps::ReaderListener* listener) override
    {
        listener_ = listener;
        return true;
    }

    virtual bool process_data_msg(
            fastrtps::rtps::CacheChange_t*)
    {
        return true;
    }

    virtual bool process_data_frag_msg(
            fastrtps::rtps::CacheChange_t*,
            uint32_t,
            uint32_t,
            uint16_t)
    {
        return true;
    }

    virtual bool process_heartbeat_msg(
            const fastrtps::rtps::GUID_t&,
            uint32_t,
            const fastrtps::rtps::SequenceNumber_t&,
            const fastrtps::rtps::SequenceNumber_t&,
            bool,
            bool)
    {
        return true;
    }

    virtual bool process_gap_msg(
            const fastrtps::rtps::GUID_t&,
            const fastrtps::rtps::SequenceNumber_t&,
            const fastrtps::rtps::SequenceNumberSet_t&)
    {
        return true;
    }

    virtual bool change_removed_by_history(
            fastrtps::rtps::CacheChange_t*,
            fastrtps::rtps::WriterProxy*)
    {
        return true;
    }

    virtual bool begin_sample_access_nts(
            fastrtps::rtps::CacheChange_t* /*change*/,
            fastrtps::rtps::WriterProxy*& /*wp*/,
            bool& /*is_future_change*/)
    {
        return true;
    }

    virtual void end_sample_access_nts(
            fastrtps::rtps::CacheChange_t* /*change*/,
            fastrtps::rtps::WriterProxy*& /*wp*/,
            bool /*mark_as_read*/)
    {
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD1(change_removed_by_history, bool(fastrtps::rtps::CacheChange_t* change));

    MOCK_METHOD2(reserve_cache, bool(fastrtps::rtps::CacheChange_t** a_change, uint32_t dataCdrSerializedSize));

    MOCK_METHOD1(release_cache, void(fastrtps::rtps::CacheChange_t* a_change));
    // *INDENT-ON*

    fastrtps::rtps::ReaderListener* listener_;

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif // RTPS_READER__BASEREADER_HPP
