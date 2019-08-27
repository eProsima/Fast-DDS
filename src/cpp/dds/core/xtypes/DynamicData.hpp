/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_HPP_
#define OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_HPP_

#include <dds/core/xtypes/detail/DynamicData.hpp>

#include <dds/core/xtypes/DynamicType.hpp>
#include <dds/core/xtypes/MemberType.hpp>

#include <dds/core/Reference.hpp>

namespace dds {
namespace core {
namespace xtypes {

/**
 * This class is used to read/write data for DynamicTypes. It allows reading and
 * writing of samples in a type-safe manner but without any compile-time knowledge
 * of the type being read or written.
 */
template<
        typename DELEGATE,
        typename DELEGATE_TYPE>
class TDynamicData : Reference<DELEGATE>
{
public:
    OMG_DDS_REF_TYPE_PROTECTED_DC(
            TDynamicData,
            Reference,
            DELEGATE)

    TDynamicData(
            const TDynamicType<DELEGATE_TYPE>& type);

    template<typename T>
    void value(
            uint32_t mid,
            const T& v) const;

    template<typename T>
    T value(
            const std::string& mid,
            const T& v) const;

    template<typename T>
    T value(
            uint32_t mid) const;

    template<typename T>
    T value(
            const std::string& mid) const;

    TDynamicType<DELEGATE_TYPE> type() const;

    MemberType member_type(
            uint32_t id) const;

    MemberType member_type(
            const std::string& name) const;

    uint32_t member_id(
            const std::string& name) const;
};

typedef TDynamicData<detail::DynamicData, detail::DynamicType> DynamicData;

} //namespace xtypes
} //namespace core
} //namespace dds

#endif //OMG_DDS_CORE_XTYPES_DYNAMIC_DATA_HPP_
