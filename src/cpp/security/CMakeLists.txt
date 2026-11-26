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

#{{{ Unit grouping the handles and artifact providers shared by the builtin security plugins

if(SECURITY)
    fastdds_add_unit(security::common
        SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/AccessPermissionsHandle.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/AccessPermissionsHandle.h
        ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/DistinguishedName.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/DistinguishedName.h
        ${PROJECT_SOURCE_DIR}/src/cpp/security/artifact_providers/FileProvider.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/artifact_providers/FileProvider.hpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/artifact_providers/Pkcs11Provider.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/artifact_providers/Pkcs11Provider.hpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/PKIHandshakeHandle.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/PKIHandshakeHandle.h
        ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/PKIIdentityHandle.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/PKIIdentityHandle.h
        ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/AESGCMGMAC_Types.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/AESGCMGMAC_Types.h
        ${PROJECT_SOURCE_DIR}/src/cpp/rtps/security/common/SharedSecretHandle.cpp
        DEPENDS
        log
        rtps::basic
        time
        utils::network
        LINK_LIBRARIES
        PUBLIC
        fastcdr
        OpenSSL::SSL
        OpenSSL::Crypto
        $<$<BOOL:${WIN32}>:crypt32.lib>
        )
endif()

#}}}

#{{{ Unit with the builtin authentication plugin (DDS:Auth:PKI-DH)

if(SECURITY)
    fastdds_add_unit(security::authentication
        SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/PKIDH.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/authentication/PKIDH.h
        DEPENDS
        log
        rtps::basic
        rtps::common
        rtps::parameter-list
        rtps::security-logging
        rtps::timed-event
        security::common
        time
        LINK_LIBRARIES
        PUBLIC
        fastcdr
        OpenSSL::SSL
        OpenSSL::Crypto
        $<$<BOOL:${WIN32}>:crypt32.lib>
        )
endif()

#}}}

#{{{ Unit with the builtin cryptographic plugin (DDS:Crypto:AES-GCM-GMAC)

if(SECURITY)
    fastdds_add_unit(security::cryptography
        SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/AESGCMGMAC.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/AESGCMGMAC.h
        ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/AESGCMGMAC_KeyExchange.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/AESGCMGMAC_KeyExchange.h
        ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/AESGCMGMAC_KeyFactory.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/AESGCMGMAC_KeyFactory.h
        ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/AESGCMGMAC_Transform.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/cryptography/AESGCMGMAC_Transform.h
        DEPENDS
        log
        rtps::basic
        security::common
        time
        LINK_LIBRARIES
        PUBLIC
        fastcdr
        OpenSSL::SSL
        OpenSSL::Crypto
        $<$<BOOL:${WIN32}>:crypt32.lib>
        )
endif()

#}}}

#{{{ Unit with the builtin logging plugin (DDS:Logging:DDS_LogTopic)

if(SECURITY)
    fastdds_add_unit(security::log-topic
        SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/security/logging/LogTopic.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/logging/LogTopic.h
        DEPENDS
        log
        rtps::basic
        rtps::security-logging
        time
        LINK_LIBRARIES
        PUBLIC
        fastcdr
        OpenSSL::SSL
        OpenSSL::Crypto
        $<$<BOOL:${WIN32}>:crypt32.lib>
        )
endif()

#}}}

#{{{ Unit with the builtin access control plugin (DDS:Access:Permissions), which parses the governance and permissions
# documents.

if(SECURITY)
    fastdds_add_unit(security::access-control
        SOURCES
        ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/CommonParser.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/CommonParser.h
        ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/GovernanceParser.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/GovernanceParser.h
        ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/Permissions.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/Permissions.h
        ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/PermissionsParser.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/PermissionsParser.h
        ${PROJECT_SOURCE_DIR}/src/cpp/security/accesscontrol/PermissionsTypes.h
        DEPENDS
        log
        rtps::basic
        rtps::security-logging
        rtps::timed-event
        security::common
        time
        xmlparser::common
        INCLUDE_DIRECTORIES
        $<$<BOOL:${TINYXML2_INCLUDE_DIR}>:${TINYXML2_INCLUDE_DIR}>
        LINK_LIBRARIES
        PUBLIC
        fastcdr
        ${TINYXML2_LIBRARY}
        OpenSSL::SSL
        OpenSSL::Crypto
        $<$<BOOL:${WIN32}>:crypt32.lib>
        )
endif()

#}}}
