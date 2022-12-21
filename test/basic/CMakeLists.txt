# Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

find_package(GTest CONFIG REQUIRED)

add_executable(basic_dds_tests
    BasicDDSTests.cpp
    types/FixedSized.cxx
    types/FixedSizedPubSubTypes.cxx
    types/StringTest.cxx
    types/StringTestPubSubTypes.cxx
)

target_link_libraries(
    basic_dds_tests
    GTest::gtest
    fastcdr
    fastrtps
    foonathan_memory
)

set(TEST_LIST
    test_fixed_intraprocess_reliable_datasharing
    test_string_intraprocess_reliable_datasharing
    test_fixed_intraprocess_reliable
    test_string_intraprocess_reliable
    test_fixed_reliable
    test_strings_reliable
    test_fixed_datasharing
    test_string_datasharing
)

foreach(test_name ${TEST_LIST})
    add_test(NAME basic_dds_tests.${test_name}
        COMMAND
            basic_dds_tests
            --gtest_filter=basic_dds_tests.${test_name}:**/basic_dds_tests.${test_name}/**)
endforeach()
