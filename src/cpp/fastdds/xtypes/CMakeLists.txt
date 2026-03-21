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

add_library(fastdds-xtypes-dynamic-types-api INTERFACE)

add_library(fastdds::xtypes::dynamic-types::api ALIAS fastdds-xtypes-dynamic-types-api)

target_compile_features(fastdds-xtypes-dynamic-types-api INTERFACE cxx_std_11)

target_sources(fastdds-xtypes-dynamic-types-api INTERFACE
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/dynamic_types/AnnotationDescriptor.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/dynamic_types/DynamicData.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/dynamic_types/DynamicType.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/dynamic_types/type_traits.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/dynamic_types/Types.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/dynamic_types/VerbatimTextDescriptor.hpp

    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/dynamic_types/detail/type_traits.hpp

    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/utils.hpp

    dynamic_types/DynamicDataFactory.cpp
    dynamic_types/DynamicTypeBuilderFactory.cpp
    )

#}}}

#{{{ Implementation

add_library(fastdds-xtypes-dynamic-types-impl INTERFACE)

add_library(fastdds::xtypes::dynamic-types::impl ALIAS fastdds-xtypes-dynamic-types-impl)

target_compile_features(fastdds-xtypes-dynamic-types-impl INTERFACE cxx_std_11)

target_sources(fastdds-xtypes-dynamic-types-impl INTERFACE
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/dynamic_types/DynamicPubSubType.hpp
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
    serializers/idl/dynamic_type_idl.cpp
    serializers/idl/dynamic_type_idl.hpp
    serializers/json/dynamic_data_json.cpp
    serializers/json/dynamic_data_json.hpp
    serializers/json/json_dynamic_data.cpp
    serializers/json/json_dynamic_data.hpp
    utils.cpp
    )

target_link_libraries(fastdds-xtypes-dynamic-types-impl INTERFACE fastdds-xtypes-dynamic-types-api)

#}}}

#}}}

#{{{ Interface library for grouping the TypeObject Representation source code.

add_library(fastdds-xtypes-type-representation INTERFACE)

add_library(fastdds::xtypes::type-representation ALIAS fastdds-xtypes-type-representation)

target_compile_features(fastdds-xtypes-type-representation INTERFACE cxx_std_11)

target_sources(fastdds-xtypes-type-representation INTERFACE
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/exception/Exception.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/type_representation/TypeObject.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp

    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobject.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobjectCdrAux.hpp
    ${FASTDDS_DIR}/include/fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobjectPubSubTypes.hpp

    exception/Exception.cpp
    type_representation/dds_xtypes_typeobjectCdrAux.ipp
    type_representation/dds_xtypes_typeobjectPubSubTypes.cxx
    type_representation/TypeIdentifierWithSizeHashSpecialization.h
    type_representation/TypeObjectRegistry.cpp
    type_representation/TypeObjectRegistry.hpp
    type_representation/TypeObjectUtils.cpp
    )

#}}}
