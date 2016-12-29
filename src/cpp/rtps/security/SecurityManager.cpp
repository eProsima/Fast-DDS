#include "SecurityManager.h"

#include <fastrtps/rtps/security/authentication/Authentication.h>
#include <fastrtps/log/Log.h>
#include <rtps/participant/RTPSParticipantImpl.h>
#include <fastrtps/rtps/participant/RTPSParticipantListener.h>

#include <fastrtps/rtps/writer/StatelessWriter.h>
#include <fastrtps/rtps/reader/StatelessReader.h>
#include <fastrtps/rtps/writer/StatefulWriter.h>
#include <fastrtps/rtps/reader/StatefulReader.h>
#include <fastrtps/rtps/history/WriterHistory.h>
#include <fastrtps/rtps/history/ReaderHistory.h>
#include <fastrtps/rtps/attributes/HistoryAttributes.h>
#include <fastrtps/rtps/builtin/data/ParticipantProxyData.h>
#include <fastrtps/rtps/builtin/discovery/participant/PDPSimple.h>
#include "timedevent/HandshakeMessageTokenResent.h"

#include <cassert>
#include <thread>
#include <boost/thread/recursive_mutex.hpp>

#define BUILTIN_ENDPOINT_PARTICIPANT_STATELESS_MESSAGE_READER (1 << 22)
#define BUILTIN_ENDPOINT_PARTICIPANT_STATELESS_MESSAGE_WRITER (1 << 23)
#define BUILTIN_ENDPOINT_PARTICIPANT_VOLATILE_MESSAGE_SECURE_READER (1 << 24)
#define BUILTIN_ENDPOINT_PARTICIPANT_VOLATILE_MESSAGE_SECURE_WRITER (1 << 25)

#define AUTHENTICATION_PARTICIPANT_STATELESS_MESSAGE "dds.sec.auth"
#define GMCLASSID_SECURITY_PARTICIPANT_CRYPTO_TOKENS "dds.sec.participant_crypto_tokens"

// TODO(Ricardo) Add event because stateless messages can be not received.

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
    participant_volatile_message_secure_listener_(*this),
    participant_(participant),
    participant_stateless_message_writer_(nullptr),
    participant_stateless_message_writer_history_(nullptr),
    participant_stateless_message_reader_(nullptr),
    participant_stateless_message_reader_history_(nullptr),
    participant_volatile_message_secure_writer_(nullptr),
    participant_volatile_message_secure_writer_history_(nullptr),
    participant_volatile_message_secure_reader_(nullptr),
    participant_volatile_message_secure_reader_history_(nullptr),
    authentication_plugin_(nullptr),
    crypto_plugin_(nullptr),
    local_identity_handle_(nullptr),
    local_participant_crypto_handle_(nullptr),
    auth_last_sequence_number_(1),
    crypto_last_sequence_number_(1)
{
    assert(participant != nullptr);
}

SecurityManager::~SecurityManager()
{
    destroy();
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
            assert(!local_identity_handle_->nil());

            // Set participant guid
            participant_->setGuid(adjusted_participant_key);

            crypto_plugin_ = factory_.create_cryptography_plugin(participant_->getRTPSParticipantAttributes().properties);

            if(crypto_plugin_ != nullptr)
            {
                NilHandle nil_handle;

                local_participant_crypto_handle_ = crypto_plugin_->cryptokeyfactory()->register_local_participant(*local_identity_handle_,
                        nil_handle,
                        participant_->getRTPSParticipantAttributes().properties.properties(),
                        exception);

                if(local_participant_crypto_handle_ != nullptr)
                {
                    assert(!local_participant_crypto_handle_->nil());
                }
                else
                {
                    logInfo(SECURITY, "Cannot register local participant in crypto plugin. (" << exception.what() << ")");
                }
            }
            else
            {
                logInfo(SECURITY, "Cryptography plugin not configured.");
            }

            if(crypto_plugin_ == nullptr || local_participant_crypto_handle_ != nullptr)
            {
                // Create RTPS entities
                if(create_entities())
                {
                    return true;
                }
            }

            if(local_participant_crypto_handle_ != nullptr)
            {
                crypto_plugin_->cryptokeyfactory()->unregister_participant(local_participant_crypto_handle_, exception);
            }

            if(crypto_plugin_ != nullptr)
            {
                delete crypto_plugin_;
                crypto_plugin_ = nullptr;
            }
        }
        else
        {
            if(strlen(exception.what()) > 0)
            {
                logError(SECURITY_AUTHENTICATION, exception.what());
            }
            else
            {
                logError(SECURITY, "Error validating the local participant");
            }
        }

        delete authentication_plugin_;
        authentication_plugin_ = nullptr;

        return false;
    }
    else
    {
        logInfo(SECURITY, "Authentication plugin not configured. Security will be disable");
    }

    return true;
}

