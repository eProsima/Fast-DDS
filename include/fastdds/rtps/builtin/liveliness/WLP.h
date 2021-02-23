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
 * @file WLP.h
 *
 */

#ifndef _FASTDDS_RTPS_WLP_H_
#define _FASTDDS_RTPS_WLP_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

#include <vector>
#include <mutex>

#include <fastdds/rtps/common/Time_t.h>
#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/common/Guid.h>
#include <fastrtps/qos/QosPolicies.h>

#include <fastdds/rtps/builtin/data/WriterProxyData.h>
#include <fastdds/rtps/builtin/data/ReaderProxyData.h>

namespace eprosima {
namespace fastrtps {
namespace rtps {

class BuiltinProtocols;
class LivelinessManager;
class ReaderHistory;
class ReaderProxyData;
class RTPSParticipantImpl;
class RTPSReader;
class RTPSWriter;
class StatefulReader;
class StatefulWriter;
class ParticipantProxyData;
class TimedEvent;
class WLPListener;
class WriterHistory;
class WriterProxyData;
class ITopicPayloadPool;

/**
 * Class WLP that implements the Writer Liveliness Protocol described in the RTPS specification.
 * @ingroup LIVELINESS_MODULE
 */
class WLP
{
    friend class WLPListener;
    friend class StatefulReader;
    friend class StatelessReader;

public:

    /**
     * Constructor
     * @param prot Pointer to the BuiltinProtocols object.
     */
    WLP(
            BuiltinProtocols* prot);
    virtual ~WLP();
    /**
     * Initialize the WLP protocol.
     * @param p Pointer to the RTPS participant implementation.
     * @return true if the initialziacion was successful.
     */
    bool initWL(
            RTPSParticipantImpl* p);
    /**
     * Assign the remote endpoints for a newly discovered RTPSParticipant.
     * @param pdata Pointer to the RTPSParticipantProxyData object.
     * @return True if correct.
     */
    bool assignRemoteEndpoints(
            const ParticipantProxyData& pdata);
    /**
     * Remove remote endpoints from the liveliness protocol.
     * @param pdata Pointer to the ParticipantProxyData to remove
     */
    void removeRemoteEndpoints(
            ParticipantProxyData* pdata);
    /**
     * Add a local writer to the liveliness protocol.
     * @param W Pointer to the RTPSWriter.
     * @param wqos Quality of service policies for the writer.
     * @return True if correct.
     */
    bool add_local_writer(
            RTPSWriter* W,
            const WriterQos& wqos);
    /**
     * Remove a local writer from the liveliness protocol.
     * @param W Pointer to the RTPSWriter.
     * @return True if removed.
     */
    bool remove_local_writer(
            RTPSWriter* W);

    /**
     * @brief Adds a local reader to the liveliness protocol
     * @param reader Pointer to the RTPS reader
     * @param rqos Quality of service policies for the reader
     * @return True if added successfully
     */
    bool add_local_reader(
            RTPSReader* reader,
            const ReaderQos& rqos);

    /**
     * @brief Removes a local reader from the livliness protocol
     * @param reader Pointer to the reader to remove
     * @return True if removed successfully
     */
    bool remove_local_reader(
            RTPSReader* reader);

    /**
     * @brief A method to assert liveliness of a given writer
     * @param writer The writer, specified via its id
     * @param kind The writer liveliness kind
     * @param lease_duration The writer lease duration
     * @return True if liveliness was asserted
     */
    bool assert_liveliness(
            GUID_t writer,
            LivelinessQosPolicyKind kind,
            Duration_t lease_duration);

    /**
     * @brief A method to assert liveliness of MANUAL_BY_PARTICIPANT writers
     * @return True if there were any MANUAL_BY_PARTICIPANT writers
     */
    bool assert_liveliness_manual_by_participant();

    /**
     * Get the livelines builtin writer
     * @return stateful writer
     */
    StatefulWriter* builtin_writer();

    /**
     * Get the livelines builtin writer's history
     * @return writer history
     */
    WriterHistory* builtin_writer_history();

#if HAVE_SECURITY
    bool pairing_remote_reader_with_local_writer_after_security(
            const GUID_t& local_writer,
            const ReaderProxyData& remote_reader_data);

    bool pairing_remote_writer_with_local_reader_after_security(
            const GUID_t& local_reader,
            const WriterProxyData& remote_writer_data);
#endif // if HAVE_SECURITY

private:

    /**
     * Create the endpoints used in the WLP.
     * @return true if correct.
     */
    bool createEndpoints();

