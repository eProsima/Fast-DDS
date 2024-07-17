// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_RTPS_READER__BASEREADER_HPP
#define FASTDDS_RTPS_READER__BASEREADER_HPP

#include <gmock/gmock.h>

#include <fastdds/rtps/reader/RTPSReader.hpp>
#include <fastdds/rtps/attributes/ReaderAttributes.hpp>
#include <fastdds/rtps/attributes/WriterAttributes.hpp>
#include <fastdds/rtps/Endpoint.hpp>
#include <fastdds/rtps/history/ReaderHistory.hpp>
#include <fastdds/rtps/interfaces/IReaderDataFilter.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>

#include <rtps/builtin/data/WriterProxyData.hpp>

namespace eprosima {

namespace fastdds {
namespace rtps {

struct CacheChange_t;
class IDataSharingListener;
struct ReaderHistoryState;
class ReaderListener;
class RTPSParticipantImpl;
class WriterProxy;
class WriterProxyData;

} // namespace rtps
} // namespace fastdds

namespace fastdds {
namespace rtps {

class BaseReader : public fastdds::rtps::RTPSReader
{
public:

    BaseReader()
        : fastdds::rtps::RTPSReader()
    {
    }

    BaseReader(
            fastdds::rtps::ReaderHistory* history,
            fastdds::RecursiveTimedMutex* mutex)
        : fastdds::rtps::RTPSReader(history, mutex)
    {
    }

    virtual ~BaseReader() = default;

    static BaseReader* downcast(
            fastdds::rtps::RTPSReader* reader)
    {
        return static_cast<BaseReader*>(reader);
    }

    static BaseReader* downcast(
            fastdds::rtps::Endpoint* endpoint)
    {
        return static_cast<BaseReader*>(endpoint);
    }

    bool matched_writer_add(
            const PublicationBuiltinTopicData& wdata) final
    {
        static_cast<void>(wdata);
        return false;
    }

    virtual bool matched_writer_add_edp(
            const WriterProxyData& wdata) = 0;

    fastdds::rtps::ReaderListener* get_listener() const override
    {
        return listener_;
    }

    virtual bool set_listener(
            fastdds::rtps::ReaderListener* listener) override
    {
        listener_ = listener;
        return true;
    }

    virtual bool process_data_msg(
            fastdds::rtps::CacheChange_t*)
    {
        return true;
    }

    virtual bool process_data_frag_msg(
            fastdds::rtps::CacheChange_t*,
            uint32_t,
            uint32_t,
            uint16_t)
    {
        return true;
    }

    virtual bool process_heartbeat_msg(
            const fastdds::rtps::GUID_t&,
            uint32_t,
            const fastdds::rtps::SequenceNumber_t&,
            const fastdds::rtps::SequenceNumber_t&,
            bool,
            bool)
    {
        return true;
    }

    virtual bool process_gap_msg(
            const fastdds::rtps::GUID_t&,
            const fastdds::rtps::SequenceNumber_t&,
            const fastdds::rtps::SequenceNumberSet_t&)
    {
        return true;
    }

    virtual bool change_removed_by_history(
            fastdds::rtps::CacheChange_t*,
            fastdds::rtps::WriterProxy*)
    {
        return true;
    }

    virtual bool begin_sample_access_nts(
            fastdds::rtps::CacheChange_t* /*change*/,
            fastdds::rtps::WriterProxy*& /*wp*/,
            bool& /*is_future_change*/)
    {
        return true;
    }

    virtual void end_sample_access_nts(
            fastdds::rtps::CacheChange_t* /*change*/,
            fastdds::rtps::WriterProxy*& /*wp*/,
            bool /*mark_as_read*/)
    {
    }

    // *INDENT-OFF* Uncrustify makes a mess with MOCK_METHOD macros
    MOCK_METHOD1(change_removed_by_history, bool(fastdds::rtps::CacheChange_t* change));

    MOCK_METHOD2(reserve_cache, bool(uint32_t, fastdds::rtps::CacheChange_t*&));

    MOCK_METHOD1(release_cache, void(fastdds::rtps::CacheChange_t* a_change));
    // *INDENT-ON*

    fastdds::rtps::ReaderListener* listener_;

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif // FASTDDS_RTPS_READER__BASEREADER_HPP
