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

macro(check_stdcxx_old)

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
        if(SUPPORTS_CXX20 AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "20"))
            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++20>)
            set(HAVE_CXX20 1)
            set(HAVE_CXX17 1)
            set(HAVE_CXX14 1)
            set(HAVE_CXX1Y 1)
        elseif(NOT SUPPORTS_CXX20 AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "20")
            message(FATAL_ERROR "Force to support stdc++20 but not supported by the compiler")
        else()
            check_cxx_compiler_flag(-std=c++17 SUPPORTS_CXX17)
            if(SUPPORTS_CXX17 AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "17"))
                add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++17>)
                set(HAVE_CXX17 1)
                set(HAVE_CXX14 1)
                set(HAVE_CXX1Y 1)
            elseif(NOT SUPPORTS_CXX17 AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "17")
                message(FATAL_ERROR "Force to support stdc++17 but not supported by the compiler")
            else()
                check_cxx_compiler_flag(-std=c++14 SUPPORTS_CXX14)
                if(SUPPORTS_CXX14 AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "14"))
                    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++14>)
                    set(HAVE_CXX14 1)
                    set(HAVE_CXX1Y 1)
                elseif(NOT SUPPORTS_CXX14 AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "14")
                    message(FATAL_ERROR "Force to support stdc++14 but not supported by the compiler")
                else()
                    check_cxx_compiler_flag(-std=c++1y SUPPORTS_CXX1Y)
                    if(SUPPORTS_CXX1Y AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "1Y"))
                        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++1y>)
                        set(HAVE_CXX1Y 1)
                    elseif(NOT SUPPORTS_CXX1Y AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "1Y")
                        message(FATAL_ERROR "Force to support stdc++1y but not supported by the compiler")
                    else()
                        check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
                        if(SUPPORTS_CXX11 AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "11"))
                            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++11>)
                        elseif(NOT SUPPORTS_CXX11 AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "11")
                            message(FATAL_ERROR "Force to support stdc++11 but not supported by the compiler")
                        else()
                            check_cxx_compiler_flag(-std=c++0x SUPPORTS_CXX0X)
                            if(SUPPORTS_CXX0X AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "0X"))
                                add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++0x>)
                            elseif(NOT SUPPORTS_CXX0X AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "0X")
                                message(FATAL_ERROR "Force to support stdc++0x but not supported by the compiler")
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
        if(SUPPORTS_CXX20 AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "20"))
            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/std:c++20>)
            set(HAVE_CXX20 1)
            set(HAVE_CXX17 1)
            set(HAVE_CXX14 1)
        elseif(NOT SUPPORTS_CXX20 AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "20")
            message(FATAL_ERROR "Force to support stdc++20 but not supported by the compiler")
        else()
            check_cxx_compiler_flag(/std:c++17 SUPPORTS_CXX17)
            if(SUPPORTS_CXX17 AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "17"))
                add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/std:c++17>)
                set(HAVE_CXX17 1)
                set(HAVE_CXX14 1)
            elseif(NOT SUPPORTS_CXX17 AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "17")
                message(FATAL_ERROR "Force to support stdc++17 but not supported by the compiler")
            else()
                check_cxx_compiler_flag(/std:c++14 SUPPORTS_CXX14)
                if(SUPPORTS_CXX14 AND (NOT FORCE_CXX OR "${FORCE_CXX}" STREQUAL "14"))
                    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/std:c++14>)
                    set(HAVE_CXX14 1)
                elseif(NOT SUPPORTS_CXX14 AND FORCE_CXX AND "${FORCE_CXX}" STREQUAL "14")
                    message(FATAL_ERROR "Force to support stdc++14 but not supported by the compiler")
                else()
                    message(STATUS "The specified C++${FORCE_CXX} option is not supported using compiler default.")
                endif() # C++14
            endif() # C++17
        endif() # C++20
    endif()
endmacro()

macro(populate_available_version_variables)

    # Populate the global HAVE_CXX variables
    if(cxx_std_23 IN_LIST CMAKE_CXX_KNOWN_FEATURES
       OR cxx_std_20 IN_LIST CMAKE_CXX_KNOWN_FEATURES)
        set(HAVE_CXX1Y 1 PARENT_SCOPE)
        set(HAVE_CXX14 1 PARENT_SCOPE)
        set(HAVE_CXX17 1 PARENT_SCOPE)
        set(HAVE_CXX20 1 PARENT_SCOPE)
    elseif(cxx_std_17 IN_LIST CMAKE_CXX_KNOWN_FEATURES)
        set(HAVE_CXX1Y 1 PARENT_SCOPE)
        set(HAVE_CXX14 1 PARENT_SCOPE)
        set(HAVE_CXX17 1 PARENT_SCOPE)
        set(HAVE_CXX20 0 PARENT_SCOPE)
    elseif(cxx_std_14 IN_LIST CMAKE_CXX_KNOWN_FEATURES)
        set(HAVE_CXX1Y 1 PARENT_SCOPE)
        set(HAVE_CXX14 1 PARENT_SCOPE)
        set(HAVE_CXX17 0 PARENT_SCOPE)
        set(HAVE_CXX20 0 PARENT_SCOPE)
    elseif(cxx_std_11 IN_LIST CMAKE_CXX_KNOWN_FEATURES)
        set(HAVE_CXX1Y 0 PARENT_SCOPE)
        set(HAVE_CXX14 0 PARENT_SCOPE)
        set(HAVE_CXX17 0 PARENT_SCOPE)
        set(HAVE_CXX20 0 PARENT_SCOPE)
    else()
        set(HAVE_CXX1Y 0 PARENT_SCOPE)
        set(HAVE_CXX14 0 PARENT_SCOPE)
        set(HAVE_CXX17 0 PARENT_SCOPE)
        set(HAVE_CXX20 0 PARENT_SCOPE)
    endif()
endmacro()

function(check_stdcxx)

    # Map force values to cmake features
    set(FORCE_CXX_KEYS 23 20 17 14 11 98)
    set(FORCE_CXX_VALUES cxx_std_23 cxx_std_20 cxx_std_17 cxx_std_14 cxx_std_11 cxx_std_98)

    # Get matching CMake feature
    list(FIND FORCE_CXX_KEYS ${FORCE_CXX} EP_CXX_STANDARD_INDEX_CHOSEN)
    if(NOT EP_CXX_STANDARD_KEY_CHOSEN EQUAL -1)
        list(GET FORCE_CXX_VALUES ${EP_CXX_STANDARD_INDEX_CHOSEN} EP_CXX_STANDARD_VALUE_CHOSEN)
        # Check CMake supports this feature
        get_property(EP_CXX_KNOWN_FEATURES GLOBAL PROPERTY CMAKE_CXX_KNOWN_FEATURES)
        if(EP_CXX_STANDARD_VALUE_CHOSEN IN_LIST EP_CXX_KNOWN_FEATURES)
            # Check the feature availability in the compiler
            if(EP_CXX_STANDARD_VALUE_CHOSEN IN_LIST CMAKE_CXX_COMPILE_FEATURES)
                # Enforce the selected standard for all the projects
                set(CMAKE_CXX_STANDARD ${FORCE_CXX})
            else()
                message(FATAL_ERROR "Force to support ${EP_CXX_STANDARD_VALUE_CHOSEN} but not supported by the compiler")
            endif()
            # Populate version variables
            populate_available_version_variables()
        else()
            # fallback to old behaviour
            check_stdcxx_old()
        endif()
    else()
        message(FATAL_ERROR "Invalid FORCE_CXX value. Choose among: ${FORCE_CXX_KEYS}")
    endif()

endfunction()
