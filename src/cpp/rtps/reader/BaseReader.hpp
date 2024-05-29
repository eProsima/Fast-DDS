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

#ifndef RTPS_READER__BASEREADER_HPP
#define RTPS_READER__BASEREADER_HPP

#include <cstdint>

#include <fastdds/rtps/reader/RTPSReader.h>
#include <fastdds/statistics/IListeners.hpp>
#include <fastdds/statistics/rtps/StatisticsCommon.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class BaseReader
    : public fastrtps::rtps::RTPSReader
    , public fastdds::statistics::StatisticsReaderImpl
{
protected:

    BaseReader(
            fastrtps::rtps::RTPSParticipantImpl* pimpl,
            const fastrtps::rtps::GUID_t& guid,
            const fastrtps::rtps::ReaderAttributes& att,
            fastrtps::rtps::ReaderHistory* hist,
            fastrtps::rtps::ReaderListener* listen);

    BaseReader(
            fastrtps::rtps::RTPSParticipantImpl* pimpl,
            const fastrtps::rtps::GUID_t& guid,
            const fastrtps::rtps::ReaderAttributes& att,
            const std::shared_ptr<fastrtps::rtps::IPayloadPool>& payload_pool,
            fastrtps::rtps::ReaderHistory* hist,
            fastrtps::rtps::ReaderListener* listen);

    BaseReader(
            fastrtps::rtps::RTPSParticipantImpl* pimpl,
            const fastrtps::rtps::GUID_t& guid,
            const fastrtps::rtps::ReaderAttributes& att,
            const std::shared_ptr<fastrtps::rtps::IPayloadPool>& payload_pool,
            const std::shared_ptr<fastrtps::rtps::IChangePool>& change_pool,
            fastrtps::rtps::ReaderHistory* hist,
            fastrtps::rtps::ReaderListener* listen);


public:

    virtual ~BaseReader();

    /**
     * Whether the reader accepts messages directed to unknown readers.
     *
     * @return true if the reader accepts messages directed to unknown readers, false otherwise.
     */
    bool accept_messages_to_unknown_readers() const
    {
        return m_acceptMessagesToUnknownReaders;
    }

    /**
     * Processes a new DATA message. Previously the message must have been accepted by function acceptMsgDirectedTo.
     *
     * @param change Pointer to the CacheChange_t.
     * @return true if the reader accepts messages from the.
     */
    virtual bool processDataMsg(
            fastrtps::rtps::CacheChange_t* change) = 0;

    /**
     * Processes a new DATA FRAG message.
     *
     * @param change Pointer to the CacheChange_t.
     * @param sampleSize Size of the complete, assembled message.
     * @param fragmentStartingNum Starting number of this particular message.
     * @param fragmentsInSubmessage Number of fragments on this particular message.
     * @return true if the reader accepts message.
     */
    virtual bool processDataFragMsg(
            fastrtps::rtps::CacheChange_t* change,
            uint32_t sampleSize,
            uint32_t fragmentStartingNum,
            uint16_t fragmentsInSubmessage) = 0;

    /**
     * Processes a new HEARTBEAT message.
     * @param writerGUID
     * @param hbCount
     * @param firstSN
     * @param lastSN
     * @param finalFlag
     * @param livelinessFlag
     * @param origin_vendor_id
     * @return true if the reader accepts messages from the.
     */
    virtual bool processHeartbeatMsg(
            const fastrtps::rtps::GUID_t& writerGUID,
            uint32_t hbCount,
            const fastrtps::rtps::SequenceNumber_t& firstSN,
            const fastrtps::rtps::SequenceNumber_t& lastSN,
            bool finalFlag,
            bool livelinessFlag,
            VendorId_t origin_vendor_id = c_VendorId_Unknown) = 0;

    /**
     * Processes a new GAP message.
     * @param writerGUID
     * @param gapStart
     * @param gapList
     * @param origin_vendor_id
     * @return true if the reader accepts messages from the.
     */
    virtual bool processGapMsg(
            const fastrtps::rtps::GUID_t& writerGUID,
            const fastrtps::rtps::SequenceNumber_t& gapStart,
            const fastrtps::rtps::SequenceNumberSet_t& gapList,
            VendorId_t origin_vendor_id = c_VendorId_Unknown) = 0;

#ifdef FASTDDS_STATISTICS

    bool add_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener) override;

    bool remove_statistics_listener(
            std::shared_ptr<fastdds::statistics::IListener> listener) override;

    void set_enabled_statistics_writers_mask(
            uint32_t enabled_writers) override;

#endif // FASTDDS_STATISTICS

private:

    void setup_datasharing(
            const fastrtps::rtps::ReaderAttributes& att);

};

}  // namespace rtps
}  // namespace fastdds
}  // namespace eprosima

#endif /* RTPS_READER__BASEREADER_HPP */
