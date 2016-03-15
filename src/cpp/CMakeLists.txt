include(${PROJECT_SOURCE_DIR}/cmake/dev/set_sources.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/dev/generate_msvc_libraries.cmake)

# Set source files
set_sources(
    log/Log.cpp
    utils/eClock.cpp 
    utils/IPFinder.cpp 
    utils/md5.cpp 
    utils/StringMatching.cpp 
    rtps/resources/ListenResourceImpl.cpp 
    rtps/resources/ListenResource.cpp 
    rtps/resources/ResourceSend.cpp 
    rtps/resources/ResourceSendImpl.cpp 
    rtps/resources/ResourceEvent.cpp 
    rtps/resources/TimedEvent.cpp 
    rtps/resources/TimedEventImpl.cpp 
    rtps/resources/AsyncWriterThread.cpp
    rtps/Endpoint.cpp 
    rtps/writer/RTPSWriter.cpp 
    rtps/writer/StatefulWriter.cpp 
    rtps/writer/ReaderProxy.cpp 
    rtps/writer/StatelessWriter.cpp 
    rtps/writer/ReaderLocator.cpp 
    rtps/writer/timedevent/PeriodicHeartbeat.cpp 
    rtps/writer/timedevent/NackResponseDelay.cpp 
    rtps/writer/timedevent/NackSupressionDuration.cpp 
    rtps/writer/timedevent/UnsentChangesNotEmptyEvent.cpp 
    rtps/history/CacheChangePool.cpp 
    rtps/history/History.cpp 
    rtps/history/WriterHistory.cpp 
    rtps/history/ReaderHistory.cpp 
    rtps/reader/timedevent/HeartbeatResponseDelay.cpp 
    rtps/reader/timedevent/WriterProxyLiveliness.cpp 
    rtps/reader/WriterProxy.cpp 
    rtps/reader/StatefulReader.cpp 
    rtps/reader/StatelessReader.cpp 
    rtps/reader/RTPSReader.cpp 
    rtps/messages/CDRMessagePool.cpp 
    rtps/messages/RTPSMessageCreator.cpp 
    rtps/messages/RTPSMessageGroup.cpp 
    rtps/messages/MessageReceiver.cpp
    rtps/messages/submessages/AckNackMsg.hpp
    rtps/messages/submessages/DataMsg.hpp
    rtps/messages/submessages/GapMsg.hpp
    rtps/messages/submessages/HeartbeatMsg.hpp
    rtps/participant/RTPSParticipant.cpp 
    rtps/participant/RTPSParticipantImpl.cpp 
    rtps/RTPSDomain.cpp 
    Domain.cpp 
    participant/Participant.cpp 
    participant/ParticipantImpl.cpp 
    publisher/Publisher.cpp 
    publisher/PublisherImpl.cpp 
    publisher/PublisherHistory.cpp 
    subscriber/Subscriber.cpp 
    subscriber/SubscriberImpl.cpp 
    subscriber/SubscriberHistory.cpp 
    qos/ParameterList.cpp 
    qos/ParameterTypes.cpp 
    qos/QosList.cpp 
    qos/QosPolicies.cpp 
    qos/WriterQos.cpp 
    qos/ReaderQos.cpp 
    rtps/builtin/BuiltinProtocols.cpp 
    rtps/builtin/discovery/participant/PDPSimple.cpp 
    rtps/builtin/discovery/participant/PDPSimpleListener.cpp 
    rtps/builtin/discovery/participant/timedevent/RemoteParticipantLeaseDuration.cpp 
    rtps/builtin/discovery/participant/timedevent/ResendParticipantProxyDataPeriod.cpp 
    rtps/builtin/discovery/endpoint/EDP.cpp 
    rtps/builtin/discovery/endpoint/EDPSimple.cpp 
    rtps/builtin/discovery/endpoint/EDPSimpleListeners.cpp 
    rtps/builtin/discovery/endpoint/EDPStaticXML.cpp 
    rtps/builtin/discovery/endpoint/EDPStatic.cpp 
    rtps/builtin/liveliness/WLP.cpp 
    rtps/builtin/liveliness/WLPListener.cpp 
    rtps/builtin/liveliness/timedevent/WLivelinessPeriodicAssertion.cpp 
    rtps/builtin/data/ParticipantProxyData.cpp 
    rtps/builtin/data/WriterProxyData.cpp 
    rtps/builtin/data/ReaderProxyData.cpp 
    fastrtps.rc
    )

# Set version
set(${PROJECT_NAME_UPPER}_VERSION_MAJOR ${PROJECT_MAJOR_VERSION})
set(${PROJECT_NAME_UPPER}_VERSION_MINOR ${PROJECT_MINOR_VERSION})
set(${PROJECT_NAME_UPPER}_VERSION_MICRO ${PROJECT_MICRO_VERSION})
set(${PROJECT_NAME_UPPER}_VERSION_STR ${PROJECT_VERSION})

configure_file(${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/config.h.in
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}/config.h)

# Definitions
# If Windows, set define for export dll functions
add_definitions(-D${PROJECT_NAME_UPPER}_SOURCE)
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0501)
endif()

# Include Boost header files.
include_directories(${Boost_INCLUDE_DIR})

