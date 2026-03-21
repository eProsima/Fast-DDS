# Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

include(CheckCXXSourceCompiles)

# Set source files
if(WIN32)

file(GLOB_RECURSE ALL_HEADERS
    ${PROJECT_SOURCE_DIR}/include/**/*.h
    ${PROJECT_SOURCE_DIR}/include/**/*.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/**/*.h
    ${PROJECT_SOURCE_DIR}/src/cpp/**/*.hpp
    )

add_definitions(-DNOMINMAX)

endif(WIN32)

set(FASTDDS_DIR ${PROJECT_SOURCE_DIR})
set(FASTDDS_SOURCE_DIR .)
include(source.cmake)

foreach(MODULE_DIR ${${PROJECT_NAME}_module_dirs})
    add_subdirectory(${MODULE_DIR})
endforeach()

list(APPEND ${PROJECT_NAME}_source_files
    rtps/domain/RTPSDomainExtras.cpp
    xmlparser/XMLParserExtras.cpp
	)

# Option to enable strict real-time. In this case, several API functions have a real-time behaviour.
# * Publisher::write() - Uses ReliabilityQosPolicy.max_blocking_time
# * Subscriber::takeNextData() - Uses ReliabilityQosPolicy.max_blocking_time
# * Subscriber::readNextData() - Uses ReliabilityQosPolicy.max_blocking_time
option(STRICT_REALTIME "Enable a strict real-time behaviour." OFF)
if(STRICT_REALTIME)
    set(HAVE_STRICT_REALTIME 1)
else()
    set(HAVE_STRICT_REALTIME 0)
endif()

if(NOT ANDROID)
    find_package(Threads REQUIRED)
endif()

if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
endif()

# Find out if libatomic link is required in this platform
find_package(Atomic MODULE)

# Check if the shared_mutex provided by the platform STL library
# prioritizes writes

# try_run cannot manage targets yet
get_target_property(CMAKE_ATOMIC_LIB eProsima_atomic INTERFACE_LINK_LIBRARIES)
if(NOT CMAKE_ATOMIC_LIB)
    set(CMAKE_ATOMIC_LIB)
endif()

try_run(SM_RUN_RESULT SM_COMPILE_RESULT
        "${PROJECT_BINARY_DIR}/shmtest"
        "${PROJECT_SOURCE_DIR}/cmake/modules/check_shared_mutex_priority.cpp"
        LINK_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_ATOMIC_LIB}
        RUN_OUTPUT_VARIABLE SM_RUN_OUTPUT)

if(SM_COMPILE_RESULT AND NOT SM_RUN_RESULT)
    string(STRIP ${SM_RUN_OUTPUT} SM_RUN_OUTPUT)
    message(STATUS "Framework's shared_mutex is ${SM_RUN_OUTPUT}")
endif()

if(SM_RUN_OUTPUT STREQUAL "PTHREAD_RWLOCK_PREFER_READER_NP")
    set(USER_CAN_CHOOSE_SHARED_MUTEX_THIRDPARTY ON)
else()
    message(STATUS "Forcing third party shared_mutex")
    set(USE_THIRDPARTY_SHARED_MUTEX ON)
endif()

cmake_dependent_option(
    USE_THIRDPARTY_SHARED_MUTEX [=[
Forces the use of a Boost-based shared_mutex implementation
instead of the framework one. Useful to cope with issues on
framework implementations like misguided sanitizer reports.
This implementation will be used by default on frameworks
lacking the shared_mutex feature like those not fulfilling
C++17.
]=] OFF
    "USER_CAN_CHOOSE_SHARED_MUTEX_THIRDPARTY"
    ON)

unset(USER_CAN_CHOOSE_SHARED_MUTEX_THIRDPARTY)
unset(SM_RUN_RESULT)
unset(SM_COMPILE_RESULT)
unset(SM_RUN_OUTPUT)
unset(CMAKE_ATOMIC_LIB)

# Generate the proper configure file
configure_file(${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/config.hpp.in
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}/config.hpp)

#Create library
add_library(${PROJECT_NAME} ${${PROJECT_NAME}_source_files})
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )

