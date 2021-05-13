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
 * @file MessageReceiver.h
 */

#ifndef _FASTDDS_RTPS_MESSAGERECEIVER_H_
#define _FASTDDS_RTPS_MESSAGERECEIVER_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <fastdds/rtps/common/all_common.h>

#include <unordered_map>
#include <mutex>
#include <functional>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSParticipantImpl;
class Endpoint;
class RTPSWriter;
class RTPSReader;
struct SubmessageHeader_t;

/**
 * Class MessageReceiver, process the received messages.
 * @ingroup MANAGEMENT_MODULE
 */
class MessageReceiver
{
public:

    /**
     * @param participant
     * @param rec_buffer_size
     */
    MessageReceiver(
            RTPSParticipantImpl* participant,
            uint32_t rec_buffer_size);

    virtual ~MessageReceiver();

    /**
     * Process a new CDR message.
     * @param [in] source_locator Locator indicating the sending address.
     * @param [in] reception_locator Locator indicating the listening address.
     * @param [in] msg Pointer to the message
     */
    void processCDRMsg(
            const Locator_t& source_locator,
            const Locator_t& reception_locator,
            CDRMessage_t* msg);

    // Functions to associate/remove associatedendpoints
    void associateEndpoint(
            Endpoint* to_add);
    void removeEndpoint(
            Endpoint* to_remove);

private:

    std::mutex mtx_;
    std::vector<RTPSWriter*> associated_writers_;
    std::unordered_map<EntityId_t, std::vector<RTPSReader*>> associated_readers_;

    RTPSParticipantImpl* participant_;
    //!Protocol version of the message
    ProtocolVersion_t source_version_;
    //!VendorID that created the message
    VendorId_t source_vendor_id_;
    //!GuidPrefix of the entity that created the message
    GuidPrefix_t source_guid_prefix_;
    //!GuidPrefix of the entity that receives the message. GuidPrefix of the RTPSParticipant.
    GuidPrefix_t dest_guid_prefix_;
    //!Has the message timestamp?
    bool have_timestamp_;
    //!Timestamp associated with the message
    Time_t timestamp_;

#if HAVE_SECURITY
    //!Buffer to process the decoded RTPS message
    CDRMessage_t crypto_msg_;
    //!Buffer to process each decoded RTPS sub-message
    CDRMessage_t crypto_submsg_;
    //!Buffer to process a decoded payload
    SerializedPayload_t crypto_payload_;
#endif // if HAVE_SECURITY

    //! Function used to process a received message
    std::function<void(
                const EntityId_t&,
                CacheChange_t&)> process_data_message_function_;
    //! Function used to process a received fragment message
    std::function<void(
                const EntityId_t&,
                CacheChange_t&,
                uint32_t,
                uint32_t,
                uint16_t)> process_data_fragment_message_function_;

    //!Reset the MessageReceiver to process a new message.
    void reset();

    /**
     * Check the RTPSHeader of a received message.
     * @param msg Pointer to the message.
     * @return True if correct.
     */
    bool checkRTPSHeader(
            CDRMessage_t* msg);
    /**
     * Read the submessage header of a message.
     * @param msg Pointer to the CDRMessage_t to read.
     * @param smh Pointer to the submessageheader structure.
     * @return True if correctly read.
     */
    bool readSubmessageHeader(
            CDRMessage_t* msg,
            SubmessageHeader_t* smh);

    /**
     * Find if there is a reader (in associated_readers_) that will accept a msg directed
     * to the given entity ID.
     */
    bool willAReaderAcceptMsgDirectedTo(
            const EntityId_t& readerID,
            RTPSReader*& first_reader);

    /**
     * Find all readers (in associated_readers_), with the given entity ID, and call the
     * callback provided.
     */
    template<typename Functor>
    void findAllReaders(
            const EntityId_t& readerID,
            const Functor& callback);

    /**@name Processing methods.
     * These methods are designed to read a part of the message
     * and perform the corresponding actions:
     * -Modify the message receiver state if necessary.
     * -Add information to the history.
     * -Return an error if the message is malformed.
     * @param[in,out] msg Pointer to the message
     * @param[in] smh Pointer to the submessage header
     * @return True if correct, false otherwise
     */

    ///@{
    /**
     *
     * @param msg
     * @param smh
     * @return
     */
    bool proc_Submsg_Data(
            CDRMessage_t* msg,
            SubmessageHeader_t* smh);
    bool proc_Submsg_DataFrag(
            CDRMessage_t* msg,
            SubmessageHeader_t* smh);
    bool proc_Submsg_Acknack(
            CDRMessage_t* msg,
            SubmessageHeader_t* smh);
    bool proc_Submsg_Heartbeat(
            CDRMessage_t* msg,
            SubmessageHeader_t* smh);
    bool proc_Submsg_Gap(
            CDRMessage_t* msg,
            SubmessageHeader_t* smh);
    bool proc_Submsg_InfoTS(
            CDRMessage_t* msg,
            SubmessageHeader_t* smh);
    bool proc_Submsg_InfoDST(
            CDRMessage_t* msg,
            SubmessageHeader_t* smh);
    bool proc_Submsg_InfoSRC(
            CDRMessage_t* msg,
            SubmessageHeader_t* smh);
    bool proc_Submsg_NackFrag(
            CDRMessage_t* msg,
            SubmessageHeader_t* smh);
    bool proc_Submsg_HeartbeatFrag(
            CDRMessage_t* msg,
            SubmessageHeader_t* smh);
    ///@}


    /**
     * @name Variants of received data message processing functions.
     *
     * @param[in] reader_id The ID of the reader to which the changes is addressed
     * @param[in] change    The CacheChange with the received data to process
     */
    ///@{
 #if HAVE_SECURITY
    void process_data_message_with_security(
            const EntityId_t& reader_id,
            CacheChange_t& change);
#endif // HAVE_SECURITY

    void process_data_message_without_security(
            const EntityId_t& reader_id,
            CacheChange_t& change);
    ///@}

    /**
     * @name Variants of received data fragment message processing functions.
     *
     * @param[in] reader_id The ID of the reader to which the changes is addressed
     * @param[in] change    The CacheChange with the received data to process
     *
     * @param[in] sample_size             The size of the message
     * @param[in] fragment_starting_num   The index of the first fragment in the message
     * @param[in] fragments_in_submessage The number of fragments in the message
     */
    ///@{
 #if HAVE_SECURITY
    void process_data_fragment_message_with_security(
            const EntityId_t& reader_id,
            CacheChange_t& change,
            uint32_t sample_size,
            uint32_t fragment_starting_num,
            uint16_t fragments_in_submessage);
#endif // HAVE_SECURITY

    void process_data_fragment_message_without_security(
            const EntityId_t& reader_id,
            CacheChange_t& change,
            uint32_t sample_size,
            uint32_t fragment_starting_num,
            uint16_t fragments_in_submessage);
    ///@}

    /**
     * Looks for the statistics specific submessage and notifies statistics related to the received message.
     *
     * @param [in] source_locator Locator indicating the sending address.
     * @param [in] reception_locator Locator indicating the listening address.
     * @param [in] msg Pointer to the message
     *
     * @pre The message header has already been read and validated.
     */
    void notify_network_statistics(
            const Locator_t& source_locator,
            const Locator_t& reception_locator,
            CDRMessage_t* msg);

};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_MESSAGERECEIVER_H_ */