# Create libraries
if(MSVC OR MSVC_IDE)
    if(EPROSIMA_INSTALLER)
        generate_msvc_libraries(i86Win32VS2010)
        generate_msvc_libraries(x64Win64VS2010)
        generate_msvc_libraries(i86Win32VS2013)
        generate_msvc_libraries(x64Win64VS2013)
        generate_msvc_libraries(i86Win32VS2015)
        generate_msvc_libraries(x64Win64VS2015)
    else()
        add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES})

        add_library(${PROJECT_NAME}_static STATIC ${${PROJECT_NAME}_SOURCES})
        # Rename the library to have a "lib" before.
        set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME lib${PROJECT_NAME})

        set_target_properties(${PROJECT_NAME} ${PROJECT_NAME}_static PROPERTIES RELEASE_POSTFIX -${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION})
        set_target_properties(${PROJECT_NAME} ${PROJECT_NAME}_static PROPERTIES RELWITHDEBINFO_POSTFIX -${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION})
        set_target_properties(${PROJECT_NAME} ${PROJECT_NAME}_static PROPERTIES DEBUG_POSTFIX d-${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION})

        # Export symbols in DLL library
        target_compile_definitions(${PROJECT_NAME} PUBLIC BOOST_ALL_DYN_LINK ${PROJECT_NAME_UPPER}_DYN_LINK)
        target_compile_definitions(${PROJECT_NAME}_static PUBLIC BOOST_ALL_DYN_LINK)

        # Define public headers
        target_include_directories(${PROJECT_NAME} PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}>
            $<INSTALL_INTERFACE:include>
            )

        # Define public headers for static library
        target_include_directories(${PROJECT_NAME}_static PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}>
            $<INSTALL_INTERFACE:include>
            )

        # Link library to external win32 libraries.
        target_link_libraries(${PROJECT_NAME}
            ${Boost_LIBRARIES} iphlpapi Shlwapi
            )
    endif()
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-D__DEBUG)
    endif()

    #Create library
    add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES})

    # Define public headers
    target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}>
        $<INSTALL_INTERFACE:include>
        )

    # Link library to external libraries.
    target_link_libraries(${PROJECT_NAME}
        ${Boost_LIBRARIES}
        )
endif()

###############################################################################
# Packaging
###############################################################################

# Install public headers
set_public_headers_directory(${PROJECT_SOURCE_DIR}/include ${PROJECT_NAME}
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT headers
    )

# Install config.h header
set_public_header(${PROJECT_BINARY_DIR}/include ${PROJECT_NAME} config.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}
    COMPONENT headers
    )

set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers" PARENT_SCOPE)
set(CPACK_COMPONENT_HEADERS_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} C++ Headers" PARENT_SCOPE)

# Install sources
if(UNIX AND EPROSIMA_INSTALLER)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/cpp
        DESTINATION src
        COMPONENT sources
        FILES_MATCHING
        PATTERN "*.c"
        PATTERN "*.cpp"
        PATTERN "*.h"
        PATTERN "*.hpp"
        )

    install(DIRECTORY ${PROJECT_BINARY_DIR}/autotools/
        DESTINATION .
        USE_SOURCE_PERMISSIONS
        OPTIONAL
        COMPONENT sources
        )
endif()

# Install libraries
if(MSVC OR MSVC_IDE)
    if(EPROSIMA_INSTALLER)
        install_msvc_libraries(i86Win32VS2010)
        install_msvc_libraries(x64Win64VS2010)
        install_msvc_libraries(i86Win32VS2013)
        install_msvc_libraries(x64Win64VS2013)
        install_msvc_libraries(i86Win32VS2015)
        install_msvc_libraries(x64Win64VS2015)
    else()
        install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_static
            EXPORT ${PROJECT_NAME}Targets
            RUNTIME DESTINATION ${BIN_INSTALL_DIR}
            LIBRARY DESTINATION ${LIB_INSTALL_DIR}
            ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
            COMPONENT libraries_${MSVC_ARCH}
            )

        export(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_static FILE ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}Targets.cmake)
        install(EXPORT ${PROJECT_NAME}Targets
            DESTINATION ${LIB_INSTALL_DIR}/${PROJECT_NAME}/cmake
            COMPONENT cmake
            )


        # Install pdb files
        install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Debug/
            DESTINATION ${LIB_INSTALL_DIR}
            COMPONENT libraries_${MSVC_ARCH}
            CONFIGURATIONS Debug
            FILES_MATCHING
            PATTERN "*.pdb"
            )

        string(TOUPPER "${MSVC_ARCH}" MSVC_ARCH_UPPER)
        set(CPACK_COMPONENT_LIBRARIES_${MSVC_ARCH_UPPER}_DISPLAY_NAME "${MSVC_ARCH}" PARENT_SCOPE)
        set(CPACK_COMPONENT_LIBRARIES_${MSVC_ARCH_UPPER}_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} libraries for platform ${MSVC_ARCH}" PARENT_SCOPE)
        set(CPACK_COMPONENT_LIBRARIES_${MSVC_ARCH_UPPER}_GROUP "Libraries" PARENT_SCOPE)
    endif()

    set(CPACK_COMPONENT_GROUP_LIBRARIES_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} libraries" PARENT_SCOPE)
else()
    install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        RUNTIME DESTINATION ${BIN_INSTALL_DIR}
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
        COMPONENT libraries
        )

    export(TARGETS ${PROJECT_NAME} FILE ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}Targets.cmake)
    install(EXPORT ${PROJECT_NAME}Targets
        DESTINATION ${LIB_INSTALL_DIR}/${PROJECT_NAME}/cmake
        COMPONENT cmake
        )

    set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries" PARENT_SCOPE)
    set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} libraries" PARENT_SCOPE)
endif()
