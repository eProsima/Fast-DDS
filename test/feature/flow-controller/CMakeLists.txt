# Copyright (C) 2025, Proyectos y Sistemas de Mantenimiento SL (eProsima)
#
# This program is commercial software licensed under the terms of the
# eProsima Software License Agreement Rev 03 (the "License")
#
# You may obtain a copy of the License at
# https://www.eprosima.com/licenses/LICENSE-REV03

find_package(ZLIB)
if (NOT ZLIB_FOUND)
    find_package(PkgConfig)
    pkg_search_module(ZLIB zlib IMPORTED_TARGET)
    if (ZLIB_FOUND)
        add_library(ZLIB::ZLIB ALIAS PkgConfig::ZLIB)
    endif()
endif()
if(ZLIB_FOUND)
    set(HAVE_ZLIB 1)
else()
    if (ZLIB_REQUIRED)
        message(FATAL_ERROR "ZLib library not found! and it is required")
    else()
        set(HAVE_ZLIB 0)
    endif()
endif()

# Configure BZIP2
find_package(BZip2)
if(NOT BZIP2_FOUND)
    find_package(PkgConfig)
    pkg_search_module(BZip2 bzip2 IMPORTED_TARGET)
    set(BZIP2_FOUND ${BZip2_FOUND})
    if (BZIP2_FOUND)
        add_library(BZip2::BZip2 ALIAS PkgConfig::BZip2)
    endif()
endif()
if(BZIP2_FOUND)
    set(HAVE_BZIP2 1)
else()
    if (BZIP2_REQUIRED)
        message(FATAL_ERROR "Bzip2 library not found! and it is required")
    else()
        set(HAVE_BZIP2 0)
    endif()
endif()

get_property(fastdds_sources_files TARGET ${PROJECT_NAME} PROPERTY SOURCES)
get_property(fastdds_include_directories TARGET ${PROJECT_NAME} PROPERTY INCLUDE_DIRECTORIES)
get_property(fastdds_link_libraries TARGET ${PROJECT_NAME} PROPERTY LINK_LIBRARIES)

add_executable(grained_flow_controller_tests
    grained_flow_controller_tests.cpp

    FlowControllerFactoryMock.cpp
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/Data1mbPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/Data1mbTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/test/blackbox/types/HelloWorldTypeObjectSupport.cxx
)

target_compile_definitions(grained_flow_controller_tests
    PRIVATE
    FASTDDS_SOURCE
    $<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS>
    $<$<BOOL:${WIN32}>:NOMINMAX>
    $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${ZLIB_FOUND}>>:ZLIB_DLL>
    $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${BZIP2_FOUND}>>:BZ_IMPORT>
)

target_sources(grained_flow_controller_tests
    PRIVATE
    ${fastdds_sources_files}
)

target_include_directories(grained_flow_controller_tests
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/test/blackbox/types
    ${fastdds_include_directories}
)

target_link_libraries(grained_flow_controller_tests
    PRIVATE
    $<$<BOOL:${ZLIB_FOUND}>:ZLIB::ZLIB>
    $<$<BOOL:${BZIP2_FOUND}>:BZip2::BZip2>
    GTest::gtest
    ${fastdds_link_libraries}
)

target_link_directories(${PROJECT_NAME} PRIVATE
    $<$<BOOL:${BZIP2_FOUND}>:${BZip2_LIBRARY_DIRS}>
    )

gtest_discover_tests(grained_flow_controller_tests
    PROPERTIES
            ENVIRONMENT "CERTS_PATH=${CMAKE_SOURCE_DIR}/test/certs"
)

