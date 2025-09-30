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

/*!
 * @file SecurityManager.h
 */
#ifndef FASTDDS_RTPS_SECURITY__SECURITYMANAGER_H
#define FASTDDS_RTPS_SECURITY__SECURITYMANAGER_H

#include <atomic>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include <fastdds/rtps/attributes/HistoryAttributes.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <fastdds/rtps/reader/ReaderListener.hpp>
#include <fastdds/rtps/writer/WriterListener.hpp>

#include <rtps/builtin/data/ParticipantProxyData.hpp>
#include <rtps/builtin/data/ReaderProxyData.hpp>
#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/resources/TimedEvent.h>
#include <rtps/security/authentication/Handshake.h>
#include <rtps/security/common/ParticipantGenericMessage.h>
#include <rtps/security/ISecurityPluginFactory.h>
#include <utils/ProxyPool.hpp>
#include <utils/shared_mutex.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class RTPSParticipantImpl;
class RTPSWriter;
class StatelessWriter;
class StatelessReader;
class StatefulWriter;
class StatefulReader;
class WriterHistory;
class ReaderHistory;
class ITopicPayloadPool;

namespace security {

class Authentication;
class AccessControl;
class Cryptography;
struct ParticipantSecurityAttributes;
struct EndpointSecurityAttributes;

/**
 * Class SecurityManager used to implemente the security handshake protocol.
 *
 * @ingroup SECURITY_MODULE
 */
class SecurityManager : private WriterListener
{
    static constexpr std::size_t PARTICIPANT_STATELESS_MESSAGE_PAYLOAD_DEFAULT_SIZE = 8192;
    static constexpr std::size_t PARTICIPANT_VOLATILE_MESSAGE_PAYLOAD_DEFAULT_SIZE = 1024;

public:

    /**
     * SecurityManager constructor
     *
     * @param participant RTPSParticipantImpl* references the associated participant
     */
    SecurityManager(
            RTPSParticipantImpl* participant,
            ISecurityPluginFactory& plugin_factory);

    // @brief Destructor
    ~SecurityManager();

    /**
     * SecurityManager initialization
     *
     * @param attributes ParticipantSecurityAttributes references the plugin configuration
     * @param participant_properties PropertyPolicy& references configuration provided on participant creation
     * @return true if the configuration is successfully applied (plugins creation and set up)
     */
    bool init(
            ParticipantSecurityAttributes& attributes,
            const PropertyPolicy& participant_properties);

    /**
     * Creates all the security builtin endpoints
     *
     * @pre SecurityManager mutex shouldn't have been taken.
     *
     * @return true on successful creation
     */
    bool create_entities();

    /**
     * @brief resource clean up
     *
     * @pre SecurityManager mutex shouldn't have been taken.
     */
    void destroy();

    /**
     * Called from the discovery listener. Begins the security handshake of the security protocol.
     *
     * @pre SecurityManager mutex shouldn't have been taken.
     * @param participant_data ParticipantProxyData& references the participant proxy
     * @return true on success
     */
    bool discovered_participant(
            const ParticipantProxyData& participant_data);

    /**
     * Called from the discovery listener. Frees all the resources associated to a demise participant.
     *
     * @pre SecurityManager mutex shouldn't have been taken.
     * @param participant_data ParticipantProxyData& references the participant proxy
     * @return true on success
     */
    void remove_participant(
            const ParticipantProxyData& participant_data);

    bool register_local_writer(
            const GUID_t& writer_guid,
            const PropertyPolicy& writer_properties,
            EndpointSecurityAttributes& security_attributes);

    bool register_local_builtin_writer(
            const GUID_t& writer_guid,
            EndpointSecurityAttributes& security_attributes);

    bool unregister_local_writer(
            const GUID_t& writer_guid);

    bool register_local_reader(
            const GUID_t& reader_guid,
            const PropertyPolicy& reader_properties,
            EndpointSecurityAttributes& security_attributes);

    bool register_local_builtin_reader(
            const GUID_t& reader_guid,
            EndpointSecurityAttributes& security_attributes);

    bool unregister_local_reader(
            const GUID_t& reader_guid);

    bool discovered_reader(
            const GUID_t& writer_guid,
            const GUID_t& remote_participant,
            ReaderProxyData& remote_reader_data,
            const EndpointSecurityAttributes& security_attributes);

