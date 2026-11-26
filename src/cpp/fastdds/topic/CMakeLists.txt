# Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#{{{ Unit grouping the DDS-SQL filter expression source code

fastdds_add_unit(dds::sql-filter
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterCompoundCondition.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterCompoundCondition.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterExpression.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterExpression.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterExpressionParser.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterExpressionParser.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterFactory.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterFactory.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterField.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterField.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterParameter.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterParameter.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterPredicate.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterPredicate.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterValue.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/DDSSQLFilter/DDSFilterValue.hpp
    DEPENDS
    dds::participant-factory
    log
    time
    xtypes
    INCLUDE_DIRECTORIES
    ${PROJECT_SOURCE_DIR}/thirdparty/taocpp-pegtl
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    )

#}}}

#{{{ Unit with the TypeSupport, which registers a type on a DomainParticipant.
# A unit test not linking core replaces it with the TypeSupport mock.

fastdds_add_unit(dds::type-support
    SOURCES
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/topic/TypeSupport.hpp

    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/topic/TypeSupport.cpp
    DEPENDS
    dds::entities
    time
    LINK_LIBRARIES
    PUBLIC
    fastcdr
    )

#}}}
