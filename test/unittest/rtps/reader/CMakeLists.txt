# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

###########################################################################
# WriterProxyTests
###########################################################################
set(WRITERPROXYTESTS_SOURCE WriterProxyTests.cpp
    )

add_executable(WriterProxyTests ${WRITERPROXYTESTS_SOURCE})
target_link_units(WriterProxyTests UNITS rtps::external-locators rtps::common xtypes rtps::builtin-data
    rtps::resource-event rtps::history MOCKS BaseReader StatefulReader BaseWriter MessageReceiver
    RTPSDomainImpl xtypes::TypeObjectUtils TimedEvent ReaderHistory WriterHistory)
target_include_directories(WriterProxyTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    )
target_link_libraries(WriterProxyTests
    fastcdr
    foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS})
gtest_discover_tests(WriterProxyTests)

###########################################################################
# WriterProxyStopTest
###########################################################################
set(WRITERPROXYSTOPTEST_SOURCE WriterProxyStopTest.cpp
    )

add_executable(WriterProxyStopTest ${WRITERPROXYSTOPTEST_SOURCE})
target_link_units(WriterProxyStopTest UNITS rtps::external-locators rtps::common rtps::timed-event xtypes
    rtps::builtin-data rtps::history MOCKS BaseReader StatefulReader BaseWriter MessageReceiver
    RTPSDomainImpl xtypes::TypeObjectUtils ReaderHistory WriterHistory)
target_include_directories(WriterProxyStopTest PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    )
target_link_libraries(WriterProxyStopTest
    fastcdr
    foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS})
gtest_discover_tests(WriterProxyStopTest)

###########################################################################
# WriterProxyAcknackTests
###########################################################################
set(WRITERPROXYACKNACKTESTS_SOURCE WriterProxyAcknackTests.cpp)

add_executable(WriterProxyAcknackTests ${WRITERPROXYACKNACKTESTS_SOURCE})
target_link_units(WriterProxyAcknackTests UNITS rtps::common xtypes rtps::builtin-data rtps::network-factory
    rtps::parameter-list rtps::timed-event rtps::proxy-data rtps::external-locators rtps::history MOCKS
    BaseReader StatefulReader BaseWriter xtypes::TypeObjectUtils MessageReceiver RTPSDomainImpl ReaderHistory
    WriterHistory)
target_include_directories(WriterProxyAcknackTests PRIVATE
    ${Asio_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/src/cpp/rtps/reader
    ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    )
target_link_libraries(WriterProxyAcknackTests
    fastcdr
    foonathan_memory
    GTest::gmock
    ${CMAKE_DL_LIBS})
gtest_discover_tests(WriterProxyAcknackTests)

get_property(fastdds_include_directories TARGET ${PROJECT_NAME} PROPERTY INCLUDE_DIRECTORIES)

###########################################################################
# StatefulReaderTests
###########################################################################
set(STATEFUL_READER_TESTS_SOURCE StatefulReaderTests.cpp)

add_executable(StatefulReaderTests ${STATEFUL_READER_TESTS_SOURCE})


target_link_units(StatefulReaderTests UNITS core)
target_include_directories(StatefulReaderTests PRIVATE
    ${fastdds_include_directories}
    )
target_link_libraries(StatefulReaderTests
    GTest::gmock
    )
gtest_discover_tests(StatefulReaderTests)

###########################################################################
# StatelessReaderTests
###########################################################################
set(STATELESS_READER_TESTS_SOURCE StatelessReaderTests.cpp)

add_executable(StatelessReaderTests ${STATELESS_READER_TESTS_SOURCE})


target_link_units(StatelessReaderTests UNITS core)
target_include_directories(StatelessReaderTests PRIVATE
    ${fastdds_include_directories}
    )
target_link_libraries(StatelessReaderTests
    GTest::gmock
    )
gtest_discover_tests(StatelessReaderTests)
