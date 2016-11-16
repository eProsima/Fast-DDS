#include "SecurityManager.h"

#include <fastrtps/rtps/security/authentication/Authentication.h>
#include <fastrtps/log/Log.h>
#include <rtps/participant/RTPSParticipantImpl.h>

#include <fastrtps/rtps/writer/StatelessWriter.h>
#include <fastrtps/rtps/reader/StatelessReader.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>

#include <cassert>
#include <thread>

#define ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_WRITER  0x000201C2
#define ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_READER  0x000201C7

#define AUTHENTICATION_PARTICIPANT_STATELESS_MESSAGE "dds.sec.auth"

// TODO(Ricardo) Add event because stateless messages can be not received.
// TODO(Ricardo) Add following of sequence in stateless messages.

const EntityId_t participant_stateless_message_writer_entity_id = ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_WRITER;
const EntityId_t participant_stateless_message_reader_entity_id = ENTITYID_P2P_BUILTIN_PARTICIPANT_STATELESS_READER;

using namespace eprosima::fastrtps;
using namespace ::rtps;
using namespace ::security;

bool usleep_bool()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return true;
}

SecurityManager::SecurityManager(RTPSParticipantImpl *participant) : 
    participant_stateless_message_listener_(*this),
    participant_(participant),
    participant_stateless_message_writer_(nullptr),
    participant_stateless_message_writer_history_(nullptr),
    participant_stateless_message_reader_(nullptr),
    participant_stateless_message_reader_history_(nullptr),
    authentication_plugin_(nullptr)
{
    assert(participant != nullptr);
}

SecurityManager::~SecurityManager()
{
    if(authentication_plugin_ != nullptr)
    {
        SecurityException exception;

        for(auto& dp_it : discovered_participants_)
        {
            if(!dp_it.second.is_handshake_handle_null())
            {
                HandshakeHandle* handle = dp_it.second.get_handshake_handle();
                authentication_plugin_->return_handshake_handle(handle, exception);
            }

            if(!dp_it.second.is_identity_handle_null())
            {
                IdentityHandle* handle = dp_it.second.get_identity_handle();
                authentication_plugin_->return_identity_handle(handle, exception);
            }
        }

        if(local_identity_handle_ != nullptr)
            authentication_plugin_->return_identity_handle(local_identity_handle_, exception);

        delete_entities();

        delete authentication_plugin_;
    }
}

bool SecurityManager::init()
{
    SecurityException exception;

    authentication_plugin_ = factory_.create_authentication_plugin(participant_->getRTPSParticipantAttributes().properties);

    if(authentication_plugin_ != nullptr)
    {
        // Validate local participant
        GUID_t adjusted_participant_key;
        ValidationResult_t ret = VALIDATION_FAILED;
        
        do
        {
            ret = authentication_plugin_->validate_local_identity(&local_identity_handle_,
                adjusted_participant_key,
                participant_->getRTPSParticipantAttributes().builtin.domainId,
                participant_->getRTPSParticipantAttributes(),
                participant_->getGuid(),
                exception);
        } while(ret == VALIDATION_PENDING_RETRY && usleep_bool());

        if(ret == VALIDATION_OK)
        {
            assert(local_identity_handle_ != nullptr);

            // Create RTPS entities
            if(create_entities())
            {
                return true;
            }
        }

        delete authentication_plugin_;
        authentication_plugin_ = nullptr;
    }
    else
    {
        logInfo(SECURITY, "Authentication plugin not configured. Security will be disable");
    }

    return false;
}

void SecurityManager::remove_discovered_participant_info(const GUID_t remote_participant_key)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto dp_it = discovered_participants_.find(remote_participant_key);

    if(dp_it != discovered_participants_.end())
    {
        IdentityHandle* identity_handle = dp_it->second.get_identity_handle();

        if(identity_handle != nullptr)
        {
            SecurityException exception;
            authentication_plugin_->return_identity_handle(identity_handle, exception);
        }

        discovered_participants_.erase(dp_it);
    }
}

void SecurityManager::restore_remote_identity_handle(const GUID_t& remote_participant_key,
        IdentityHandle* remote_identity_handle,
        HandshakeHandle* handshake_handle)
{
    SecurityException exception;

    std::unique_lock<std::mutex> lock(mutex_);
    auto dp_it = discovered_participants_.find(remote_participant_key);

    if(dp_it != discovered_participants_.end())
    {
        assert(dp_it->second.is_identity_handle_null());
        dp_it->second.set_identity_handle(remote_identity_handle);
        assert(dp_it->second.is_handshake_handle_null());
        dp_it->second.set_handshake_handle(handshake_handle);
    }
    else
    {
        authentication_plugin_->return_identity_handle(remote_identity_handle, exception);

        if(handshake_handle)
            authentication_plugin_->return_handshake_handle(handshake_handle, exception);
    }
}

