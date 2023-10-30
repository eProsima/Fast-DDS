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
if(WIN32)
    add_definitions(
        -D_WIN32_WINNT=0x0601
        -D_CRT_SECURE_NO_WARNINGS
        )
endif()

###############################################################################
# Unit tests
###############################################################################
set(XTYPES_TESTS_SOURCE XTypesTests.cpp
    idl/Types.cxx
    idl/Typesv1.cxx
    idl/TypesPubSubTypes.cxx
    idl/TypesTypeObject.cxx
    TestPublisher.cpp
    TestSubscriber.cpp
    )

add_executable(XTypesBlackBoxTests ${XTYPES_TESTS_SOURCE})
target_compile_definitions(XTypesBlackBoxTests PRIVATE
    EPROSIMA_SHAPES_DLL_EXPORT
    BUILD_SHARED_LIBS
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_include_directories(XTypesBlackBoxTests PRIVATE ${Asio_INCLUDE_DIR})
target_link_libraries(XTypesBlackBoxTests fastrtps fastcdr GTest::gtest)
gtest_discover_tests(XTypesBlackBoxTests)
