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

macro(check_stdcxx)

    # Note that FASTDDS_REQUIRED_FLAGS provides access to the compiler flags in the try-compile testing like
    # CheckLibatomic module 
    include(CheckCXXCompilerFlag)

    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
        CMAKE_CXX_COMPILER_ID MATCHES "QCC")
        check_cxx_compiler_flag(-std=c++20 SUPPORTS_CXX20)
        set(HAVE_CXX20 0)
        set(HAVE_CXX17 0)
        set(HAVE_CXX14 0)
        set(HAVE_CXX1Y 0)
        set(HAVE_CXX11 0)
        set(HAVE_CXX0X 0)
        if(SUPPORTS_CXX20 AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "20"))
            set(FASTDDS_REQUIRED_FLAGS "-std=c++20 ${FASTDDS_REQUIRED_FLAGS}")
            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++20>)
            set(HAVE_CXX20 1)
            set(HAVE_CXX17 1)
            set(HAVE_CXX14 1)
            set(HAVE_CXX1Y 1)
            set(HAVE_CXX11 1)
            set(HAVE_CXX0X 1)
        elseif(NOT SUPPORTS_CXX20 AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "20")
            message(FATAL_ERROR "Force to support stdc++20 but not supported by the compiler")
        else()
            check_cxx_compiler_flag(-std=c++17 SUPPORTS_CXX17)
            if(SUPPORTS_CXX17 AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "17"))
                set(FASTDDS_REQUIRED_FLAGS "-std=c++17 ${FASTDDS_REQUIRED_FLAGS}")
                add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++17>)
                set(HAVE_CXX17 1)
                set(HAVE_CXX14 1)
                set(HAVE_CXX1Y 1)
                set(HAVE_CXX11 1)
                set(HAVE_CXX0X 1)
            elseif(NOT SUPPORTS_CXX17 AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "17")
                message(FATAL_ERROR "Force to support stdc++17 but not supported by the compiler")
            else()
                check_cxx_compiler_flag(-std=c++14 SUPPORTS_CXX14)
                if(SUPPORTS_CXX14 AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "14"))
                    set(FASTDDS_REQUIRED_FLAGS "-std=c++14 ${FASTDDS_REQUIRED_FLAGS}")
                    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++14>)
                    set(HAVE_CXX14 1)
                    set(HAVE_CXX1Y 1)
                    set(HAVE_CXX11 1)
                    set(HAVE_CXX0X 1)
                elseif(NOT SUPPORTS_CXX14 AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "14")
                    message(FATAL_ERROR "Force to support stdc++14 but not supported by the compiler")
                else()
                    check_cxx_compiler_flag(-std=c++1y SUPPORTS_CXX1Y)
                    if(SUPPORTS_CXX1Y AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "1Y"))
                        set(FASTDDS_REQUIRED_FLAGS "-std=c++1y ${FASTDDS_REQUIRED_FLAGS}")
                        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++1y>)
                        set(HAVE_CXX1Y 1)
                        set(HAVE_CXX11 1)
                        set(HAVE_CXX0X 1)
                    elseif(NOT SUPPORTS_CXX1Y AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "1Y")
                        message(FATAL_ERROR "Force to support stdc++1y but not supported by the compiler")
                    else()
                        check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
                        if(SUPPORTS_CXX11 AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "11"))
                            set(FASTDDS_REQUIRED_FLAGS "-std=c++11 ${FASTDDS_REQUIRED_FLAGS}")
                            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++11>)
                            set(HAVE_CXX11 1)
                            set(HAVE_CXX0X 1)
                        elseif(NOT SUPPORTS_CXX11 AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "11")
                            message(FATAL_ERROR "Force to support stdc++11 but not supported by the compiler")
                        else()
                            check_cxx_compiler_flag(-std=c++0x SUPPORTS_CXX0X)
                            if(SUPPORTS_CXX0X AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "0X"))
                                set(FASTDDS_REQUIRED_FLAGS "-std=c++0x ${FASTDDS_REQUIRED_FLAGS}")
                                add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++0x>)
                                set(HAVE_CXX0X 1)
                            elseif(NOT SUPPORTS_CXX0X AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "0X")
                                message(FATAL_ERROR "Force to support stdc++0x but not supported by the compiler")
                            else()
                                set(HAVE_CXX0X 0)
                            endif() # c++0x
                        endif() # C++11
                    endif() # C++1Y
                endif() # C++14
            endif() # C++17
        endif() # C++20
    elseif(MSVC OR MSVC_IDE)
        check_cxx_compiler_flag(/std:c++20 SUPPORTS_CXX20)
        set(HAVE_CXX20 0)
        set(HAVE_CXX17 0)
        set(HAVE_CXX14 0)
        set(HAVE_CXX11 0)
        set(HAVE_CXX0X 0)
        if(SUPPORTS_CXX20 AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "20"))
            set(FASTDDS_REQUIRED_FLAGS "/std:c++20 ${FASTDDS_REQUIRED_FLAGS}")
            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/std:c++20>)
            set(HAVE_CXX20 1)
            set(HAVE_CXX17 1)
            set(HAVE_CXX14 1)
            set(HAVE_CXX11 1)
            set(HAVE_CXX0X 1)
        elseif(NOT SUPPORTS_CXX20 AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "20")
            message(FATAL_ERROR "Force to support stdc++20 but not supported by the compiler")
        else()
            check_cxx_compiler_flag(/std:c++17 SUPPORTS_CXX17)
            if(SUPPORTS_CXX17 AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "17"))
                set(FASTDDS_REQUIRED_FLAGS "/std:c++17 ${FASTDDS_REQUIRED_FLAGS}")
                add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/std:c++17>)
                set(HAVE_CXX17 1)
                set(HAVE_CXX14 1)
                set(HAVE_CXX11 1)
                set(HAVE_CXX0X 1)
            elseif(NOT SUPPORTS_CXX17 AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "17")
                message(FATAL_ERROR "Force to support stdc++17 but not supported by the compiler")
            else()
                check_cxx_compiler_flag(/std:c++14 SUPPORTS_CXX14)
                if(SUPPORTS_CXX14 AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "14"))
                    set(FASTDDS_REQUIRED_FLAGS "/std:c++14 ${FASTDDS_REQUIRED_FLAGS}")
                    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/std:c++14>)
                    set(HAVE_CXX14 1)
                    set(HAVE_CXX11 1)
                    set(HAVE_CXX0X 1)
                elseif(NOT SUPPORTS_CXX14 AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "14")
                    message(FATAL_ERROR "Force to support stdc++14 but not supported by the compiler")
                else()
                    message(STATUS "The specified C++${FORCE_CXX} option is not supported using compiler default.")
                    set(HAVE_CXX11 1)
                    set(HAVE_CXX0X 1)
                endif() # C++14
            endif() # C++17
        endif() # C++20
    else()
        set(HAVE_CXX11 0)
        set(HAVE_CXX0X 0)
    endif()
endmacro()

macro(check_compile_feature)
    # Check constexpr
    list(FIND CMAKE_CXX_COMPILE_FEATURES "cxx_constexpr" CXX_CONSTEXPR_SUPPORTED)
    if(${CXX_CONSTEXPR_SUPPORTED} GREATER -1)
        set(HAVE_CXX_CONSTEXPR 1)
    else()
        set(HAVE_CXX_CONSTEXPR 0)
    endif()
endmacro()

macro(check_endianness)
    # Test endianness
    include(TestBigEndian)
    test_big_endian(BIG_ENDIAN)
    set(FASTDDS_IS_BIG_ENDIAN_TARGET ${BIG_ENDIAN})
endmacro()