    void remove_reader(
            const GUID_t& writer_guid,
            const GUID_t& remote_participant,
            const GUID_t& remote_reader_guid);

    bool discovered_builtin_reader(
            const GUID_t& writer_guid,
            const GUID_t& remote_participant,
            ReaderProxyData& remote_reader_data,
            const EndpointSecurityAttributes& security_attributes);

    bool discovered_writer(
            const GUID_t& reader_guid,
            const GUID_t& remote_participant,
            WriterProxyData& remote_writer_guid,
            const EndpointSecurityAttributes& security_attributes);

    void remove_writer(
            const GUID_t& reader_guid,
            const GUID_t& remote_participant,
            const GUID_t& remote_writer_guid);

    bool discovered_builtin_writer(
            const GUID_t& reader_guid,
            const GUID_t& remote_participant,
            WriterProxyData& remote_writer_guid,
            const EndpointSecurityAttributes& security_attributes);

    bool get_datawriter_sec_attributes(
            const PropertyPolicy& writer_properties,
            EndpointSecurityAttributes& security_attributes);

    bool get_datareader_sec_attributes(
            const PropertyPolicy& reader_properties,
            EndpointSecurityAttributes& security_attributes);

    /**
     * Retrieves a handle to the identity token used in handshake
     *
     * @param identity_token IdentityToken** handle reference to set
     * @return true on successful retrieval
     */
    bool get_identity_token(
            IdentityToken** identity_token) const;

    /**
     * Releases a handle to the identity token used in handshake
     *
     * @param identity_token IdentityToken* handle reference to release
     * @return true on successful disposal
     */
    bool return_identity_token(
            IdentityToken* identity_token) const;

    /**
     * Retrieves a handle to the permision token used in handshake
     *
     * @param permissions_token PermissionsToken** handle reference to set
     * @return true on successful retrieval
     */
    bool get_permissions_token(
            PermissionsToken** permissions_token) const;

    /**
     * Releases a handle to the permissions token used in handshake
     *
     * @param permissions_token PermissionsToken* handle reference to release
     * @return true on successful disposal
     */
    bool return_permissions_token(
            PermissionsToken* permissions_token) const;

    /**
     * Returns a mask of available security builtin endpoints
     *
     * @pre SecurityManager mutex shouldn't have been taken with exclusive ownership.
     * @return mask
     */
    uint32_t builtin_endpoints() const;

    /**
     * Returns whether a mangled GUID is the same as the original.
     * @param adjusted Mangled GUID prefix
     * @param original Original GUID prefix candidate to compare
     * @return true when @c adjusted corresponds to @c original
     */
    bool check_guid_comes_from(
            const GUID_t& adjusted,
            const GUID_t& original) const;

    RTPSParticipantImpl* participant() const
    {
        return participant_;
    }

    bool encode_rtps_message(
            const CDRMessage_t& input_message,
            CDRMessage_t& output_message,
            const std::vector<GuidPrefix_t>& receiving_list) const;

    int decode_rtps_message(
            const CDRMessage_t& message,
            CDRMessage_t& out_message,
            const GuidPrefix_t& sending_participant) const;

    bool encode_writer_submessage(
            const CDRMessage_t& input_message,
            CDRMessage_t& output_message,
            const GUID_t& writer_guid,
            const std::vector<GUID_t>& receiving_list) const;

    bool encode_reader_submessage(
            const CDRMessage_t& input_message,
            CDRMessage_t& output_message,
            const GUID_t& reader_guid,
            const std::vector<GUID_t>& receiving_list) const;

    int decode_rtps_submessage(
            CDRMessage_t& message,
            CDRMessage_t& out_message,
            const GuidPrefix_t& sending_participant) const;

    bool encode_serialized_payload(
            const SerializedPayload_t& payload,
            SerializedPayload_t& output_payload,
            const GUID_t& writer_guid) const;

    bool decode_serialized_payload(
            const SerializedPayload_t& secure_payload,
            SerializedPayload_t& payload,
            const GUID_t& reader_guid,
            const GUID_t& writer_guid) const;

    uint32_t calculate_extra_size_for_rtps_message() const;