void SecurityManager::destroy()
{
    if(authentication_plugin_ != nullptr)
    {
        SecurityException exception;

        for(auto& dp_it : discovered_participants_)
        {
            if(!dp_it.second.is_shared_secret_handle_null())
            {
                SharedSecretHandle* handle = dp_it.second.get_shared_secret();
                authentication_plugin_->return_sharedsecret_handle(handle, exception);
            }

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

            if(dp_it.second.get_event() != nullptr)
                delete dp_it.second.get_event();
        }

        discovered_participants_.clear();

        if(local_participant_crypto_handle_ != nullptr)
        {
            crypto_plugin_->cryptokeyfactory()->unregister_participant(local_participant_crypto_handle_, exception);
            local_participant_crypto_handle_ = nullptr;
        }

        if(local_identity_handle_ != nullptr)
        {
            authentication_plugin_->return_identity_handle(local_identity_handle_, exception);
            local_identity_handle_ = nullptr;
        }

        delete_entities();

        if(crypto_plugin_ != nullptr)
        {
            delete crypto_plugin_;
            crypto_plugin_ = nullptr;
        }

        delete authentication_plugin_;
        authentication_plugin_ = nullptr;
    }
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
        HandshakeHandle* handshake_handle,
        const SequenceNumber_t& sequence_number,
        HandshakeMessageTokenResent* event)
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

        if(sequence_number != SequenceNumber_t())
            dp_it->second.get_change_sequence_number() = sequence_number;

        dp_it->second.get_event() = event;
    }
    else
    {
        authentication_plugin_->return_identity_handle(remote_identity_handle, exception);

        if(handshake_handle)
            authentication_plugin_->return_handshake_handle(handshake_handle, exception);
    }
}

bool SecurityManager::discovered_participant(ParticipantProxyData* participant_data)
{
    if(authentication_plugin_ == nullptr)
    {
        participant_->pdpsimple()->notifyAboveRemoteEndpoints(participant_data);
        return true;
    }

    assert(participant_data);

    IdentityHandle* remote_identity_handle = nullptr;
    SecurityException exception;
    AuthenticationStatus auth_status = AUTHENTICATION_INIT;

    // Find information
    mutex_.lock();
    auto dp_it = discovered_participants_.find(participant_data->m_guid);

    if(dp_it == discovered_participants_.end())
    {
        discovered_participants_.emplace(std::piecewise_construct, std::forward_as_tuple(participant_data->m_guid),
                std::forward_as_tuple(participant_data, auth_status));

        mutex_.unlock();

        // Validate remote participant.
        ValidationResult_t validation_ret = authentication_plugin_->validate_remote_identity(&remote_identity_handle,
                *local_identity_handle_, IdentityToken(participant_data->identity_token_),
                participant_data->m_guid, exception);

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
                if(strlen(exception.what()) > 0)
                {
                    logError(SECURITY_AUTHENTICATION, exception.what());
                }

                remove_discovered_participant_info(participant_data->m_guid);

                // Inform user about authenticated remote participant.
                if(participant_->getListener() != nullptr)
                {
                    RTPSParticipantAuthenticationInfo info;
                    info.status(UNAUTHORIZED_RTPSPARTICIPANT);
                    info.guid(participant_data->m_guid);
                    participant_->getListener()->onRTPSParticipantAuthentication(participant_->getUserRTPSParticipant(), info);
                }
                //TODO(Ricardo) cryptograhy registration in AUTHENTICAITON_OK

                return false;
        };

        // Match entities
        match_builtin_endpoints(participant_data);

        // Store remote handle.
        mutex_.lock();
        dp_it = discovered_participants_.find(participant_data->m_guid);
        if(dp_it != discovered_participants_.end())
        {
            dp_it->second.set_auth_status(auth_status);
            bool ret = dp_it->second.set_identity_handle(remote_identity_handle);
            (void)ret; assert(ret);
            remote_identity_handle = nullptr;

            if(auth_status == AUTHENTICATION_OK)
            {
                participant_->pdpsimple()->notifyAboveRemoteEndpoints(dp_it->second.get_participant_data());

                // Inform user about authenticated remote participant.
                if(participant_->getListener() != nullptr)
                {
                    RTPSParticipantAuthenticationInfo info;
                    info.status(AUTHORIZED_RTPSPARTICIPANT);
                    info.guid(participant_data->m_guid);
                    participant_->getListener()->onRTPSParticipantAuthentication(participant_->getUserRTPSParticipant(), info);
                }
            }
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
        returnedValue = on_process_handshake(participant_data->m_guid, AUTHENTICATION_REQUEST_NOT_SEND,
                MessageIdentity(), HandshakeMessageToken(),
                remote_identity_handle, nullptr, SequenceNumber_t::unknown(), nullptr);
    }

    return returnedValue;
}

