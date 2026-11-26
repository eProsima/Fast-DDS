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

# Link-seam mock: compiled against the real SecurityPluginFactory.h, for unit tests not linking core, where
# SecurityPluginFactory.cpp is. Plugins are not built from the properties: each factory method hands out, once, the
# plugin a test set with SecurityPluginFactoryMock. Also provides the gmock implementations of the plugin interfaces.
add_mock_library(SecurityPluginFactory
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/rtps/security/MockAccessControlPlugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/rtps/security/MockAuthenticationPlugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/rtps/security/MockCryptographyPlugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/rtps/security/MockCryptoKeyExchange.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/rtps/security/MockCryptoKeyFactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/rtps/security/MockCryptoTransform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/rtps/security/SecurityPluginFactoryMock.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SecurityPluginFactory.cpp
    )
