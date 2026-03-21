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
 * @file WriterListener.hpp
 */
#ifndef FASTDDS_RTPS_WRITER__WRITERLISTENER_HPP
#define FASTDDS_RTPS_WRITER__WRITERLISTENER_HPP

#include <fastdds/dds/core/status/BaseStatus.hpp>
#include <fastdds/dds/core/status/IncompatibleQosStatus.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/MatchingInfo.hpp>
#include <fastdds/rtps/reader/ReaderDiscoveryStatus.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSWriter;
struct CacheChange_t;

/**
 * Class WriterListener with virtual method so the user can implement callbacks to certain events.
 * @ingroup WRITER_MODULE
 */
class FASTDDS_EXPORTED_API WriterListener
{
public:

    WriterListener() = default;

    virtual ~WriterListener() = default;

    /**
     * This method is called when a new Reader is matched with this Writer by the builtin protocols
     * @param writer Pointer to the RTPSWriter.
     * @param info Matching Information.
     */
    virtual void on_writer_matched(
            RTPSWriter* writer,
            const MatchingInfo& info)
    {
        static_cast<void>(writer);
        static_cast<void>(info);
    }

    /**
     * This method is called when a new Reader is discovered, with a Topic that
     * matches that of a local writer, but with a requested QoS that is incompatible
     * with the one offered by the local writer
     * @param writer Pointer to the RTPSWriter.
     * @param qos A mask with the bits of all incompatible Qos activated.
     */
    virtual void on_offered_incompatible_qos(
            RTPSWriter* writer,
            eprosima::fastdds::dds::PolicyMask qos)
    {
        static_cast<void>(writer);
        static_cast<void>(qos);
    }

    /**
     * This method is called when all the readers matched with this Writer acknowledge that a cache
     * change has been received.
     * @param writer Pointer to the RTPSWriter.
     * @param change Pointer to the affected CacheChange_t.
     */
    virtual void on_writer_change_received_by_all(
            RTPSWriter* writer,
            CacheChange_t* change)
    {
        static_cast<void>(writer);
        static_cast<void>(change);
    }

    /**
     * @brief Method called when the liveliness of a writer is lost
     *
     * @param writer The writer
     * @param status The liveliness lost status
     */
    virtual void on_liveliness_lost(
            RTPSWriter* writer,
            const LivelinessLostStatus& status)
    {
        static_cast<void>(writer);
        static_cast<void>(status);
    }

    /**
     * @brief Method called when the discovery information of a reader regarding a writer changes.
     *
     * @param writer       The writer.
     * @param reason       The reason motivating this method to be called.
     * @param reader_guid  The GUID of the reader for which the discovery information changed.
     * @param reader_info  Discovery information about the reader. Will be @c nullptr for reason @c REMOVED_READER.
     */
    virtual void on_reader_discovery(
            RTPSWriter* writer,
            ReaderDiscoveryStatus reason,
            const GUID_t& reader_guid,
            const SubscriptionBuiltinTopicData* reader_info)
    {
        static_cast<void>(writer);
        static_cast<void>(reason);
        static_cast<void>(reader_guid);
        static_cast<void>(reader_info);
    }

    /**
     * This method is called when a new Reader is discovered, with a Topic that
     * matches the name of a local writer, but with an incompatible type
     *
     * @param writer Pointer to the RTPSWriter.
     */
    virtual void on_incompatible_type(
            RTPSWriter* writer)
    {
        static_cast<void>(writer);
    }

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima



#endif // FASTDDS_RTPS_WRITER__WRITERLISTENER_HPP