option(INTERNAL_DEBUG "Activate developer debug messages" OFF)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    ${PROJECT_NAME_UPPER}_SOURCE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    ASIO_DISABLE_VISIBILITY
    EPROSIMA_USER_DLL_EXPORT
    SQLITE_WIN32_GETVERSIONEX=0
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    $<$<AND:$<BOOL:${WIN32}>,$<STREQUAL:"${CMAKE_SYSTEM_NAME}","WindowsStore">>:_WIN32_WINNT=0x0603>
    $<$<AND:$<BOOL:${WIN32}>,$<NOT:$<STREQUAL:"${CMAKE_SYSTEM_NAME}","WindowsStore">>>:_WIN32_WINNT=0x0601>
    $<$<AND:$<BOOL:${WIN32}>,$<STREQUAL:"${CMAKE_SYSTEM_NAME}","WindowsStore">>:SQLITE_OS_WINRT>
    $<$<AND:$<BOOL:${ANDROID}>,$<NOT:$<BOOL:${HAVE_CXX14}>>,$<NOT:$<BOOL:${HAVE_CXX1Y}>>>:ASIO_DISABLE_STD_STRING_VIEW>
    $<$<BOOL:${WIN32}>:_ENABLE_ATOMIC_ALIGNMENT_FIX>
    $<$<NOT:$<BOOL:${IS_THIRDPARTY_BOOST_SUPPORTED}>>:FASTDDS_SHM_TRANSPORT_DISABLED> # Do not compile SHM Transport
    $<$<BOOL:${SHM_TRANSPORT_DEFAULT}>:SHM_TRANSPORT_BUILTIN> # Enable SHM as built-in transport
    $<$<BOOL:${STDOUTERR_LOG_CONSUMER}>:STDOUTERR_LOG_CONSUMER> # Enable StdoutErrConsumer as default LogConsumer
    INTERFACE
    $<$<BOOL:${WIN32}>:${PROJECT_NAME_UPPER}_NO_LIB>
    PUBLIC
    $<$<TARGET_EXISTS:${PROJECT_NAME}>:$<$<STREQUAL:$<TARGET_PROPERTY:${PROJECT_NAME},TYPE>,SHARED_LIBRARY>:${PROJECT_NAME_UPPER}_DYN_LINK>>
    )

# Define public headers
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/cpp>
    $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
    PRIVATE
    ${Asio_INCLUDE_DIR}
    $<$<BOOL:${TINYXML2_INCLUDE_DIR}>:${TINYXML2_INCLUDE_DIR}>
    $<$<BOOL:${ANDROID}>:${ANDROID_IFADDRS_INCLUDE_DIR}>
    ${THIRDPARTY_BOOST_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    )

# Link library to external libraries.
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    fastcdr
    foonathan_memory

    PRIVATE
    eProsima_atomic
    ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS}
    ${TINYXML2_LIBRARY}
    $<$<BOOL:${LINK_SSL}>:OpenSSL::SSL$<SEMICOLON>OpenSSL::Crypto$<$<BOOL:${WIN32}>:$<SEMICOLON>crypt32.lib>>
    $<$<BOOL:${WIN32}>:iphlpapi$<SEMICOLON>Shlwapi>
    ${THIRDPARTY_BOOST_LINK_LIBS}
    )

if (APPLE)
    target_link_libraries(${PROJECT_NAME}
    PRIVATE
    "-framework CoreFoundation"
    "-framework IOKit"
    )
endif()