    uint32_t calculate_extra_size_for_rtps_submessage(
            const GUID_t& writer_guid) const;

    uint32_t calculate_extra_size_for_encoded_payload(
            const GUID_t& writer_guid) const;

    /**
     * Queries the state
     *
     * @return true if enabled
     */
    bool is_security_active() const
    {
        if (ready_state_)
        {
            return *ready_state_;
        }
        return false;
    }

    /**
     * Queries the initialization state
     *
     * @return false if not initialized or disabled
     */
    bool is_security_initialized() const
    {
        return (bool)ready_state_;
    }

    /**
     * Access the temporary proxy pool for reader proxies
     * @return pool reference
     */
    ProxyPool<ReaderProxyData>& get_temporary_reader_proxies_pool()
    {
        return temp_reader_proxies_;
    }

    /**
     * Access the temporary proxy pool for writer proxies
     * @return pool reference
     */
    ProxyPool<WriterProxyData>& get_temporary_writer_proxies_pool()
    {
        return temp_writer_proxies_;
    }

private:

    enum AuthenticationStatus : uint32_t
    {
        AUTHENTICATION_OK = 0,
        AUTHENTICATION_FAILED,
        AUTHENTICATION_INIT,
        AUTHENTICATION_REQUEST_NOT_SEND,
        AUTHENTICATION_WAITING_REQUEST,
        AUTHENTICATION_WAITING_REPLY,
        AUTHENTICATION_WAITING_FINAL,
        AUTHENTICATION_NOT_AVAILABLE
    };

    struct AuthenticationHandshakeProperties
    {
        AuthenticationHandshakeProperties();
        ~AuthenticationHandshakeProperties() = default;

        /**
         * @brief Parses the properties from a propertypolicy rule
         * @param properties PropertyPolicy reference to the properties to parse
         */
        void parse_from_property_policy(
                const PropertyPolicy& properties);

        // Maximum number of handshake requests to be sent
        // Must be greater than 0
        int32_t max_handshake_requests_;
        // Initial wait time (in milliseconds) for the first handshake request resend
        // Must be greater than 0
        int32_t initial_handshake_resend_period_ms_;
        // Gain for the period between handshake request resends
        // The initial period is multiplied by this value each time a resend is performed
        // Must be greater than 1
        double handshake_resend_period_gain_;
    };

    class DiscoveredParticipantInfo
    {
        struct AuthenticationInfo
        {
        public:

            typedef std::unique_ptr<TimedEvent> EventUniquePtr;

            AuthenticationInfo(
                    AuthenticationStatus auth_status)
                : handshake_requests_sent_(0)
                , identity_handle_(nullptr)
                , handshake_handle_(nullptr)
                , auth_status_(auth_status)
                , expected_sequence_number_(0)
                , change_sequence_number_(SequenceNumber_t::unknown())

            {
            }

            AuthenticationInfo(
                    AuthenticationInfo&& auth) noexcept
                : identity_handle_(std::move(auth.identity_handle_))
                , handshake_handle_(std::move(auth.handshake_handle_))
                , auth_status_(auth.auth_status_)
                , expected_sequence_number_(auth.expected_sequence_number_)
                , change_sequence_number_(std::move(auth.change_sequence_number_))
                , event_(std::move(auth.event_))
            {
                auth.identity_handle_ = nullptr;
                auth.handshake_handle_ = nullptr;
                auth.auth_status_ = AUTHENTICATION_NOT_AVAILABLE;
                auth.expected_sequence_number_ = 0;
                auth.change_sequence_number_ = SequenceNumber_t::unknown();
            }

            int32_t handshake_requests_sent_;

            IdentityHandle* identity_handle_;

            HandshakeHandle* handshake_handle_;

            AuthenticationStatus auth_status_;

            int64_t expected_sequence_number_;

            SequenceNumber_t change_sequence_number_;

            EventUniquePtr event_;

        private:

            AuthenticationInfo(
                    const AuthenticationInfo& auth) = delete;
        };

    public:

        typedef std::unique_ptr<AuthenticationInfo> AuthUniquePtr;

        DiscoveredParticipantInfo(
                AuthenticationStatus auth_status,
                const ParticipantProxyData& participant_data)
            : auth_(new AuthenticationInfo(auth_status))
            , permissions_handle_(nullptr)
            , participant_crypto_(nullptr)
            , participant_data_(participant_data)
        {
        }