bool SecurityManager::on_process_handshake(const GUID_t& remote_participant_key,
        AuthenticationStatus pre_auth_status,
        MessageIdentity&& message_identity,
        HandshakeMessageToken&& message_in,
        IdentityHandle* remote_identity_handle,
        HandshakeHandle* handshake_handle,
        const SequenceNumber_t& previous_change,
        HandshakeMessageTokenResent* previous_event)
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
                participant_->pdpsimple()->get_participant_proxy_data_serialized(BIGEND),
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
                participant_->pdpsimple()->get_participant_proxy_data_serialized(BIGEND),
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
        if(strlen(exception.what()) > 0)
        {
            logError(SECURITY_AUTHENTICATION, exception.what());
        }

        restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle, previous_change, previous_event);

        // Inform user about authenticated remote participant.
        if(participant_->getListener() != nullptr)
        {
            RTPSParticipantAuthenticationInfo info;
            info.status(UNAUTHORIZED_RTPSPARTICIPANT);
            info.guid(remote_participant_key);
            participant_->getListener()->onRTPSParticipantAuthentication(participant_->getUserRTPSParticipant(), info);
        }

        return false;
    }

    assert(handshake_handle);

    // Remove previous change
    if(previous_event != nullptr)
        delete previous_event;
    if(previous_change != SequenceNumber_t::unknown())
        participant_stateless_message_writer_history_->remove_change(previous_change);

    bool handshake_message_send = true;
    int64_t expected_sequence_number = 0;
    SequenceNumber_t sequence_number{SequenceNumber_t::unknown()};

    if(ret == VALIDATION_PENDING_HANDSHAKE_MESSAGE ||
            ret == VALIDATION_OK_WITH_FINAL_MESSAGE)
    {
        handshake_message_send = false;

        assert(handshake_message);

        // Send hanshake message

        // Create message
        ParticipantGenericMessage message = generate_authentication_message(std::move(message_identity),
                remote_participant_key, *handshake_message);

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
                change->serializedPayload.length = aux_msg.length;

                // Send
                if(participant_stateless_message_writer_history_->add_change(change))
                {
                    handshake_message_send = true;
                    expected_sequence_number = message.message_identity().sequence_number();
                    sequence_number = change->sequenceNumber;
                }
                else
                {
                    logError(SECURITY, "WriterHistory cannot add the CacheChange_t");
                }
            }
            else
            {
                //TODO (Ricardo) Return change.
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
            case VALIDATION_OK_WITH_FINAL_MESSAGE:
            case VALIDATION_PENDING_HANDSHAKE_MESSAGE:
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
                        if(auth_status == AUTHENTICATION_OK)
                        {
                            assert(dp_it->second.is_shared_secret_handle_null());
                            dp_it->second.set_shared_secret_handle(authentication_plugin_->get_shared_secret(
                                    *handshake_handle, exception));
                            participant_->pdpsimple()->notifyAboveRemoteEndpoints(dp_it->second.get_participant_data());

                            // Inform user about authenticated remote participant.
                            if(participant_->getListener() != nullptr)
                            {
                                RTPSParticipantAuthenticationInfo info;
                                info.status(AUTHORIZED_RTPSPARTICIPANT);
                                info.guid(remote_participant_key);
                                participant_->getListener()->onRTPSParticipantAuthentication(participant_->getUserRTPSParticipant(), info);
                            }

                            // Starts cryptography mechanism
                            ParticipantCryptoHandle* remote_participant_crypto =
                                register_and_match_crypto_endpoint(dp_it->second.get_participant_data(),
                                        *remote_identity_handle,
                                        *dp_it->second.get_shared_secret());

                            if(remote_participant_crypto != nullptr && !remote_participant_crypto->nil())
                            {
                                dp_it->second.set_participant_crypto(remote_participant_crypto);
                            }
                            else
                            {
                                logError(SECURITY, "Cannot register remote participant in crypto plugin ("
                                        << remote_participant_key << ")");
                            }

                        }

                        assert(dp_it->second.get_auth_status() == pre_auth_status);
                        dp_it->second.set_auth_status(auth_status);
                        assert(dp_it->second.is_identity_handle_null());
                        dp_it->second.set_identity_handle(remote_identity_handle);
                        remote_identity_handle = nullptr;
                        assert(dp_it->second.is_handshake_handle_null());
                        dp_it->second.set_handshake_handle(handshake_handle);
                        handshake_handle = nullptr;
                        dp_it->second.get_change_sequence_number() = sequence_number;
                        if(ret == VALIDATION_PENDING_HANDSHAKE_MESSAGE)
                        {
                            assert(expected_sequence_number != 0);
                            dp_it->second.set_expected_sequence_number(expected_sequence_number);
                            dp_it->second.get_event() = new HandshakeMessageTokenResent(*this, remote_participant_key, 500); // TODO (Ricardo) Configurable
                            dp_it->second.get_event()->restart_timer();
                        }

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

    return returnedValue;
}