    //! Minimum time among liveliness periods of automatic writers, in milliseconds
    double min_automatic_ms_;
    //! Minimum time among liveliness periods of manual by participant writers, in milliseconds
    double min_manual_by_participant_ms_;
    //!Pointer to the local RTPSParticipant.
    RTPSParticipantImpl* mp_participant;
    //!Pointer to the builtinprotocol class.
    BuiltinProtocols* mp_builtinProtocols;
    //!Pointer to the builtinRTPSParticipantMEssageWriter.
    StatefulWriter* mp_builtinWriter;
    //!Pointer to the builtinRTPSParticipantMEssageReader.
    StatefulReader* mp_builtinReader;
    //!Writer History
    WriterHistory* mp_builtinWriterHistory;
    //!Reader History
    ReaderHistory* mp_builtinReaderHistory;
    //!Listener object.
    WLPListener* mp_listener;
    //!Pointer to the periodic assertion timer object for automatic liveliness writers
    TimedEvent* automatic_liveliness_assertion_;
    //!Pointer to the periodic assertion timer object for manual by participant liveliness writers
    TimedEvent* manual_liveliness_assertion_;
    //! List of the writers using automatic liveliness.
    std::vector<RTPSWriter*> automatic_writers_;
    //! List of the writers using manual by participant liveliness.
    std::vector<RTPSWriter*> manual_by_participant_writers_;
    //! List of writers using manual by topic liveliness
    std::vector<RTPSWriter*> manual_by_topic_writers_;

    //! List of readers
    std::vector<RTPSReader*> readers_;
    //! A boolean indicating that there is at least one reader requesting automatic liveliness
    bool automatic_readers_;

    //! A class used by writers in this participant to keep track of their liveliness
    LivelinessManager* pub_liveliness_manager_;
    //! A class used by readers in this participant to keep track of liveliness of matched writers
    LivelinessManager* sub_liveliness_manager_;

    InstanceHandle_t automatic_instance_handle_;
    InstanceHandle_t manual_by_participant_instance_handle_;

    /**
     * @brief A method invoked by pub_liveliness_manager_ to inform that a writer changed its liveliness
     * @param writer The writer losing liveliness
     * @param kind The liveliness kind
     * @param lease_duration The liveliness lease duration
     * @param alive_change The change in the alive count
     * @param not_alive_change The change in the not alive count
     */
    void pub_liveliness_changed(
            const GUID_t& writer,
            const LivelinessQosPolicyKind& kind,
            const Duration_t& lease_duration,
            int32_t alive_change,
            int32_t not_alive_change);

    /**
     * @brief A method invoked by sub_liveliness_manager_ to inform that a writer changed its liveliness
     * @param writer The writer losing liveliness
     * @param kind The liveliness kind of the writer losing liveliness
     * @param lease_duration The liveliness lease duration of the writer losing liveliness
     * @param alive_change The change in the alive count
     * @param not_alive_change The change in the not alive count
     */
    void sub_liveliness_changed(
            const GUID_t& writer,
            const LivelinessQosPolicyKind& kind,
            const Duration_t& lease_duration,
            int32_t alive_change,
            int32_t not_alive_change);

    /**
     * @brief A method to update the liveliness changed status of a given reader
     * @param writer The writer changing liveliness, specified by its guid
     * @param reader The reader whose liveliness needs to be updated
     * @param alive_change The change requested for alive count. Should be -1, 0 or +1
     * @param not_alive_change The change requested for not alive count. Should be -1, 0 or +1
     */
    void update_liveliness_changed_status(
            GUID_t writer,
            RTPSReader* reader,
            int32_t alive_change,
            int32_t not_alive_change);

    /**
     * Implements the automatic liveliness timed event
     */
    bool automatic_liveliness_assertion();

    /**
     * Implements the manual by participant liveliness timed event
     */
    bool participant_liveliness_assertion();

    /**
     * Adds a cache change to the WLP writer
     * @param instance key of the change to add
     * @return true if change is correctly added
     */
    bool send_liveliness_message(
            const InstanceHandle_t& instance);

#if HAVE_SECURITY
    //!Pointer to the builtinRTPSParticipantMEssageWriter.
    StatefulWriter* mp_builtinWriterSecure;
    //!Pointer to the builtinRTPSParticipantMEssageReader.
    StatefulReader* mp_builtinReaderSecure;
    //!Writer History
    WriterHistory* mp_builtinWriterSecureHistory;
    //!Reader History
    ReaderHistory* mp_builtinReaderSecureHistory;

    /**
     * Create the secure endpoitns used in the WLP.
     * @return true if correct.
     */
    bool createSecureEndpoints();
#endif // if HAVE_SECURITY

    std::mutex temp_data_lock_;
    ReaderProxyData temp_reader_proxy_data_;
    WriterProxyData temp_writer_proxy_data_;

    std::shared_ptr<ITopicPayloadPool> payload_pool_;
#if HAVE_SECURITY
    std::shared_ptr<ITopicPayloadPool> secure_payload_pool_;
#endif // if HAVE_SECURITY
};

} /* namespace rtps */
} /* namespace fastrtps */
} /* namespace eprosima */

#endif // ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#endif /* _FASTDDS_RTPS_WLP_H_ */