        DiscoveredParticipantInfo(
                DiscoveredParticipantInfo&& info)
            : auth_(std::move(info.auth_))
            , shared_secret_handle_(std::move(info.shared_secret_handle_))
            , permissions_handle_(std::move(info.permissions_handle_))
            , participant_crypto_(info.participant_crypto_)
            , participant_data_(std::move(info.participant_data_))
        {
        }

        AuthUniquePtr get_auth()
        {
            std::lock_guard<std::mutex> g(mtx_);
            return std::move(auth_);
        }

        void set_auth(
                AuthUniquePtr& auth)
        {
            std::lock_guard<std::mutex> g(mtx_);
            auth_ = std::move(auth);
        }

        void set_shared_secret(
                std::shared_ptr<SecretHandle>& shared_secret)
        {
            std::lock_guard<std::mutex> g(mtx_);
            shared_secret_handle_ = shared_secret;
        }

        std::shared_ptr<SecretHandle> get_shared_secret()
        {
            std::lock_guard<std::mutex> g(mtx_);
            return shared_secret_handle_;
        }

        void set_permissions_handle(
                PermissionsHandle* handle)
        {
            std::lock_guard<std::mutex> g(mtx_);
            permissions_handle_ = handle;
        }

        PermissionsHandle* get_permissions_handle()
        {
            std::lock_guard<std::mutex> g(mtx_);
            return permissions_handle_;
        }

        const PermissionsHandle* get_permissions_handle() const
        {
            std::lock_guard<std::mutex> g(mtx_);
            return permissions_handle_;
        }

        void set_participant_crypto(
                std::shared_ptr<ParticipantCryptoHandle> participant_crypto)
        {
            std::lock_guard<std::mutex> g(mtx_);
            participant_crypto_ = participant_crypto;
        }

        std::shared_ptr<ParticipantCryptoHandle> get_participant_crypto()
        {
            std::lock_guard<std::mutex> g(mtx_);
            return participant_crypto_;
        }

        const ParticipantProxyData& participant_data() const
        {
            std::lock_guard<std::mutex> g(mtx_);
            return participant_data_;
        }

        bool check_guid_comes_from(
                Authentication* const auth_plugin,
                const GUID_t& adjusted,
                const GUID_t& original);

        AuthenticationStatus get_auth_status() const
        {
            std::lock_guard<std::mutex> g(mtx_);
            if (auth_.get() != nullptr)
            {
                return auth_->auth_status_;
            }
            else
            {
                return AUTHENTICATION_NOT_AVAILABLE;
            }

        }

    private:

        DiscoveredParticipantInfo(
                const DiscoveredParticipantInfo& info) = delete;

        mutable std::mutex mtx_;

        AuthUniquePtr auth_;

        std::shared_ptr<SecretHandle> shared_secret_handle_;

        PermissionsHandle* permissions_handle_;

        std::shared_ptr<ParticipantCryptoHandle> participant_crypto_;

        ParticipantProxyData participant_data_;

    };

    class ParticipantStatelessMessageListener : public eprosima::fastdds::rtps::ReaderListener
    {
    public:

        ParticipantStatelessMessageListener(
                SecurityManager& manager)
            : manager_(manager)
        {
        }

        ~ParticipantStatelessMessageListener()
        {
        }

        void on_new_cache_change_added(
                RTPSReader* reader,
                const CacheChange_t* const change) override;

    private:

        ParticipantStatelessMessageListener& operator =(
                const ParticipantStatelessMessageListener&) = delete;

        SecurityManager& manager_;
    }
    participant_stateless_message_listener_;

    class ParticipantVolatileMessageListener : public eprosima::fastdds::rtps::ReaderListener
    {
    public:

        ParticipantVolatileMessageListener(
                SecurityManager& manager)
            : manager_(manager)
        {
        }

        ~ParticipantVolatileMessageListener()
        {
        }

        void on_new_cache_change_added(
                RTPSReader* reader,
                const CacheChange_t* const change) override;

    private:

        ParticipantVolatileMessageListener& operator =(
                const ParticipantVolatileMessageListener&) = delete;