if(MSVC OR MSVC_IDE)
    # On installed binaries use manifest to specify dependencies
    if(INSTALLER_PLATFORM AND OPENSSL_FOUND)

        # Get Fast DDS version suitable manifest format
        set(FASTDDS_CANONICAL_VERSION ${fastdds_VERSION})

        if( NOT FASTDDS_CANONICAL_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
            message(FATAL_ERROR "Fast DDS version number ${fastdds_VERSION} must include major, minor and patch.")
        endif()

        if( NOT fastdds_VERSION_TWEAK)
            set(FASTDDS_CANONICAL_VERSION "${FASTDDS_CANONICAL_VERSION}.0")
        endif()

        # Get OpenSSL version suitable manifest format
        execute_process( COMMAND PowerShell -NoLogo -Command "&{ param([string]$original)
        if ($original -notmatch '\\d+$')
            { $res = $original.Substring(0,$original.length-1) + '.' +
            ([int]$original[$original.length-1]-[int][char]'a'+1); }
            else { while(($original -split '\\.').count -le 3)
            { $original += '.0'; } $res = $original; } $res }" -original ${OPENSSL_VERSION}
            OUTPUT_VARIABLE OPENSSL_CANONICAL_VERSION
            )
        string(STRIP ${OPENSSL_CANONICAL_VERSION} OPENSSL_CANONICAL_VERSION)

        # replace the values in the manifest template
        configure_file(
            "${PROJECT_SOURCE_DIR}/cmake/packaging/windows/fastdds.manifest.in"
            "${PROJECT_BINARY_DIR}/fastdds.manifest"
            @ONLY
            )

        file(TO_NATIVE_PATH "${PROJECT_BINARY_DIR}/fastdds.manifest" MANIFEST_FILE_PATH)
        target_link_options(${PROJECT_NAME} PRIVATE "/MANIFEST:EMBED" "/MANIFESTINPUT:${MANIFEST_FILE_PATH}")

        unset(WINDOWS_SOURCE_DIR)
        unset(FASTDDS_CANONICAL_VERSION)
        unset(OPENSSL_CANONICAL_VERSION)
        unset(MANIFEST_FILE_PATH)
    endif()

    set_target_properties(${PROJECT_NAME} PROPERTIES
        RELEASE_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        RELWITHDEBINFO_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        DEBUG_POSTFIX d-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        )

    get_target_property(TARGET_TYPE ${PROJECT_NAME} TYPE)
    if(TARGET_TYPE STREQUAL "SHARED_LIBRARY")
        # Export symbols in DLL library
        set_target_properties(${PROJECT_NAME} PROPERTIES
            PDB_NAME_DEBUG ${PROJECT_NAME}d-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
            PDB_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/symbols"
            )
    else()
        # Rename the library to have a "lib" before.
        set_target_properties(${PROJECT_NAME} PROPERTIES
            OUTPUT_NAME lib${PROJECT_NAME}
            COMPILE_PDB_NAME_DEBUG lib${PROJECT_NAME}d-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
            COMPILE_PDB_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/symbols"
            )
    endif()

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
        set_target_properties(${PROJECT_NAME} PROPERTIES VS_WINRT_COMPONENT "true")
    endif()
elseif(BUILD_SHARED_LIBS)
    set(VERSION_SCRIPT_SUPPORT_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libfastdds.version)
    set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
    set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS} ${VERSION_SCRIPT_SUPPORT_FLAGS})
    CHECK_CXX_SOURCE_COMPILES("int main(int, char**){return 0;}" HAS_VERSION_SCRIPT_SUPPORT)
    set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
    unset(OLD_CMAKE_REQUIRED_FLAGS)
    if(HAS_VERSION_SCRIPT_SUPPORT)
        message(STATUS "Using linker version script to select exported symbols")
        target_link_options(${PROJECT_NAME} PRIVATE ${VERSION_SCRIPT_SUPPORT_FLAGS})
    endif()
endif()

###############################################################################
# Packaging
###############################################################################

# Install public headers

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT headers
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
    PATTERN "*.ipp"
    )

# Install config.hpp header
install(FILES ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}/config.hpp
    DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}
    COMPONENT headers
    )

# Install public statistics idls
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/fastdds/statistics
    DESTINATION ${INCLUDE_INSTALL_DIR}/fastdds
    FILES_MATCHING
    PATTERN "*.idl"
    )

# Install libraries

install(TARGETS ${PROJECT_NAME} eProsima_atomic
    EXPORT ${PROJECT_NAME}-targets
    COMPONENT libraries
    RUNTIME DESTINATION "${BIN_INSTALL_DIR}${MSVCARCH_DIR_EXTENSION}"
    LIBRARY DESTINATION "${LIB_INSTALL_DIR}${MSVCARCH_DIR_EXTENSION}"
    ARCHIVE DESTINATION "${LIB_INSTALL_DIR}${MSVCARCH_DIR_EXTENSION}"
    )

