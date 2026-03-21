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
 * @file RTPSParticipantListener.hpp
 */

#ifndef FASTDDS_RTPS_PARTICIPANT__RTPSPARTICIPANTLISTENER_HPP
#define FASTDDS_RTPS_PARTICIPANT__RTPSPARTICIPANTLISTENER_HPP

#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/ParticipantBuiltinTopicData.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.hpp>
#include <fastdds/rtps/reader/ReaderDiscoveryStatus.hpp>
#include <fastdds/rtps/writer/WriterDiscoveryStatus.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSParticipant;

/**
 * Class RTPSParticipantListener with virtual method that the user can overload to respond to certain events.
 * @ingroup RTPS_MODULE
 */
class FASTDDS_EXPORTED_API RTPSParticipantListener
{
public:

    RTPSParticipantListener()
    {
    }

    virtual ~RTPSParticipantListener()
    {
    }

    /*!
     * This method is called when a new Participant is discovered, or a previously discovered participant changes
     * its QOS or is removed.
     *
     * @param [out] participant Pointer to the Participant which discovered the remote participant.
     * @param [out] reason Reason of the change in the status of the discovered participant.
     * @param [out] info Remote participant information. User can take ownership of the object.
     * @param [out] should_be_ignored Flag to indicate the library to automatically ignore the discovered Participant.
     */
    virtual void on_participant_discovery(
            RTPSParticipant* participant,
            ParticipantDiscoveryStatus reason,
            const ParticipantBuiltinTopicData& info,
            bool& should_be_ignored)
    {
        static_cast<void>(participant);
        static_cast<void>(reason);
        static_cast<void>(info);

        should_be_ignored = false;
    }

#if HAVE_SECURITY
    virtual void onParticipantAuthentication(
            RTPSParticipant* participant,
            ParticipantAuthenticationInfo&& info)
    {
        static_cast<void>(participant);
        static_cast<void>(info);
    }

#endif // if HAVE_SECURITY

    /*!
     * This method is called when a new Reader is discovered, or a previously discovered reader changes
     * its QOS or is removed.
     *
     * @param [in]  participant        Pointer to the Participant which discovered the remote reader.
     * @param [in]  reason             The reason motivating this method to be called.
     * @param [in]  info               Remote reader information.
     * @param [out] should_be_ignored  Flag to indicate the library to automatically ignore the discovered reader.
     */
    virtual void on_reader_discovery(
            RTPSParticipant* participant,
            ReaderDiscoveryStatus reason,
            const SubscriptionBuiltinTopicData& info,
            bool& should_be_ignored)
    {
        static_cast<void>(participant);
        static_cast<void>(reason);
        static_cast<void>(info);
        should_be_ignored = false;
    }

    /*!
     * This method is called when a new Writer is discovered, or a previously discovered writer changes
     * its QOS or is removed.
     *
     * @param [in]  participant        Pointer to the Participant which discovered the remote writer.
     * @param [in]  reason             The reason motivating this method to be called.
     * @param [in]  info               Remote writer information.
     * @param [out] should_be_ignored  Flag to indicate the library to automatically ignore the discovered writer.
     */
    virtual void on_writer_discovery(
            RTPSParticipant* participant,
            WriterDiscoveryStatus reason,
            const PublicationBuiltinTopicData& info,
            bool& should_be_ignored)
    {
        static_cast<void>(participant);
        static_cast<void>(reason);
        static_cast<void>(info);
        should_be_ignored = false;
    }

};

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

#endif //FASTDDS_RTPS_PARTICIPANT__RTPSPARTICIPANTLISTENER_HPP
