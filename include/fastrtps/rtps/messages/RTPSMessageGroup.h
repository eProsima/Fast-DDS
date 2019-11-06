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
 * @file RTPSMessageGroup.h
 *
 */

#ifndef RTPSMESSAGEGROUP_H_
#define RTPSMESSAGEGROUP_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include "./RTPSMessageSenderInterface.hpp"
#include "./RTPSMessageCreator.h"
#include "../../qos/ParameterList.h"
#include <fastrtps/rtps/common/FragmentNumber.h>

#include <vector>
#include <chrono>
#include <cassert>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class RTPSParticipantImpl;
class Endpoint;

/**
 * RTPSMessageGroup Class used to construct a RTPS message.
 * @ingroup WRITER_MODULE
 */
 class RTPSMessageGroup 
{
    public:

        class timeout : public std::runtime_error
        {
            public:

                timeout() : std::runtime_error("timeout") {}

                virtual ~timeout() = default;
        };

        /**
         * Basic constructor.
         * Constructs a RTPSMessageGroup allowing the destination endpoints to change.
         * @param participant Pointer to the participant sending data.
         * @param endpoint Pointer to the endpoint sending data.
         * @param msg_sender Reference to message sender interface.
         * @param max_blocking_time_point Future time point where blocking send should end.
         */
        RTPSMessageGroup(
                RTPSParticipantImpl* participant,
                Endpoint* endpoint,
                const RTPSMessageSenderInterface& msg_sender,
                std::chrono::steady_clock::time_point max_blocking_time_point =
                    std::chrono::steady_clock::now() + std::chrono::hours(24));

        ~RTPSMessageGroup() noexcept(false);

        /**
         * Adds a DATA message to the group.
         * @param change Reference to the cache change to send.
         * @param expects_inline_qos True when one destination is expecting inline QOS.
         * @return True when message was added to the group.
         */
        bool add_data(
                const CacheChange_t& change,
                bool expects_inline_qos);

        /**
         * Adds a DATA_FRAG message to the group.
         * @param change Reference to the cache change to send.
         * @param fragment_number Index (1 based) of the fragment to send.
         * @param expects_inline_qos True when one destination is expecting inline QOS.
         * @return True when message was added to the group.
         */
        bool add_data_frag(
                const CacheChange_t& change,
                const uint32_t fragment_number,
                bool expects_inline_qos);

        /**
         * Adds a HEARTBEAT message to the group.
         * @param first_seq First available sequence number.
         * @param last_seq Last available sequence number.
         * @param count Counting identifier.
         * @param is_final Should final flag be set?
         * @param liveliness_flag Should liveliness flag be set?
         * @return True when message was added to the group.
         */
        bool add_heartbeat(
                const SequenceNumber_t& first_seq,
                const SequenceNumber_t& last_seq,
                Count_t count,
                bool is_final,
                bool liveliness_flag);

        /**
         * Adds a GAP message to the group.
         * @param changes_seq_numbers Set of missed sequence numbers.
         * @return True when message was added to the group.
         */
        bool add_gap(std::set<SequenceNumber_t>& changes_seq_numbers);

        /**
         * Adds a ACKNACK message to the group.
         * @param seq_num_set Set of missing sequence numbers.
         * @param count Counting identifier.
         * @param final_flag Should final flag be set?
         * @return True when message was added to the group.
         */
        bool add_acknack(
                const SequenceNumberSet_t& seq_num_set,
                int32_t count,
                bool final_flag);

        /**
         * Adds a NACKFRAG message to the group.
         * @param seq_number Sequence number being nack'ed.
         * @param fn_state Set of missing fragment numbers.
         * @param count Counting identifier.
         * @return True when message was added to the group.
         */
        bool add_nackfrag(
                const SequenceNumber_t& seq_number,
                FragmentNumberSet_t fn_state,
                int32_t count);

        uint32_t get_current_bytes_processed() { return currentBytesSent_ + full_msg_->length; }

        /**
         * To be used whenever destination locators/guids change between two add_xxx calls.
         * Automatically called inside add_xxx calls if destinations_have_changed() method of
         * RTPSMessageSenderInterface returns true.
         * May become private again with a refactor of RTPSMessageSenderInterface, adding a
         * group_has_been_flushed() method.
         */
        void flush_and_reset();

    private:

        void reset_to_header();

        void flush();

        void send();

        void check_and_maybe_flush();

        bool insert_submessage();

        bool add_info_dst_in_buffer(CDRMessage_t* buffer);

        bool add_info_ts_in_buffer(const Time_t& timestamp);

        const RTPSMessageSenderInterface& sender_;
            
        Endpoint* endpoint_;

        CDRMessage_t* full_msg_;

        CDRMessage_t* submessage_msg_;

        uint32_t currentBytesSent_;

        GuidPrefix_t current_dst_;

#if HAVE_SECURITY
        RTPSParticipantImpl* participant_;

        CDRMessage_t* encrypt_msg_;
#endif

        std::chrono::steady_clock::time_point max_blocking_time_point_;

};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif
#endif /* RTPSMESSAGEGROUP_H_ */