bool SecurityManager::create_entities()
{
    if(create_participant_stateless_message_entities())
    {
        if(crypto_plugin_ == nullptr || create_participant_volatile_message_secure_entities())
        {
            return true;
        }

        delete_participant_stateless_message_entities();
    }

    return false; 
}

void SecurityManager::delete_entities()
{
    delete_participant_volatile_message_secure_entities();
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
    ratt.endpoint.multicastLocatorList = participant_->getRTPSParticipantAttributes().builtin.metatrafficMulticastLocatorList;
    ratt.endpoint.unicastLocatorList = participant_->getRTPSParticipantAttributes().builtin.metatrafficUnicastLocatorList;

    RTPSReader* rout = nullptr;
    if(participant_->createReader(&rout, ratt, participant_stateless_message_reader_history_, &participant_stateless_message_listener_,
                participant_stateless_message_reader_entity_id, true, true))
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

bool SecurityManager::create_participant_volatile_message_secure_entities()
{
    if(create_participant_volatile_message_secure_writer())
    {
        if(create_participant_volatile_message_secure_reader())
        {
            return true;
        }

        delete_participant_volatile_message_secure_writer();
    }

    return false;
}

void SecurityManager::delete_participant_volatile_message_secure_entities()
{
    delete_participant_volatile_message_secure_reader();
    delete_participant_volatile_message_secure_writer();
}

bool SecurityManager::create_participant_volatile_message_secure_writer()
{
    HistoryAttributes hatt;
    hatt.payloadMaxSize = 5000;
    hatt.initialReservedCaches = 100;
    hatt.maximumReservedCaches = 5000;
    participant_volatile_message_secure_writer_history_ = new WriterHistory(hatt);
    WriterAttributes watt;
    watt.endpoint.endpointKind = WRITER;
    watt.endpoint.reliabilityKind = RELIABLE;
    watt.endpoint.topicKind = NO_KEY;
    watt.endpoint.durabilityKind = VOLATILE;
    watt.endpoint.unicastLocatorList = participant_->getRTPSParticipantAttributes().builtin.metatrafficUnicastLocatorList;
    // TODO(Ricardo) Study keep_all

    if(participant_->getRTPSParticipantAttributes().throughputController.bytesPerPeriod != UINT32_MAX &&
            participant_->getRTPSParticipantAttributes().throughputController.periodMillisecs != 0)
        watt.mode = ASYNCHRONOUS_WRITER;

    RTPSWriter* wout = nullptr;
    if(participant_->createWriter(&wout, watt, participant_volatile_message_secure_writer_history_, nullptr, participant_volatile_message_secure_writer_entity_id, true))
    {
        participant_volatile_message_secure_writer_ = dynamic_cast<StatefulWriter*>(wout);

        return true;
    }

    logError(SECURITY,"Participant Volatile Message Writer creation failed");
    delete(participant_volatile_message_secure_writer_history_);
    participant_volatile_message_secure_writer_history_ = nullptr;

    return false;
}

void SecurityManager::delete_participant_volatile_message_secure_writer()
{
    if(participant_volatile_message_secure_writer_ != nullptr)
    {
        delete participant_volatile_message_secure_writer_;
        participant_volatile_message_secure_writer_ = nullptr;
    }

    if(participant_volatile_message_secure_writer_history_ != nullptr)
    {
        delete participant_volatile_message_secure_writer_history_;
        participant_volatile_message_secure_writer_history_ = nullptr;
    }
}

bool SecurityManager::create_participant_volatile_message_secure_reader()
{
    HistoryAttributes hatt;
    hatt.payloadMaxSize = 5000;
    hatt.initialReservedCaches = 100;
    hatt.maximumReservedCaches = 1000000;
    participant_volatile_message_secure_reader_history_ = new ReaderHistory(hatt);
    ReaderAttributes ratt;
    ratt.endpoint.topicKind = NO_KEY;
    ratt.endpoint.reliabilityKind = RELIABLE;
    ratt.endpoint.durabilityKind = VOLATILE;
    ratt.endpoint.unicastLocatorList = participant_->getRTPSParticipantAttributes().builtin.metatrafficUnicastLocatorList;

    RTPSReader* rout = nullptr;
    if(participant_->createReader(&rout, ratt, participant_volatile_message_secure_reader_history_, &participant_volatile_message_secure_listener_,
                participant_volatile_message_secure_reader_entity_id, true, true))
    {
        participant_volatile_message_secure_reader_ = dynamic_cast<StatefulReader*>(rout);

        return true;
    }

    logError(SECURITY,"Participant Volatile Message Reader creation failed");
    delete(participant_volatile_message_secure_reader_history_);
    participant_volatile_message_secure_reader_history_ = nullptr;
    return false;
}

void SecurityManager::delete_participant_volatile_message_secure_reader()
{
    if(participant_volatile_message_secure_reader_ != nullptr)
    {
        delete participant_volatile_message_secure_reader_;
        participant_volatile_message_secure_reader_ = nullptr;
    }

    if(participant_volatile_message_secure_reader_history_ != nullptr)
    {
        delete participant_volatile_message_secure_reader_history_;
        participant_volatile_message_secure_reader_history_ = nullptr;
    }
}

ParticipantGenericMessage SecurityManager::generate_authentication_message(const MessageIdentity& related_message_identity,
        const GUID_t& destination_participant_key,
        HandshakeMessageToken& handshake_message)
{
    ParticipantGenericMessage message;

    message.message_identity().source_guid(auth_source_guid);
    message.message_identity().sequence_number(auth_last_sequence_number_.fetch_add(1));
    message.related_message_identity(related_message_identity);
    message.destination_participant_key(destination_participant_key);
    message.message_class_id(AUTHENTICATION_PARTICIPANT_STATELESS_MESSAGE);
    message.message_data().push_back(handshake_message);

    return message;
}

ParticipantGenericMessage SecurityManager::generate_crypto_token_message(const GUID_t& destination_participant_key,
        ParticipantCryptoTokenSeq& crypto_tokens)
{
    ParticipantGenericMessage message;

    message.message_identity().source_guid(auth_source_guid);
    message.message_identity().sequence_number(crypto_last_sequence_number_.fetch_add(1));
    message.destination_participant_key(destination_participant_key);
    message.message_class_id(GMCLASSID_SECURITY_PARTICIPANT_CRYPTO_TOKENS);
    message.message_data().assign(crypto_tokens.begin(), crypto_tokens.end());

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
        if(message.message_identity().source_guid() == GUID_t::unknown())
        {
            logInfo(SECURITY, "Bad ParticipantGenericMessage. message_identity.source_guid is GUID_t::unknown()");
            return;
        }
        if(message.destination_participant_key() != participant_->getGuid())
        {
            logInfo(SECURITY, "Destination of ParticipantGenericMessage is not me");
            return;
        }
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

        const GUID_t remote_participant_key(message.message_identity().source_guid().guidPrefix, c_EntityId_RTPSParticipant);
        AuthenticationStatus auth_status;
        IdentityHandle* remote_identity_handle = nullptr;
        HandshakeHandle* handshake_handle = nullptr;
        int64_t expected_sequence_number = 0;
        SequenceNumber_t previous_change{SequenceNumber_t::unknown()};
        HandshakeMessageTokenResent* previous_event = nullptr;

        mutex_.lock();
        auto dp_it = discovered_participants_.find(remote_participant_key);

        if(dp_it != discovered_participants_.end())
        {
            auth_status = dp_it->second.get_auth_status();
            remote_identity_handle = dp_it->second.get_identity_handle();
            handshake_handle = dp_it->second.get_handshake_handle();
            expected_sequence_number = dp_it->second.get_expected_sequence_number();
            previous_change = dp_it->second.get_change_sequence_number();
            previous_event = dp_it->second.get_event();
            dp_it->second.get_event() = nullptr;
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
                if(message.related_message_identity().source_guid() != GUID_t::unknown())
                {
                    logInfo(SECURITY, "Bad ParticipantGenericMessage. related_message_identity.source_guid is not GUID_t::unknown()");
                    restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle, previous_change, previous_event);
                    return;
                }
                if(message.message_data().size() != 1)
                {
                    logInfo(SECURITY, "Bad ParticipantGenericMessage. message_data size is not 1");
                    restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle, previous_change, previous_event);
                    return;
                }
            }
            else if(auth_status == AUTHENTICATION_WAITING_REPLY ||
                    auth_status == AUTHENTICATION_WAITING_FINAL)
            {
                assert(handshake_handle);

                if(message.related_message_identity().source_guid() == GUID_t::unknown() &&
                        auth_status == AUTHENTICATION_WAITING_FINAL)
                {
                    // Maybe the reply was missed. Resent.
                    if(previous_change != SequenceNumber_t::unknown())
                    {
                        // Remove previous change and send a new one.
                        SequenceNumber_t sequence_number{SequenceNumber_t::unknown()};
                        CacheChange_t* p_change = participant_stateless_message_writer_history_->remove_change_and_reuse(previous_change);

                        if(p_change != nullptr)
                        {
                            if(participant_stateless_message_writer_history_->add_change(p_change))
                            {
                                sequence_number = p_change->sequenceNumber;
                            }
                            //TODO (Ricardo) What to do if not added?
                        }

                        restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle, sequence_number, previous_event);
                        return;
                    }
                }

                // Preconditions
                if(message.related_message_identity().source_guid() != participant_stateless_message_writer_->getGuid())
                {
                    logInfo(SECURITY, "Bad ParticipantGenericMessage. related_message_identity.source_guid is not mine");
                    restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle, previous_change, previous_event);
                    return;
                }
                if(message.related_message_identity().sequence_number() != expected_sequence_number)
                {
                    logInfo(SECURITY, "Bad ParticipantGenericMessage. related_message_identity.sequence_number is not expected");
                    restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle, previous_change, previous_event);
                    return;
                }
                if(message.message_data().size() != 1)
                {
                    logInfo(SECURITY, "Bad ParticipantGenericMessage. message_data size is not 1");
                    restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle, previous_change, previous_event);
                    return;
                }
            }
            else if(auth_status == AUTHENTICATION_OK)
            {
                // Preconditions
                if(message.related_message_identity().source_guid() != participant_stateless_message_writer_->getGuid())
                {
                    logInfo(SECURITY, "Bad ParticipantGenericMessage. related_message_identity.source_guid is not mine");
                    restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle, previous_change, previous_event);
                    return;
                }
                if(message.related_message_identity().sequence_number() != expected_sequence_number)
                {
                    logInfo(SECURITY, "Bad ParticipantGenericMessage. related_message_identity.sequence_number is not expected");
                    restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle, previous_change, previous_event);
                    return;
                }
                if(message.message_data().size() != 1)
                {
                    logInfo(SECURITY, "Bad ParticipantGenericMessage. message_data size is not 1");
                    restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle, previous_change, previous_event);
                    return;
                }

                // Maybe final message was missed. Resent.
                if(previous_change != SequenceNumber_t::unknown())
                {
                    // Remove previous change and send a new one.
                    SequenceNumber_t sequence_number{SequenceNumber_t::unknown()};
                    CacheChange_t* p_change = participant_stateless_message_writer_history_->remove_change_and_reuse(previous_change);

                    if(p_change != nullptr)
                    {
                        if(participant_stateless_message_writer_history_->add_change(p_change))
                        {
                            sequence_number = p_change->sequenceNumber;
                        }
                        //TODO (Ricardo) What to do if not added?
                    }

                    restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle, sequence_number, previous_event);
                    return;
                }
            }
            else
            {
                restore_remote_identity_handle(remote_participant_key, remote_identity_handle, handshake_handle, previous_change, previous_event);
                return;
            }

            on_process_handshake(remote_participant_key, auth_status, std::move(message.message_identity()),
                    std::move(message.message_data().at(0)), remote_identity_handle,
                    handshake_handle, previous_change, previous_event);
        }
    }
    else
    {
        logInfo(SECURITY, "Discarted ParticipantGenericMessage with class id " << message.message_class_id());
    }
}

