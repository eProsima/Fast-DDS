# Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#{{{ Interface library for grouping the Log source code

add_library(fastdds-log INTERFACE)

add_library(fastdds::log ALIAS fastdds-log)

target_compile_features(fastdds-log INTERFACE cxx_std_11)

target_sources(fastdds-log INTERFACE
    ${FASTDDS_DIR}/include/fastdds/dds/log/Colors.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/log/FileConsumer.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/log/Log.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/log/OStreamConsumer.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/log/StdoutConsumer.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/log/StdoutErrConsumer.hpp

    FileConsumer.cpp
    Log.cpp
    LogResources.hpp
    OStreamConsumer.cpp
    StdoutConsumer.cpp
    StdoutErrConsumer.cpp
    )

# Settings this group's code is sensitive to. See cmake/common/reusable_objects.cmake.
fastdds_declare_object_group(fastdds-log
    MOCKS
        rtps/Log
    DEFINES
        ENABLE_OLD_LOG_MACROS_
        FASTDDS_ENFORCE_LOG_INFO
        HAVE_LOG_NO_ERROR
        HAVE_LOG_NO_INFO
        HAVE_LOG_NO_WARNING
        STDOUTERR_LOG_CONSUMER
    )

#}}}
