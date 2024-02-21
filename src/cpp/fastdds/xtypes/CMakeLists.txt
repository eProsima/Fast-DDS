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

#{{{ Interface library for grouping the Dynamic Language Binding source code.

#{{{ API

add_library(xtypes-dynamic-types-api INTERFACE)

add_library(fastdds::xtypes::dynamic-types::api ALIAS xtypes-dynamic-types-api)

target_compile_features(xtypes-dynamic-types-api INTERFACE cxx_std_11)

target_sources(xtypes-dynamic-types-api INTERFACE
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/xtypes/dynamic_types/AnnotationDescriptor.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/xtypes/dynamic_types/DynamicData.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/xtypes/dynamic_types/DynamicType.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/xtypes/dynamic_types/type_traits.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/xtypes/dynamic_types/Types.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/xtypes/dynamic_types/VerbatimTextDescriptor.hpp

    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/xtypes/dynamic_types/detail/type_traits.hpp

    dynamic_types/DynamicDataFactory.cpp
    dynamic_types/DynamicTypeBuilderFactory.cpp
    )

#}}}

#{{{ Implementation

add_library(xtypes-dynamic-types-impl INTERFACE)

add_library(fastdds::xtypes::dynamic-types::impl ALIAS xtypes-dynamic-types-impl)

target_compile_features(xtypes-dynamic-types-impl INTERFACE cxx_std_11)

target_sources(xtypes-dynamic-types-impl INTERFACE
    ${PROJECT_SOURCE_DIR}/include/fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp
    dynamic_types/AnnotationDescriptorImpl.cpp
    dynamic_types/AnnotationDescriptorImpl.hpp
    dynamic_types/DynamicDataImpl.cpp
    dynamic_types/DynamicDataImpl.hpp
    dynamic_types/DynamicDataFactoryImpl.cpp
    dynamic_types/DynamicDataFactoryImpl.hpp
    dynamic_types/DynamicPubSubType.cpp
    dynamic_types/DynamicTypeImpl.cpp
    dynamic_types/DynamicTypeImpl.hpp
    dynamic_types/DynamicTypeBuilderImpl.cpp
    dynamic_types/DynamicTypeBuilderImpl.hpp
    dynamic_types/DynamicTypeBuilderFactoryImpl.cpp
    dynamic_types/DynamicTypeBuilderFactoryImpl.hpp
    dynamic_types/DynamicTypeMemberImpl.cpp
    dynamic_types/DynamicTypeMemberImpl.hpp
    dynamic_types/MemberDescriptorImpl.cpp
    dynamic_types/MemberDescriptorImpl.hpp
    dynamic_types/TypeDescriptorImpl.cpp
    dynamic_types/TypeDescriptorImpl.hpp
    dynamic_types/VerbatimTextDescriptorImpl.cpp
    dynamic_types/VerbatimTextDescriptorImpl.hpp
    )

target_link_libraries(xtypes-dynamic-types-impl INTERFACE xtypes-dynamic-types-api)

#}}}

#}}}
