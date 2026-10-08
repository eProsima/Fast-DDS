# Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#{{{ Interface library for grouping the PKI authentication plugin and artifact providers

add_library(fastdds-security-pki INTERFACE)

add_library(fastdds::security::pki ALIAS fastdds-security-pki)

target_compile_features(fastdds-security-pki INTERFACE cxx_std_11)

# Same condition under which the consumer suites compile these sources.
if(SECURITY)
    target_sources(fastdds-security-pki INTERFACE
        artifact_providers/FileProvider.cpp
        artifact_providers/Pkcs11Provider.cpp
        authentication/PKIDH.cpp
        authentication/PKIHandshakeHandle.cpp
        )
endif()

# Settings this group's code is sensitive to. See cmake/common/reusable_objects.cmake.
fastdds_declare_object_group(fastdds-security-pki
    MOCKS
        dds/QosPolicies
        rtps/Endpoint
        rtps/Log
        rtps/MessageReceiver
        rtps/NetworkFactory
        rtps/ParticipantProxyData
        rtps/ReaderLocator
        rtps/ReaderProxyData
        rtps/ReceiverResource
        rtps/RTPSMessageGroup
        rtps/RTPSParticipantAttributes
        rtps/RTPSWriter
        rtps/SharedSecretHandle
        rtps/StatefulWriter
        rtps/StatelessWriter
        rtps/TimedEvent
        rtps/UDPTransportDescriptor
        rtps/UDPv4TransportDescriptor
        rtps/WriterHistory
    DEFINES
        ENABLE_OLD_LOG_MACROS_
        FASTDDS_ENFORCE_LOG_INFO
        HAVE_LOG_NO_ERROR
        HAVE_LOG_NO_INFO
        HAVE_LOG_NO_WARNING
    )

#}}}

#{{{ Interface library for grouping the permissions access control plugin

add_library(fastdds-security-access-control INTERFACE)

add_library(fastdds::security::access-control ALIAS fastdds-security-access-control)

target_compile_features(fastdds-security-access-control INTERFACE cxx_std_11)

# Same condition under which the consumer suites compile these sources.
if(SECURITY)
    target_sources(fastdds-security-access-control INTERFACE
        accesscontrol/CommonParser.cpp
        accesscontrol/GovernanceParser.cpp
        accesscontrol/Permissions.cpp
        accesscontrol/PermissionsParser.cpp
        )
endif()

# Settings this group's code is sensitive to. See cmake/common/reusable_objects.cmake.
fastdds_declare_object_group(fastdds-security-access-control
    MOCKS
        dds/QosPolicies
        rtps/Endpoint
        rtps/Log
        rtps/MessageReceiver
        rtps/NetworkFactory
        rtps/ParticipantProxyData
        rtps/ReaderLocator
        rtps/ReaderProxyData
        rtps/ReceiverResource
        rtps/RTPSMessageGroup
        rtps/RTPSParticipantAttributes
        rtps/RTPSWriter
        rtps/StatefulWriter
        rtps/StatelessWriter
        rtps/TimedEvent
        rtps/UDPTransportDescriptor
        rtps/UDPv4TransportDescriptor
        rtps/WriterHistory
        rtps/WriterProxyData
    DEFINES
        ENABLE_OLD_LOG_MACROS_
        FASTDDS_ENFORCE_LOG_INFO
        HAVE_LOG_NO_ERROR
        HAVE_LOG_NO_INFO
        HAVE_LOG_NO_WARNING
    )

#}}}

#{{{ Interface library for grouping the AES-GCM-GMAC cryptography plugin

add_library(fastdds-security-crypto INTERFACE)

add_library(fastdds::security::crypto ALIAS fastdds-security-crypto)

target_compile_features(fastdds-security-crypto INTERFACE cxx_std_11)

# Same condition under which the consumer suites compile these sources.
if(SECURITY)
    target_sources(fastdds-security-crypto INTERFACE
        cryptography/AESGCMGMAC.cpp
        cryptography/AESGCMGMAC_KeyExchange.cpp
        cryptography/AESGCMGMAC_KeyFactory.cpp
        cryptography/AESGCMGMAC_Transform.cpp
        )
endif()

# Settings this group's code is sensitive to. See cmake/common/reusable_objects.cmake.
fastdds_declare_object_group(fastdds-security-crypto
    MOCKS
        rtps/Log
        rtps/SharedSecretHandle
    DEFINES
        ENABLE_OLD_LOG_MACROS_
        FASTDDS_ENFORCE_LOG_INFO
        HAVE_LOG_NO_ERROR
        HAVE_LOG_NO_INFO
        HAVE_LOG_NO_WARNING
    )

#}}}
