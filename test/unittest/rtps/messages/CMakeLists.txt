# Copyright (C) 2026, Proyectos y Sistemas de Mantenimiento SL (eProsima)
#
# This program is commercial software licensed under the terms of the
# eProsima Software License Agreement Rev 03 (the "License")
#
# You may obtain a copy of the License at
# https://www.eprosima.com/licenses/LICENSE-REV03

#######################
# MaxAsioBuffers test #
#######################

set(MAXASIOBUFFERSTESTS_SOURCE MaxAsioBuffersTests.cpp)

add_executable(MaxAsioBuffersTests ${MAXASIOBUFFERSTESTS_SOURCE})
target_compile_definitions(MaxAsioBuffersTests PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(MaxAsioBuffersTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp)
target_link_libraries(MaxAsioBuffersTests
    GTest::gtest
    $<$<BOOL:${WIN32}>:ws2_32>
    $<$<BOOL:${QNX}>:socket>
    )
gtest_discover_tests(MaxAsioBuffersTests PROPERTIES LABELS "NoMemoryCheck")