void SecurityManager::process_participant_volatile_message_secure(const CacheChange_t* const change)
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

    if(message.message_class_id().compare(GMCLASSID_SECURITY_PARTICIPANT_CRYPTO_TOKENS) == 0)
    {
        if(message.message_identity().source_guid() == GUID_t::unknown())
        {
            logInfo(SECURITY, "Bad ParticipantGenericMessage. message_identity.source_guid is GUID_t::unknown()");
            return;
        }
        if(message.destination_participant_key() != participant_->getGuid())
        {
            logInfo(SECURITY, "Destination of ParticipantGenericMessage is not me");
            return;
        }
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

        const GUID_t remote_participant_key(message.message_identity().source_guid().guidPrefix, c_EntityId_RTPSParticipant);
        ParticipantCryptoHandle* remote_participant_crypto = nullptr;

        // Search remote participant crypto handle.
        std::unique_lock<std::mutex> lock(mutex_);
        auto dp_it = discovered_participants_.find(remote_participant_key);

        if(dp_it != discovered_participants_.end())
        {
            std::cout <<"ALA" << std::endl;
            if(dp_it->second.get_participant_crypto() == nullptr)
                return;

            remote_participant_crypto = dp_it->second.get_participant_crypto();
            std::cout <<"TOMA" << std::endl;
        }
        else
        {
            logInfo(SECURITY, "Received Participant Cryptography message but not found related remote_participant_key");
        }

        if(remote_participant_crypto != nullptr)
        {
            SecurityException exception;

            if(!crypto_plugin_->cryptkeyexchange()->set_remote_participant_crypto_tokens(*local_participant_crypto_handle_,
                    *remote_participant_crypto,
                    message.message_data(),
                    exception))
            {
                logError(SECURITY, "Cannot set remote participant crypto tokens ("
                        << remote_participant_key << ") - (" << exception.what() << ")");
            }
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

void SecurityManager::ParticipantVolatileMessageListener::onNewCacheChangeAdded(RTPSReader* reader,
        const CacheChange_t* const change)
{
    manager_.process_participant_volatile_message_secure(change);

    ReaderHistory *history = reader->getHistory();
    assert(history);
    history->remove_change((CacheChange_t*)change);
}

bool SecurityManager::get_identity_token(IdentityToken** identity_token)
{
    assert(identity_token);

    if(authentication_plugin_)
    {
        SecurityException exception;
        return authentication_plugin_->get_identity_token(identity_token,
                *local_identity_handle_, exception);
    }

    return false;
}

bool SecurityManager::return_identity_token(IdentityToken* identity_token)
{
    if(identity_token == nullptr)
        return true;

    if(authentication_plugin_)
    {
        SecurityException exception;
        return authentication_plugin_->return_identity_token(identity_token,
                exception);
    }

    return false;
}

uint32_t SecurityManager::builtin_endpoints()
{
    uint32_t be = 0;

    if(participant_stateless_message_reader_ != nullptr)
        be |= BUILTIN_ENDPOINT_PARTICIPANT_STATELESS_MESSAGE_READER;
    if(participant_stateless_message_writer_ != nullptr)
        be |= BUILTIN_ENDPOINT_PARTICIPANT_STATELESS_MESSAGE_WRITER;
    if(participant_volatile_message_secure_reader_ != nullptr)
        be |= BUILTIN_ENDPOINT_PARTICIPANT_VOLATILE_MESSAGE_SECURE_READER;
    if(participant_volatile_message_secure_writer_ != nullptr)
        be |= BUILTIN_ENDPOINT_PARTICIPANT_VOLATILE_MESSAGE_SECURE_WRITER;

    return be;
}

void SecurityManager::match_builtin_endpoints(ParticipantProxyData* participant_data)
{
    uint32_t builtin_endpoints = participant_data->m_availableBuiltinEndpoints;

    if(participant_stateless_message_reader_ != nullptr &&
            builtin_endpoints & BUILTIN_ENDPOINT_PARTICIPANT_STATELESS_MESSAGE_WRITER)
    {
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = participant_data->m_guid.guidPrefix;
        watt.guid.entityId = participant_stateless_message_writer_entity_id;
        watt.endpoint.unicastLocatorList = participant_data->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = participant_data->m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = BEST_EFFORT;
        participant_stateless_message_reader_->matched_writer_add(watt);
    }

    if(participant_stateless_message_writer_ != nullptr &&
            builtin_endpoints & BUILTIN_ENDPOINT_PARTICIPANT_STATELESS_MESSAGE_READER)
    {
        RemoteReaderAttributes ratt;
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = participant_data->m_guid.guidPrefix;
        ratt.guid.entityId = participant_stateless_message_reader_entity_id;
        ratt.endpoint.unicastLocatorList = participant_data->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = participant_data->m_metatrafficMulticastLocatorList;
        ratt.endpoint.reliabilityKind = BEST_EFFORT;
        participant_stateless_message_writer_->matched_reader_add(ratt);
    }

    if(participant_volatile_message_secure_reader_ != nullptr &&
            builtin_endpoints & BUILTIN_ENDPOINT_PARTICIPANT_VOLATILE_MESSAGE_SECURE_WRITER)
    {
        RemoteWriterAttributes watt;
        watt.guid.guidPrefix = participant_data->m_guid.guidPrefix;
        watt.guid.entityId = participant_volatile_message_secure_writer_entity_id;
        watt.endpoint.unicastLocatorList = participant_data->m_metatrafficUnicastLocatorList;
        watt.endpoint.multicastLocatorList = participant_data->m_metatrafficMulticastLocatorList;
        watt.endpoint.reliabilityKind = RELIABLE;
        watt.endpoint.durabilityKind = VOLATILE;
        participant_volatile_message_secure_reader_->matched_writer_add(watt);
    }

    if(participant_volatile_message_secure_writer_ != nullptr &&
            builtin_endpoints & BUILTIN_ENDPOINT_PARTICIPANT_VOLATILE_MESSAGE_SECURE_READER)
    {
        RemoteReaderAttributes ratt;
        ratt.expectsInlineQos = false;
        ratt.guid.guidPrefix = participant_data->m_guid.guidPrefix;
        ratt.guid.entityId = participant_volatile_message_secure_reader_entity_id;
        ratt.endpoint.unicastLocatorList = participant_data->m_metatrafficUnicastLocatorList;
        ratt.endpoint.multicastLocatorList = participant_data->m_metatrafficMulticastLocatorList;
        ratt.endpoint.reliabilityKind = RELIABLE;
        ratt.endpoint.durabilityKind = VOLATILE;
        participant_volatile_message_secure_writer_->matched_reader_add(ratt);
    }
}

ParticipantCryptoHandle* SecurityManager::register_and_match_crypto_endpoint(ParticipantProxyData* participant_data,
        IdentityHandle& remote_participant_identity,
        SharedSecretHandle& shared_secret)
{
    if(crypto_plugin_ == nullptr)
        return nullptr;

    NilHandle nil_handle;
    SecurityException exception;

    // Register remote participant into crypto plugin.
    ParticipantCryptoHandle* remote_participant_crypto =
        crypto_plugin_->cryptokeyfactory()->register_matched_remote_participant(*local_participant_crypto_handle_,
            remote_participant_identity, nil_handle, shared_secret, exception);

    if(remote_participant_crypto != nullptr)
    {
        // Match remote reader and writer.

        // Get participant crypto tokens.
        ParticipantCryptoTokenSeq local_participant_crypto_tokens;
        if(crypto_plugin_->cryptkeyexchange()->create_local_participant_crypto_tokens(local_participant_crypto_tokens,
                *local_participant_crypto_handle_, *remote_participant_crypto, exception))
        {
            ParticipantGenericMessage message = generate_crypto_token_message(participant_data->m_guid,
                    local_participant_crypto_tokens);

            CacheChange_t* change = participant_volatile_message_secure_writer_->new_change([&message]() -> uint32_t
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
                    change->serializedPayload.length = aux_msg.length;

                    // Send
                    if(!participant_volatile_message_secure_writer_history_->add_change(change))
                    {
                        participant_volatile_message_secure_writer_history_->release_Cache(change);
                        logError(SECURITY, "WriterHistory cannot add the CacheChange_t");
                    }
                }
                else
                {
                    participant_volatile_message_secure_writer_history_->release_Cache(change);
                    logError(SECURITY, "Cannot serialize ParticipantGenericMessage");
                }
            }
            else
            {
                logError(SECURITY, "WriterHistory cannot retrieve a CacheChange_t");
            }
        }
        else
        {
            logError(SECURITY, "Error generating crypto token. (" << exception.what() << ")");
        }

        return remote_participant_crypto;
    }
    else
    {
        logError(SECURITY, "Error registering remote participant in cryptography plugin. (" << exception.what() << ")");
    }

    return nullptr;
}

bool SecurityManager::encode_rtps_message(CDRMessage_t& message,
        const std::vector<GuidPrefix_t> &receiving_list)
{
    if(crypto_plugin_ == nullptr)
        return false;

    assert(receiving_list.size() > 0);

    mutex_.lock();

    std::vector<ParticipantCryptoHandle*> receiving_crypto_list;
    for(const auto remote_participant : receiving_list)
    {
        const GUID_t remote_participant_key(remote_participant, c_EntityId_RTPSParticipant);
        auto dp_it = discovered_participants_.find(remote_participant_key);

        if(dp_it != discovered_participants_.end() && dp_it->second.get_participant_crypto() != nullptr)
        {
            receiving_crypto_list.push_back(dp_it->second.get_participant_crypto());
        }
        else
        {
            logInfo(SECURITY, "Cannot encode message for participant " << remote_participant_key);
        }
    }


    std::vector<uint8_t> cdr_message(message.buffer, message.buffer + message.length);
    std::vector<uint8_t> encode_cdr_message;

    SecurityException exception;
    bool ret = crypto_plugin_->cryptotransform()->encode_rtps_message(encode_cdr_message,
            cdr_message,
            *local_participant_crypto_handle_,
            receiving_crypto_list,
            exception);

    mutex_.unlock();

    memcpy(message.buffer, encode_cdr_message.data(), encode_cdr_message.size());
    message.length = encode_cdr_message.size();

    return ret;
}

bool SecurityManager::decode_rtps_message(CDRMessage_t& message,
        const GuidPrefix_t& remote_participant)
{
    if(crypto_plugin_ == nullptr)
        return false;

    std::unique_lock<std::mutex>(mutex_);

    const GUID_t remote_participant_key(remote_participant, c_EntityId_RTPSParticipant);
    auto dp_it = discovered_participants_.find(remote_participant_key);
    bool ret = false;

    if(dp_it != discovered_participants_.end() && dp_it->second.get_participant_crypto() != nullptr)
    {
        std::vector<uint8_t> encode_cdr_message(message.buffer, message.buffer + message.length);
        std::vector<uint8_t> cdr_message;

        SecurityException exception;
        ret = crypto_plugin_->cryptotransform()->decode_rtps_message(cdr_message,
                encode_cdr_message,
                *local_participant_crypto_handle_,
                *dp_it->second.get_participant_crypto(),
                exception);

        if(ret)
        {
            // TODO(Ricardo) Temporal
            memcpy(message.buffer + 20 /*RTPS_HEADER_SIZE*/, cdr_message.data(), cdr_message.size());
            message.length = 20 + cdr_message.size();
        }
        else
        {
            logInfo(SECURITY, "Cannot decode rtps message (" << exception.what() << ")");
        }
    }
    else
    {
        logInfo(SECURITY, "Cannot decode message for participant " << remote_participant_key);
    }

    return ret;
}
