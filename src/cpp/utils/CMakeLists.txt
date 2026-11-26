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

#{{{ Unit grouping the Time_t source code

fastdds_add_unit(time
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/core/Time_t.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/common/Time_t.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/core/Time_t.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/Time_t.cpp
    LINK_LIBRARIES
    PRIVATE
    fastcdr
    )

#}}}

#{{{ Unit grouping the network utilities source code (IP addresses, locators and network interfaces)

fastdds_add_unit(utils::network
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/common/LocatorWithMask.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/rtps/transport/network/NetmaskFilterKind.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/utils/IPFinder.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/utils/IPLocator.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/utils/md5.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/common/LocatorWithMask.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/netmask_filter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/network/utils/network.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetmaskFilterKind.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterface.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/transport/network/NetworkInterfaceWithFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/Host.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPFinder.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/IPLocator.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/md5.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/SystemInfo.cpp
    DEPENDS
    log
    INCLUDE_DIRECTORIES
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    ${Asio_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi> # Used by IPFinder
    )

#}}}

#{{{ Unit with the shared memory segments used by the shared memory transport and data sharing.
# A unit test not linking core replaces it with the SharedMemSegment mock.

fastdds_add_unit(utils::shared-memory
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/shared_memory/BoostAtExitRegistry.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/shared_memory/RobustInterprocessCondition.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/shared_memory/SharedMemSegment.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/shared_memory/SharedMemSegment.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/utils/shared_memory/SharedMemUUID.hpp
    DEPENDS
    log
    INCLUDE_DIRECTORIES
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    ${THIRDPARTY_BOOST_LINK_LIBS}
    )

#}}}
