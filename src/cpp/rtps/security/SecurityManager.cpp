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

SecurityManager::SecurityManager(RTPSParticipantImpl *participant) : participant_(participant),
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
        delete authentication_plugin_;
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
        IdentityHandle* remote_identity_handle)
{
    SecurityException exception;

    std::unique_lock<std::mutex> lock(mutex_);
    auto dp_it = discovered_participants_.find(remote_participant_key);

    if(dp_it != discovered_participants_.end())
    {
        assert(dp_it->second.is_identity_handle_null());
        dp_it->second.set_identity_handle(remote_identity_handle);
    }
    else
        authentication_plugin_->return_identity_handle(remote_identity_handle, exception);
}

bool SecurityManager::discovered_participant(IdentityToken&& remote_identity_token,
        const GUID_t& remote_participant_key)
{
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

    if(auth_status == AUTHENTICATION_REQUEST_NOT_SEND)
    {
        remote_identity_handle = dp_it->second.get_identity_handle();
        assert(remote_identity_handle);
    }
    mutex_.unlock();

    bool returnedValue = true;

    // Maybe send request.
    if(remote_identity_handle != nullptr)
    {
        returnedValue = on_request_not_send(remote_participant_key, remote_identity_handle);
    }

    return returnedValue;
}

bool SecurityManager::on_request_not_send(const GUID_t& remote_participant_key,
        IdentityHandle* remote_identity_handle)
{
    assert(remote_identity_handle);

    HandshakeHandle* handshake_handle = nullptr;
    HandshakeMessageToken* handshake_message = nullptr;
    SecurityException exception;
    
    ValidationResult_t ret = authentication_plugin_->begin_handshake_request(&handshake_handle,
                &handshake_message,
                *local_identity_handle_,
                *remote_identity_handle,
                exception);

    if(ret == VALIDATION_FAILED)
    {
        restore_remote_identity_handle(remote_participant_key, remote_identity_handle);
        return false;
    }

    assert(handshake_handle);
    assert(handshake_message);

    // Send hanshake message
    // 
    delete handshake_message;

    AuthenticationStatus auth_status = AUTHENTICATION_FAILED;

    switch(ret)
    {
        case VALIDATION_OK:
            auth_status = AUTHENTICATION_OK;
            break;
        case VALIDATION_PENDING_HANDSHAKE_MESSAGE:
            auth_status = AUTHENTICATION_WAITING_REPLY;
            break;
        case VALIDATION_OK_WITH_FINAL_MESSAGE:
            auth_status = AUTHENTICATION_OK;
            break;
        case VALIDATION_PENDING_RETRY:
            // TODO(Ricardo) Send event.
        default:
            restore_remote_identity_handle(remote_participant_key, remote_identity_handle);
            return false;
    };

    // Store status
    std::unique_lock<std::mutex> lock(mutex_);

    auto dp_it = discovered_participants_.find(remote_participant_key);

    if(dp_it != discovered_participants_.end())
    {
        assert(dp_it->second.get_auth_status() == AUTHENTICATION_REQUEST_NOT_SEND);
        dp_it->second.set_auth_status(auth_status);
        assert(dp_it->second.is_identity_handle_null());
        dp_it->second.set_identity_handle(remote_identity_handle);
        assert(dp_it->second.is_handshake_handle_null());
        dp_it->second.set_handshake_handle(handshake_handle);
    }
    else
    {
        authentication_plugin_->return_handshake_handle(handshake_handle, exception);
        authentication_plugin_->return_identity_handle(remote_identity_handle, exception);
        return false;
    }

    return true;
}

bool SecurityManager::create_entities()
{
    if(create_participant_stateless_message_entities())
    {
        return true;
    }

    return false; 
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
        delete participant_stateless_message_writer_;

    if(participant_stateless_message_writer_history_ != nullptr)
        delete participant_stateless_message_writer_history_;
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
    //mp_listener = new PDPSimpleListener(this);

    RTPSReader* rout = nullptr;
    if(participant_->createReader(&rout, ratt, participant_stateless_message_reader_history_, nullptr, participant_stateless_message_reader_entity_id, true, false))
    {
        participant_stateless_message_reader_ = dynamic_cast<StatelessReader*>(rout);

        return true;
    }

    logError(SECURITY,"Participant Stateless Message Reader creation failed");
    delete(participant_stateless_message_reader_history_);
    participant_stateless_message_reader_history_ = nullptr;
    //delete(mp_listener);
    //mp_listener = nullptr;
    return false;
}

void SecurityManager::delete_participant_stateless_message_reader()
{
    if(participant_stateless_message_reader_ != nullptr)
        delete participant_stateless_message_reader_;

    if(participant_stateless_message_reader_history_ != nullptr)
        delete participant_stateless_message_reader_history_;
}