bool SecurityManager::discovered_participant(IdentityToken&& remote_identity_token,
        const GUID_t& remote_participant_key)
{
    if(authentication_plugin_ == nullptr)
        return true;

    IdentityHandle* remote_identity_handle = nullptr;
    SecurityException exception;
    AuthenticationStatus auth_status = AUTHENTICATION_INIT;

    // Find information
    mutex_.lock();
    auto dp_it = discovered_participants_.find(remote_participant_key);

    if(dp_it == discovered_participants_.end())
    {
        discovered_participants_.emplace(remote_participant_key, auth_status);

        mutex_.unlock();

        // Validate remote participant.
        ValidationResult_t validation_ret = authentication_plugin_->validate_remote_identity(&remote_identity_handle,
                *local_identity_handle_, std::move(remote_identity_token),
                remote_participant_key, exception);

        switch(validation_ret)
        {
            case VALIDATION_OK:
                assert(remote_identity_handle != nullptr);
                auth_status = AUTHENTICATION_OK;
                break;
            case VALIDATION_PENDING_HANDSHAKE_REQUEST:
                assert(remote_identity_handle != nullptr);
                auth_status = AUTHENTICATION_REQUEST_NOT_SEND;
                break;
            case VALIDATION_PENDING_HANDSHAKE_MESSAGE:
                assert(remote_identity_handle != nullptr);
                auth_status = AUTHENTICATION_WAITING_REQUEST;
                break;
            case VALIDATION_PENDING_RETRY:
                // TODO(Ricardo) Send event.
            default:
                remove_discovered_participant_info(remote_participant_key);
                return false;
        };

        // Store remote handle.
        mutex_.lock();
        dp_it = discovered_participants_.find(remote_participant_key);
        if(dp_it != discovered_participants_.end())
        {
            dp_it->second.set_auth_status(auth_status);
            bool ret = dp_it->second.set_identity_handle(remote_identity_handle);
            (void)ret; assert(ret);
            remote_identity_handle = nullptr;
        }
        else
        {
            mutex_.unlock();
            authentication_plugin_->return_identity_handle(remote_identity_handle, exception); // TODO(Ricardo) Check error.
            return false;
        }
    }
    else
    {
        auth_status = dp_it->second.get_auth_status();

        if(auth_status == AUTHENTICATION_INIT)
            return false;
    }

    int64_t last_sequence_number = 0;

    if(auth_status == AUTHENTICATION_REQUEST_NOT_SEND)
    {
        remote_identity_handle = dp_it->second.get_identity_handle();
        assert(remote_identity_handle);
        last_sequence_number = dp_it->second.get_last_sequence_number();

    }
    mutex_.unlock();

    bool returnedValue = true;

    // Maybe send request.
    if(remote_identity_handle != nullptr)
    {
        returnedValue = on_process_handshake(remote_participant_key, AUTHENTICATION_REQUEST_NOT_SEND,
                MessageIdentity(), HandshakeMessageToken(),
                remote_identity_handle, nullptr, last_sequence_number);
    }

    return returnedValue;
}

