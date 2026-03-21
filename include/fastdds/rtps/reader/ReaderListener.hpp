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
 * @file ReaderListener.hpp
 *
 */

#ifndef FASTDDS_RTPS_READER__READERLISTENER_HPP
#define FASTDDS_RTPS_READER__READERLISTENER_HPP

#include <fastdds/dds/core/status/LivelinessChangedStatus.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/core/status/SampleRejectedStatus.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/MatchingInfo.hpp>
#include <fastdds/rtps/writer/WriterDiscoveryStatus.hpp>


namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSReader;
struct CacheChange_t;

/**
 * Class ReaderListener, to be used by the user to override some of is virtual method to program actions to
 * certain events.
 * @ingroup READER_MODULE
 */
class FASTDDS_EXPORTED_API ReaderListener
{
public:

    ReaderListener() = default;

    virtual ~ReaderListener() = default;

    /**
     * This method is invoked when a new reader matches
     * @param reader Matching reader
     * @param info Matching information of the reader
     */
    virtual void on_reader_matched(
            RTPSReader* reader,
            const MatchingInfo& info)
    {
        static_cast<void>(reader);
        static_cast<void>(info);
    }

    /**
     * This method is called when a new CacheChange_t is added to the ReaderHistory.
     * @param reader Pointer to the reader.
     * @param change Pointer to the CacheChange_t. This is a const pointer to const data
     * to indicate that the user should not dispose of this data himself.
     * To remove the data call the remove_change method of the ReaderHistory.
     * reader->get_history()->remove_change((CacheChange_t*)change).
     */
    virtual void on_new_cache_change_added(
            RTPSReader* reader,
            const CacheChange_t* const change)
    {
        static_cast<void>(reader);
        static_cast<void>(change);
    }

    /**
     * @brief Method called when the liveliness of a reader changes
     * @param reader The reader
     * @param status The liveliness changed status
     */
    virtual void on_liveliness_changed(
            RTPSReader* reader,
            const eprosima::fastdds::dds::LivelinessChangedStatus& status)
    {
        static_cast<void>(reader);
        static_cast<void>(status);
    }

    /**
     * This method is called when a new Writer is discovered, with a Topic that
     * matches that of a local reader, but with an offered QoS that is incompatible
     * with the one requested by the local reader
     * @param reader Pointer to the RTPSReader.
     * @param qos A mask with the bits of all incompatible Qos activated.
     */
    virtual void on_requested_incompatible_qos(
            RTPSReader* reader,
            eprosima::fastdds::dds::PolicyMask qos)
    {
        static_cast<void>(reader);
        static_cast<void>(qos);
    }

    /**
     * This method is called when the reader detects that one or more samples have been lost.
     *
     * @param reader                         Pointer to the RTPSReader.
     * @param sample_lost_since_last_update  The number of samples that were lost since the last time this
     *                                       method was called for the same reader.
     */
    virtual void on_sample_lost(
            RTPSReader* reader,
            int32_t sample_lost_since_last_update)
    {
        static_cast<void>(reader);
        static_cast<void>(sample_lost_since_last_update);
    }

    /**
     * @brief Method called when the discovery information of a writer regarding a reader changes.
     *
     * @param reader       The reader.
     * @param reason       The reason motivating this method to be called.
     * @param writer_guid  The GUID of the writer for which the discovery information changed.
     * @param writer_info  Discovery information about the writer. Will be @c nullptr for reason @c REMOVED_WRITER.
     */
    virtual void on_writer_discovery(
            RTPSReader* reader,
            WriterDiscoveryStatus reason,
            const GUID_t& writer_guid,
            const PublicationBuiltinTopicData* writer_info)
    {
        static_cast<void>(reader);
        static_cast<void>(reason);
        static_cast<void>(writer_guid);
        static_cast<void>(writer_info);
    }

    /**
     * This method is called when the reader rejects a samples.
     *
     * @param reader  Pointer to the RTPSReader.
     * @param reason  Indicates reason for sample rejection.
     * @param change  Pointer to the CacheChange_t. This is a const pointer to const data
     *                to indicate that the user should not dispose of this data himself.
     */
    virtual void on_sample_rejected(
            RTPSReader* reader,
            eprosima::fastdds::dds::SampleRejectedStatusKind reason,
            const CacheChange_t* const change)
    {
        static_cast<void>(reader);
        static_cast<void>(reason);
        static_cast<void>(change);
    }

    /**
     * This method is called when new CacheChange_t objects are made available to the user.
     * @note This method is currently never called. Implementation will be added in future releases.
     *
     * @param [in]  reader                            Pointer to the reader performing the notification.
     * @param [in]  writer_guid                       GUID of the writer from which the changes were received.
     * @param [in]  first_sequence                    Sequence number of the first change made available.
     * @param [in]  last_sequence                     Sequence number of the last change made available.
     *                                                It will always be greater or equal than @c first_sequence.
     * @param [out] should_notify_individual_changes  Whether the individual changes should be notified by means of
     *                                                @c on_new_cache_change_added.
     */
    virtual void on_data_available(
            RTPSReader* reader,
            const GUID_t& writer_guid,
            const SequenceNumber_t& first_sequence,
            const SequenceNumber_t& last_sequence,
            bool& should_notify_individual_changes)
    {
        static_cast<void>(reader);
        static_cast<void>(writer_guid);
        static_cast<void>(first_sequence);
        static_cast<void>(last_sequence);

        should_notify_individual_changes = true;
    }

    /**
     * This method is called when a new Writer is discovered, with a Topic that
     * matches the name of a local reader, but with an incompatible type
     *
     * @param reader Pointer to the RTPSReader.
     */
    virtual void on_incompatible_type(
            RTPSReader* reader)
    {
        static_cast<void>(reader);
    }

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_RTPS_READER__READERLISTENER_HPP