        SecurityManager& manager_;
    }
    participant_volatile_message_secure_listener_;

    void cancel_init();

    /**
     * Releases resources associated to another secured participant discovered.
     *
     * @param auth_ptr DiscoveredParticipantInfo::AuthUniquePtr& remote participant info to release.
     */
    void remove_discovered_participant_info(
            const DiscoveredParticipantInfo::AuthUniquePtr& auth_ptr);

    /**
     * Sets the specified info for a remote participant
     *
     * @pre SecurityManager mutex should not have been taken yet.
     *
     * @param remote_participant_key GUID_t& reference to the participant to update
     * @param auth_ptr DiscoveredParticipantInfo::AuthUniquePtr& remote participant info to set.
     */
    bool restore_discovered_participant_info(
            const GUID_t& remote_participant_key,
            DiscoveredParticipantInfo::AuthUniquePtr& auth_ptr);

    /**
     * Deletes all the security builtin endpoints
     *
     * @pre SecurityManager mutex should have been taken as exclusive ownership.
     */
    void delete_entities();
    bool create_participant_stateless_message_entities();
    void delete_participant_stateless_message_entities();
    void create_participant_stateless_message_pool();
    void delete_participant_stateless_message_pool();
    bool create_participant_stateless_message_writer();
    void delete_participant_stateless_message_writer();
    bool create_participant_stateless_message_reader();
    void delete_participant_stateless_message_reader();
    bool create_participant_volatile_message_secure_entities();
    void delete_participant_volatile_message_secure_entities();
    void create_participant_volatile_message_secure_pool();
    void delete_participant_volatile_message_secure_pool();
    bool create_participant_volatile_message_secure_writer();
    void delete_participant_volatile_message_secure_writer();
    bool create_participant_volatile_message_secure_reader();
    void delete_participant_volatile_message_secure_reader();

    bool discovered_reader(
            const GUID_t& writer_guid,
            const GUID_t& remote_participant,
            ReaderProxyData& remote_reader_data,
            const EndpointSecurityAttributes& security_attributes,
            bool is_builtin);

    bool discovered_writer(
            const GUID_t& reader_guid,
            const GUID_t& remote_participant,
            WriterProxyData& remote_writer_guid,
            const EndpointSecurityAttributes& security_attributes,
            bool is_builtin);

    /**
     * Match builtin endpoints with those of a remote participant
     *
     * @pre SecurityManager mutex shouldn't have been taken with exclusive ownership.
     * @param participant_data ParticipantProxyData& remote participant proxy
     */
    void match_builtin_endpoints(
            const ParticipantProxyData& participant_data);

    /**
     * Match builtin endpoints devoted to key exchanges with those of a remote participant
     *
     * @pre SecurityManager mutex shouldn't have been taken with exclusive ownership.
     * @param participant_data ParticipantProxyData& remote participant proxy
     */
    void match_builtin_key_exchange_endpoints(
            const ParticipantProxyData& participant_data);

    /**
     * Unmatch builtin endpoints with those of a remote participant
     *
     * @pre SecurityManager mutex shouldn't have been taken with exclusive ownership.
     * @param participant_data ParticipantProxyData& remote participant proxy
     */
    void unmatch_builtin_endpoints(
            const ParticipantProxyData& participant_data);

    std::shared_ptr<ParticipantCryptoHandle> register_and_match_crypto_endpoint(
            IdentityHandle& remote_participant_identity,
            SecretHandle& shared_secret);

    /**
     * Manage cryptographic exchange with a remote participant
     *
     * @pre SecurityManager mutex shouldn't have been taken with exclusive ownership.
     * @param remote_participant_crypto ParticipantCryptoHandle* handle to cryptographic data
     * @param remote_participant_guid GUID_t& remote participant id
     */
    void exchange_participant_crypto(
            std::shared_ptr<ParticipantCryptoHandle> remote_participant_crypto,
            const GUID_t& remote_participant_guid);

    void process_participant_stateless_message(
            const CacheChange_t* const change);

    void process_participant_volatile_message_secure(
            const CacheChange_t* const change);