bool SecurityManager::on_process_handshake(const GUID_t& remote_participant_key,
        AuthenticationStatus pre_auth_status,
        MessageIdentity&& message_identity,
        HandshakeMessageToken&& message_in,
        IdentityHandle* remote_identity_handle,
        HandshakeHandle* handshake_handle, int64_t last_sequence_number)
{
    assert(remote_identity_handle);

    HandshakeMessageToken* handshake_message = nullptr;
    SecurityException exception;
    
    ValidationResult_t ret = VALIDATION_FAILED;
    
    if(pre_auth_status == AUTHENTICATION_REQUEST_NOT_SEND)
    {
        ret = authentication_plugin_->begin_handshake_request(&handshake_handle,
                &handshake_message,
                *local_identity_handle_,
                *remote_identity_handle,
                exception);
    }
    else if(pre_auth_status == AUTHENTICATION_WAITING_REQUEST)
    {
        assert(!handshake_handle);
        ret = authentication_plugin_->begin_handshake_reply(&handshake_handle,
                &handshake_message,
                std::move(message_in),
                *remote_identity_handle,
                *local_identity_handle_,
                exception);
    }
    else if(pre_auth_status == AUTHENTICATION_WAITING_REPLY ||
            pre_auth_status == AUTHENTICATION_WAITING_FINAL)
    {
        assert(handshake_handle);
        ret = authentication_plugin_->process_handshake(&handshake_message,
                std::move(message_in),
                *handshake_handle, 
                exception);
    }

    if(ret == VALIDATION_FAILED)
    {
        restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle);
        return false;
    }

    assert(handshake_handle);

    bool handshake_message_send = true;

    if(ret == VALIDATION_PENDING_HANDSHAKE_MESSAGE ||
            ret == VALIDATION_OK_WITH_FINAL_MESSAGE)
    {
        handshake_message_send = false;

        assert(handshake_message);

        // Send hanshake message

        // Create message
        ParticipantGenericMessage message = generate_authentication_message(last_sequence_number,
                std::move(message_identity), remote_participant_key, std::move(*handshake_message));

        CacheChange_t* change = participant_stateless_message_writer_->new_change([&message]() -> uint32_t
                {
                return ParticipantGenericMessageHelper::serialized_size(message);
                }
                , ALIVE, c_InstanceHandle_Unknown);

        if(change != nullptr)
        {
            // Serialize message
            CDRMessage_t aux_msg(0);
            aux_msg.wraps = true;
            aux_msg.buffer = change->serializedPayload.data;
            aux_msg.length = change->serializedPayload.length;
            aux_msg.max_size = change->serializedPayload.max_size;
            aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;

            if(CDRMessage::addParticipantGenericMessage(&aux_msg, message))
            {
                // Send
                if(participant_stateless_message_writer_history_->add_change(change))
                {
                    handshake_message_send = true;
                }
                else
                {
                    logError(SECURITY, "WriterHistory cannot add the CacheChange_t");
                }
            }
            else
            {
                logError(SECURITY, "Cannot serialize ParticipantGenericMessage");
            }
        }
        else
        {
            logError(SECURITY, "WriterHistory cannot retrieve a CacheChange_t");
        }
    }

    bool returnedValue = false;

    if(handshake_message_send)
    {
        AuthenticationStatus auth_status = AUTHENTICATION_FAILED;

        switch(ret)
        {
            case VALIDATION_OK:
            case VALIDATION_PENDING_HANDSHAKE_MESSAGE:
            case VALIDATION_OK_WITH_FINAL_MESSAGE:
                {
                    auth_status = AUTHENTICATION_OK;
                    if(ret == VALIDATION_PENDING_HANDSHAKE_MESSAGE)
                    {
                        if(pre_auth_status == AUTHENTICATION_REQUEST_NOT_SEND)
                            auth_status = AUTHENTICATION_WAITING_REPLY;
                        else if(pre_auth_status == AUTHENTICATION_WAITING_REQUEST)
                            auth_status = AUTHENTICATION_WAITING_FINAL;
                    }

                    // Store status
                    std::unique_lock<std::mutex> lock(mutex_);

                    auto dp_it = discovered_participants_.find(remote_participant_key);

                    if(dp_it != discovered_participants_.end())
                    {
                        assert(dp_it->second.get_auth_status() == pre_auth_status);
                        dp_it->second.set_auth_status(auth_status);
                        assert(dp_it->second.is_identity_handle_null());
                        dp_it->second.set_identity_handle(remote_identity_handle);
                        remote_identity_handle = nullptr;
                        assert(dp_it->second.is_handshake_handle_null());
                        dp_it->second.set_handshake_handle(handshake_handle);
                        handshake_handle = nullptr;
                        if(ret == VALIDATION_PENDING_HANDSHAKE_MESSAGE ||
                                ret == VALIDATION_OK_WITH_FINAL_MESSAGE)
                            dp_it->second.set_last_sequence_number(++last_sequence_number);

                        returnedValue = true;
                    }
                    else
                    {
                        authentication_plugin_->return_handshake_handle(handshake_handle, exception);
                        handshake_handle = nullptr;
                        authentication_plugin_->return_identity_handle(remote_identity_handle, exception);
                        remote_identity_handle = nullptr;
                    }
                }
                break;
            case VALIDATION_PENDING_RETRY:
                // TODO(Ricardo) Send event.
            default:
                break;
        };
    }

    if(handshake_handle != nullptr && (pre_auth_status == AUTHENTICATION_REQUEST_NOT_SEND ||
                pre_auth_status == AUTHENTICATION_WAITING_REQUEST))
    {
        authentication_plugin_->return_handshake_handle(handshake_handle, exception);
        handshake_handle = nullptr;
    }
    if(remote_identity_handle != nullptr)
        restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle);

    delete handshake_message;

    return returnedValue;
}

