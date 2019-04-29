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

#ifndef WLP_H_
#define WLP_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <vector>

#include "../../common/Time_t.h"
#include "../../common/Locator.h"
#include "../../common/Guid.h"

namespace eprosima {
namespace fastrtps{

class WriterQos;


namespace rtps {

class BuiltinProtocols;
class LivelinessManager;
class ReaderHistory;
class ReaderProxyData;
class RTPSParticipantImpl;
class RTPSWriter;
class StatefulReader;
class StatefulWriter;
class ParticipantProxyData;
class WLivelinessPeriodicAssertion;
class WLPListener;
class WriterHistory;
class WriterProxyData;

/**
 * Class WLP that implements the Writer Liveliness Protocol described in the RTPS specification.
 * @ingroup LIVELINESS_MODULE
 */
class WLP
{
	friend class WLPListener;
	friend class WLivelinessPeriodicAssertion;
public:
	/**
	* Constructor
	* @param prot Pointer to the BuiltinProtocols object.
	*/
	WLP(BuiltinProtocols* prot);
	virtual ~WLP();
	/**
	 * Initialize the WLP protocol.
	 * @param p Pointer to the RTPS participant implementation.
	 * @return true if the initialziacion was succesful.
	 */
	bool initWL(RTPSParticipantImpl* p);
	/**
	 * Create the endpoitns used in the WLP.
	 * @return true if correct.
	 */
	bool createEndpoints();
	/**
	 * Assign the remote endpoints for a newly discovered RTPSParticipant.
	 * @param pdata Pointer to the RTPSParticipantProxyData object.
	 * @return True if correct.
	 */
	bool assignRemoteEndpoints(const ParticipantProxyData& pdata);
	/**
	 * Remove remote endpoints from the liveliness protocol.
	 * @param pdata Pointer to the ParticipantProxyData to remove
	 */
	void removeRemoteEndpoints(ParticipantProxyData* pdata);
	/**
	 * Add a local writer to the liveliness protocol.
	 * @param W Pointer to the RTPSWriter.
	 * @param wqos Quality of service policies for the writer.
    * @return True if correct.
	 */
	bool addLocalWriter(RTPSWriter* W, const WriterQos& wqos);
	/**
	 * Remove a local writer from the liveliness protocol.
	 * @param W Pointer to the RTPSWriter.
	 * @return True if removed.
	 */
	bool removeLocalWriter(RTPSWriter* W);

    //! Minimum time among liveliness periods of automatic writers, in milliseconds
    double min_automatic_ms_;
    //! Minimum time among liveliness periods of manual by participant writers, in milliseconds
    double min_manual_by_participant_ms_;
	
	/**
	 * Get the builtin protocols
	 * @return Builtin protocols
	 */
	BuiltinProtocols* getBuiltinProtocols(){return mp_builtinProtocols;};
	
	/**
	 * Update local writer.
	 * @param W Writer to update
	 * @param wqos New writer QoS
	 * @return True on success
	 */
	bool updateLocalWriter(RTPSWriter* W, const WriterQos& wqos);
	
	/**
	 * Get the RTPS participant
	 * @return RTPS participant
	 */
	inline RTPSParticipantImpl* getRTPSParticipant(){return mp_participant;}

    /**
     * Get the livelines builtin writer
     * @return stateful writer
     */
    StatefulWriter* getBuiltinWriter();

    /**
    * Get the livelines builtin writer's history
    * @return writer history
    */
    WriterHistory* getBuiltinWriterHistory();
	
#if HAVE_SECURITY
    bool pairing_remote_reader_with_local_writer_after_security(const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data);

    bool pairing_remote_writer_with_local_reader_after_security(const GUID_t& local_reader,
        const WriterProxyData& remote_writer_data);
#endif

private:
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
    WLivelinessPeriodicAssertion* automatic_liveliness_assertion_;
    //!Pointer to the periodic assertion timer object for manual by participant liveliness writers
    WLivelinessPeriodicAssertion* manual_liveliness_assertion_;
    //! List of the writers using automatic liveliness.
    std::vector<RTPSWriter*> automatic_writers_;
    //! List of the writers using manual by participant liveliness.
    std::vector<RTPSWriter*> manual_by_participant_writers_;
    //! List of writers using manual by topic liveliness
    std::vector<RTPSWriter*> manual_by_topic_writers_;

    //! A class managing liveliness of writers in this participant
    LivelinessManager* liveliness_manager_;

    /**
     * @brief A method invoked by the liveliness manager to inform that a writer lost liveliness
     * @param writer The writer losing liveliness
     */
    void on_liveliness_lost(GUID_t writer);

    /**
     * @brief A method invoked by the liveliness manager to inform that a writer recovered liveliness
     * @param writer The writer losing liveliness
     */
    void on_livelienss_recovered(GUID_t writer);

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
#endif
};

}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* WLP_H_ */