    /**
     * Called from the discovery listener and security builitin listeners.
     * Implements security protocol handshake logic state machine.
     *
     * @pre SecurityManager mutex shouldn't have been taken.
     * @param participant_data ParticipantProxyData& exchange partner
     * @param remote_participant_info DiscoveredParticipantInfo::AuthUniquePtr& exchange partner authorization data
     * @param message_identity MessageIdentity&& identifies the message to process
     * @param message_in HandshakeMessageToken&& required by the protocol
     * @param notify_part_authorized [out] Whether to notify afterwards if Authentication was successfulful
     * @return true on success
     */
    bool on_process_handshake(
            const ParticipantProxyData& participant_data,
            DiscoveredParticipantInfo::AuthUniquePtr& remote_participant_info,
            MessageIdentity&& message_identity,
            HandshakeMessageToken&& message_in,
            bool& notify_part_authorized);

    ParticipantGenericMessage generate_authentication_message(
            const MessageIdentity& related_message_identity,
            const GUID_t& destination_participant_key,
            HandshakeMessageToken& handshake_message) const;

    ParticipantGenericMessage generate_participant_crypto_token_message(
            const GUID_t& destination_participant_key,
            ParticipantCryptoTokenSeq& crypto_tokens) const;

    ParticipantGenericMessage generate_writer_crypto_token_message(
            const GUID_t& destination_participant_key,
            const GUID_t& destination_endpoint_key,
            const GUID_t& source_endpoint_key,
            ParticipantCryptoTokenSeq& crypto_tokens) const;

    ParticipantGenericMessage generate_reader_crypto_token_message(
            const GUID_t& destination_participant_key,
            const GUID_t& destination_endpoint_key,
            const GUID_t& source_endpoint_key,
            ParticipantCryptoTokenSeq& crypto_tokens) const;

    /**
     * Performs cryptography by exchanging secrets
     * if ok, participant is authorized
     *
     * @param participant_data ParticipantProxyData& exchange partner
     * @param remote_participant_info DiscoveredParticipantInfo::AuthUniquePtr& exchange partner authorization data
     * @param shared_secret_handle shared secret key
     * @return true on success
     */
    bool participant_authorized(
            const ParticipantProxyData& participant_data,
            const DiscoveredParticipantInfo::AuthUniquePtr& remote_participant_info,
            std::shared_ptr<SecretHandle>& shared_secret_handle);

    /**
     * Notifies above remote endpoints and participants listener
     * that a participant was authorized
     * @param participant_data ParticipantProxyData& remote partner
     */
    void notify_participant_authorized(
            const ParticipantProxyData& participant_data);

    void resend_handshake_message_token(
            const GUID_t& remote_participant_key) const;

    /**
     * Determines de action to do when validation process fails
     * @param participant_data ParticipantProxyData& exchange partner
     * @param exception Exception to generate (if any)
     * @return true if this participant should be considered as authenticated
     */
    void on_validation_failed(
            const ParticipantProxyData& participant_data,
            const SecurityException& exception) const;

    RTPSParticipantImpl* participant_ = nullptr;
    StatelessWriter* participant_stateless_message_writer_ = nullptr;
    WriterHistory* participant_stateless_message_writer_history_ = nullptr;
    StatelessReader* participant_stateless_message_reader_ = nullptr;
    ReaderHistory* participant_stateless_message_reader_history_ = nullptr;
    StatefulWriter* participant_volatile_message_secure_writer_ = nullptr;
    WriterHistory* participant_volatile_message_secure_writer_history_ = nullptr;
    StatefulReader* participant_volatile_message_secure_reader_ = nullptr;
    ReaderHistory* participant_volatile_message_secure_reader_history_ = nullptr;
    ISecurityPluginFactory& factory_;

    Logging* logging_plugin_ = nullptr;
    Authentication* authentication_plugin_ = nullptr;
    AccessControl* access_plugin_ = nullptr;
    Cryptography* crypto_plugin_ = nullptr;

    uint32_t domain_id_ = 0;

    AuthenticationHandshakeProperties auth_handshake_props_;

    IdentityHandle* local_identity_handle_ = nullptr;

    PermissionsHandle* local_permissions_handle_ = nullptr;

    std::shared_ptr<ParticipantCryptoHandle> local_participant_crypto_handle_;

    // collection members can be modified inside SecurityManager const calls because them take care of its own
    // synchronization
    std::map<GUID_t, std::unique_ptr<DiscoveredParticipantInfo>> discovered_participants_;