bool SecurityManager::create_entities()
{
    if(create_participant_stateless_message_entities())
    {
        return true;
    }

    return false; 
}

void SecurityManager::delete_entities()
{
    delete_participant_stateless_message_entities();
}

bool SecurityManager::create_participant_stateless_message_entities()
{
    if(create_participant_stateless_message_writer())
    {
        if(create_participant_stateless_message_reader())
        {
            return true;
        }

        delete_participant_stateless_message_writer();
    }

    return false;
}

void SecurityManager::delete_participant_stateless_message_entities()
{
    delete_participant_stateless_message_reader();
    delete_participant_stateless_message_writer();
}

bool SecurityManager::create_participant_stateless_message_writer()
{
    HistoryAttributes hatt;
    hatt.payloadMaxSize = 5000;
    hatt.initialReservedCaches = 20;
    hatt.maximumReservedCaches = 100;
    participant_stateless_message_writer_history_ = new WriterHistory(hatt);
    WriterAttributes watt;
    watt.endpoint.endpointKind = WRITER;
    watt.endpoint.reliabilityKind = BEST_EFFORT;
    watt.endpoint.topicKind = NO_KEY;

    if(participant_->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
            participant_->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
        watt.mode = ASYNCHRONOUS_WRITER;

    RTPSWriter* wout = nullptr;
    if(participant_->createWriter(&wout, watt, participant_stateless_message_writer_history_, nullptr, participant_stateless_message_writer_entity_id, true))
    {
        participant_stateless_message_writer_ = dynamic_cast<StatelessWriter*>(wout);
        auth_source_guid = participant_stateless_message_writer_->getGuid();

        return true;
    }

    logError(SECURITY,"Participant Stateless Message Writer creation failed");
    delete(participant_stateless_message_writer_history_);
    participant_stateless_message_writer_history_ = nullptr;

    return false;
}

void SecurityManager::delete_participant_stateless_message_writer()
{
    if(participant_stateless_message_writer_ != nullptr)
    {
        delete participant_stateless_message_writer_;
        participant_stateless_message_writer_ = nullptr;
    }

    if(participant_stateless_message_writer_history_ != nullptr)
    {
        delete participant_stateless_message_writer_history_;
        participant_stateless_message_writer_history_ = nullptr;
    }
}

bool SecurityManager::create_participant_stateless_message_reader()
{
    HistoryAttributes hatt;
    hatt.payloadMaxSize = 5000;
    hatt.initialReservedCaches = 250;
    hatt.maximumReservedCaches = 5000;
    participant_stateless_message_reader_history_ = new ReaderHistory(hatt);
    ReaderAttributes ratt;
    ratt.endpoint.topicKind = NO_KEY;
    ratt.endpoint.reliabilityKind = BEST_EFFORT;

    RTPSReader* rout = nullptr;
    if(participant_->createReader(&rout, ratt, participant_stateless_message_reader_history_, &participant_stateless_message_listener_,
                participant_stateless_message_reader_entity_id, true, false))
    {
        participant_stateless_message_reader_ = dynamic_cast<StatelessReader*>(rout);

        return true;
    }

    logError(SECURITY,"Participant Stateless Message Reader creation failed");
    delete(participant_stateless_message_reader_history_);
    participant_stateless_message_reader_history_ = nullptr;
    return false;
}

void SecurityManager::delete_participant_stateless_message_reader()
{
    if(participant_stateless_message_reader_ != nullptr)
    {
        delete participant_stateless_message_reader_;
        participant_stateless_message_reader_ = nullptr;
    }

    if(participant_stateless_message_reader_history_ != nullptr)
    {
        delete participant_stateless_message_reader_history_;
        participant_stateless_message_reader_history_ = nullptr;
    }
}

ParticipantGenericMessage SecurityManager::generate_authentication_message(int64_t sequence_number,
        const MessageIdentity& related_message_identity,
        const GUID_t& destination_participant_key,
        HandshakeMessageToken&& handshake_message)
{
    ParticipantGenericMessage message;

    message.message_identity().source_guid(auth_source_guid);
    message.message_identity().sequence_number(sequence_number);
    message.related_message_identity(related_message_identity);
    message.destination_participant_key(destination_participant_key);
    message.message_class_id(AUTHENTICATION_PARTICIPANT_STATELESS_MESSAGE);
    message.message_data().push_back(std::move(handshake_message));

    return message;
}

void SecurityManager::process_participant_stateless_message(const CacheChange_t* const change)
{
    assert(change);

    // Deserialize message
    ParticipantGenericMessage message;
    CDRMessage_t aux_msg(0);
    aux_msg.wraps = true;
    aux_msg.buffer = change->serializedPayload.data;
    aux_msg.length = change->serializedPayload.length;
    aux_msg.max_size = change->serializedPayload.max_size;
    aux_msg.msg_endian = change->serializedPayload.encapsulation == PL_CDR_BE ? BIGEND : LITTLEEND;

    CDRMessage::readParticipantGenericMessage(&aux_msg, message);

    if(message.message_class_id().compare(AUTHENTICATION_PARTICIPANT_STATELESS_MESSAGE) == 0)
    {
        if(message.destination_endpoint_key() != GUID_t::unknown())
        {
            logInfo(SECURITY, "Bad ParticipantGenericMessage. destination_endpoint_key is not GUID_t::unknown()");
            return;
        }
        if(message.source_endpoint_key() != GUID_t::unknown())
        {
            logInfo(SECURITY, "Bad ParticipantGenericMessage. source_endpoint_key is not GUID_t::unknown()");
            return;
        }

        const GUID_t& remote_participant_key = message.destination_participant_key();
        AuthenticationStatus auth_status;
        IdentityHandle* remote_identity_handle = nullptr;
        HandshakeHandle* handshake_handle = nullptr;
        int64_t last_sequence_number = 0;

        mutex_.lock();
        auto dp_it = discovered_participants_.find(remote_participant_key);

        if(dp_it != discovered_participants_.end())
        {
            auth_status = dp_it->second.get_auth_status();
            remote_identity_handle = dp_it->second.get_identity_handle();
            handshake_handle = dp_it->second.get_handshake_handle();
            last_sequence_number = dp_it->second.get_last_sequence_number();
        }
        else
        {
            logInfo(SECURITY, "Received Authentication message but not found related remote_participant_key");
        }
        mutex_.unlock();

        if(remote_identity_handle != nullptr)
        {
            if(auth_status == AUTHENTICATION_WAITING_REQUEST)
            {
                assert(!handshake_handle);

                // Preconditions
                if(message.message_identity().source_guid() == GUID_t::unknown())
                {
                    logInfo(SECURITY, "Bad ParticipantGenericMessage. message_identity.source_guid is GUID_t::unknown()");
                    restore_remote_identity_handle(remote_participant_key, remote_identity_handle);
                    return;
                }
                if(message.related_message_identity().source_guid() != GUID_t::unknown())
                {
                    logInfo(SECURITY, "Bad ParticipantGenericMessage. related_message_identity.source_guid is not GUID_t::unknown()");
                    restore_remote_identity_handle(remote_participant_key, remote_identity_handle);
                    return;
                }
                if(message.message_data().size() != 1)
                {
                    logInfo(SECURITY, "Bad ParticipantGenericMessage. message_data size is not 1");
                    restore_remote_identity_handle(remote_participant_key, remote_identity_handle);
                    return;
                }
            }
            else if(auth_status == AUTHENTICATION_WAITING_REPLY ||
                    auth_status == AUTHENTICATION_WAITING_FINAL)
            {
                assert(handshake_handle);

                // Preconditions
                if(message.message_identity().source_guid() == GUID_t::unknown())
                {
                    logInfo(SECURITY, "Bad ParticipantGenericMessage. message_identity.source_guid is GUID_t::unknown()");
                    restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle);
                    return;
                }
                if(message.related_message_identity().source_guid() == GUID_t::unknown())
                {
                    logInfo(SECURITY, "Bad ParticipantGenericMessage. related_message_identity.source_guid is GUID_t::unknown()");
                    restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle);
                    return;
                }
                if(message.message_data().size() != 1)
                {
                    logInfo(SECURITY, "Bad ParticipantGenericMessage. message_data size is not 1");
                    restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle);
                    return;
                }
            }
            else
            {
                restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle);
                return;
            }

            on_process_handshake(remote_participant_key, auth_status, std::move(message.related_message_identity()),
                    std::move(message.message_data().at(0)), remote_identity_handle,
                    handshake_handle, last_sequence_number);
        }
    }
    else
    {
        logInfo(SECURITY, "Discarted ParticipantGenericMessage with class id " << message.message_class_id());
    }
}

void SecurityManager::ParticipantStatelessMessageListener::onNewCacheChangeAdded(RTPSReader* reader,
        const CacheChange_t* const change)
{
    manager_.process_participant_stateless_message(change);

    ReaderHistory *history = reader->getHistory();
    assert(history);
    history->remove_change((CacheChange_t*)change);
}