# Generate different target names depending on linking
get_target_property(TARGET_TYPE ${PROJECT_NAME} TYPE)
if(TARGET_TYPE STREQUAL "SHARED_LIBRARY")
    set(FASTDDS_LINKING shared)
else()
    set(FASTDDS_LINKING static)
endif()

if(INSTALLER_PLATFORM)
    set(INSTALL_DESTINATION_PATH ${DATA_INSTALL_DIR}/${PROJECT_NAME}-${INSTALLER_PLATFORM}/cmake)
else()
    set(INSTALL_DESTINATION_PATH ${DATA_INSTALL_DIR}/${PROJECT_NAME}/cmake${MSVCARCH_DIR_EXTENSION_EXT})
endif()

install(EXPORT ${PROJECT_NAME}-targets
    DESTINATION ${INSTALL_DESTINATION_PATH}
    FILE ${PROJECT_NAME}-${FASTDDS_LINKING}-targets.cmake
    COMPONENT cmake
    )

if(MSVC OR MSVC_IDE)

    # first try dll symbols
    get_target_property(PDB_FILE ${PROJECT_NAME} PDB_NAME_DEBUG)
    if(PDB_FILE)
        get_target_property(PDB_DIR ${PROJECT_NAME} PDB_OUTPUT_DIRECTORY_DEBUG)
        set(PDB_FILE "${PDB_DIR}/${PDB_FILE}.pdb")
    else()
        # fallback to static lib symbols
        get_target_property(MSVC_DEBUG_INFORMATION_FORMAT ${PROJECT_NAME} MSVC_DEBUG_INFORMATION_FORMAT)
        if(NOT MSVC_DEBUG_INFORMATION_FORMAT MATCHES ".*Embedded.*")
            get_target_property(PDB_FILE ${PROJECT_NAME} COMPILE_PDB_NAME_DEBUG)
            if(PDB_FILE)
                get_target_property(PDB_DIR ${PROJECT_NAME} COMPILE_PDB_OUTPUT_DIRECTORY_DEBUG)
                set(PDB_FILE "${PDB_DIR}/${PDB_FILE}.pdb")
            endif()
        endif()
    endif()

    # install symbols if any
    if(PDB_FILE)
        install(FILES ${PDB_FILE}
            DESTINATION ${LIB_INSTALL_DIR}${MSVCARCH_DIR_EXTENSION}
            COMPONENT symbols
            CONFIGURATIONS Debug
            )
    endif()
endif()

###############################################################################
# Create CMake package config file
###############################################################################
include(CMakePackageConfigHelpers)

if(BUILD_SHARED_LIBS)
    set(FASTDDS_PACKAGE_OPT_BIN_DIR_CONDITION "if(MSVC OR MSVC_IDE)")
else()
    set(FASTDDS_PACKAGE_OPT_BIN_DIR_CONDITION "if(0)")
endif()

# include the option parser targets
set(INCLUDE_FASTDDS_TARGETS "include(\${CMAKE_CURRENT_LIST_DIR}/optionparser-targets.cmake)")

# Prepare dependencies for static linking.
set(FASTDDS_STATIC_LINK_ONLY_OPT_DEPS "")
if (LINK_SSL)
    set(FASTDDS_STATIC_LINK_ONLY_OPT_DEPS "${FASTDDS_STATIC_LINK_ONLY_OPT_DEPS}\nfind_dependency(OpenSSL REQUIRED)")
endif()
if (TARGET tinyxml2::tinyxml2)
    set(FASTDDS_STATIC_LINK_ONLY_OPT_DEPS "${FASTDDS_STATIC_LINK_ONLY_OPT_DEPS}\nfind_dependency(TinyXml2 REQUIRED)")
endif()

configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/packaging/Config.cmake.in
    ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${INSTALL_DESTINATION_PATH}
    PATH_VARS BIN_INSTALL_DIR INCLUDE_INSTALL_DIR LIB_INSTALL_DIR
    )
write_basic_package_version_file(${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
    )
install(FILES ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config.cmake
    ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config-version.cmake
    DESTINATION ${INSTALL_DESTINATION_PATH}
    COMPONENT cmake
    )