    GUID_t auth_source_guid;

    /**
     * Enables the use of the plugins for the other methods
     *
     * @pre this method was never called before
     * @return RAII object
     */
    void enable_security_manager()
    {
        assert(!ready_state_);
        ready_state_.reset(new bool(nullptr != authentication_plugin_));
    }

    /**
     * Returns an object that:
     * @li its <tt>bool operator()</tt> allows to check plugins availability
     * @li guarantees the plugings won't be destroyed while the object is alive
     * Use as:
     *  @code{.cpp}
     *      auto sentry = is_security_manager_enabled();
     *      if(!sentry)
     *          return;
     *      // henceforth plugins are available
     *  @endcode
     *
     * @return RAII object
     */
    std::shared_ptr<bool> is_security_manager_initialized() const
    {
        return ready_state_;
    }

    /**
     * Disables the use of the plugins for the other methods.
     * Waits until no other method uses the plugins
     *
     * @pre <tt>enable_security_manager()</tt> was called
     * @post no method using the plugins will be ongoing or called
     */
    void disable_security_manager()
    {
        std::weak_ptr<bool> wp(ready_state_);
        ready_state_.reset();

        while (!wp.expired())
        {
            std::this_thread::yield();
        }
    }

    void on_writer_change_received_by_all(
            RTPSWriter* writer,
            CacheChange_t* change) override;

    /**
     * Syncronization object for plugin initialization, <tt>mutex_</tt> protection is not necessary to guarantee plugin
     * availability.
     * @li (bool)ready_state_ -> SecurityManager initialized
     * @li (bool)*ready_state_ -> security active
     */
    std::shared_ptr<bool> ready_state_;

    mutable shared_mutex mutex_;

    mutable std::atomic<int64_t> auth_last_sequence_number_;

    mutable std::atomic<int64_t> crypto_last_sequence_number_;

    struct DatawriterAssociations
    {
        DatawriterAssociations(
                DatawriterCryptoHandle* wh)
            : writer_handle(wh)
        {
        }

        DatawriterCryptoHandle* writer_handle;

        std::map<GUID_t, std::tuple<ReaderProxyData, DatareaderCryptoHandle*>> associated_readers;
    };

    struct DatareaderAssociations
    {
        DatareaderAssociations(
                DatareaderCryptoHandle* rh)
            : reader_handle(rh)
        {
        }

        DatareaderCryptoHandle* reader_handle;

        std::map<GUID_t, std::tuple<WriterProxyData, DatawriterCryptoHandle*>> associated_writers;
    };

    // TODO(Ricardo) Temporal. Store individual in Fast DDS code.
    std::map<GUID_t, DatawriterAssociations> writer_handles_;
    std::map<GUID_t, DatareaderAssociations> reader_handles_;

    std::map<GUID_t, DataHolderSeq> remote_participant_pending_messages_;
    std::map<std::pair<GUID_t, GUID_t>, DataHolderSeq> remote_writer_pending_messages_;
    std::map<std::pair<GUID_t, GUID_t>, DataHolderSeq> remote_reader_pending_messages_;
    std::list<std::tuple<ReaderProxyData, GUID_t, GUID_t>> remote_reader_pending_discovery_messages_;
    std::list<std::tuple<WriterProxyData, GUID_t, GUID_t>> remote_writer_pending_discovery_messages_;

    //! ProxyPool for temporary reader proxies
    ProxyPool<ReaderProxyData> temp_reader_proxies_;
    //! ProxyPool for temporary writer proxies
    ProxyPool<WriterProxyData> temp_writer_proxies_;


    HistoryAttributes participant_stateless_message_writer_hattr_;
    HistoryAttributes participant_stateless_message_reader_hattr_;
    std::shared_ptr<ITopicPayloadPool> participant_stateless_message_pool_;

    HistoryAttributes participant_volatile_message_secure_hattr_;
    std::shared_ptr<ITopicPayloadPool> participant_volatile_message_secure_pool_;
};

} //namespace security
} //namespace rtps
} //namespace fastdds
} //namespace eprosima

#endif // FASTDDS_RTPS_SECURITY__SECURITYMANAGER_H
