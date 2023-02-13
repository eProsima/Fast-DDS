// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fastdds/dds/log/Log.hpp>
#include <fastrtps/types/AnnotationDescriptor.h>
#include <fastrtps/types/DynamicType.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeMember.h>
#include <fastrtps/types/MemberDescriptor.h>

#include <cassert>
#include <tuple>

using namespace eprosima::fastrtps::types;

bool DynamicTypeMember::operator==(const DynamicTypeMember& other) const
{
    return get_descriptor() == other.get_descriptor() && annotation_ == other.annotation_;
}

bool DynamicTypeMember::equals(
        const DynamicTypeMember& other) const
{
    return *this == other;
}

ReturnCode_t DynamicTypeMember::get_descriptor(
        MemberDescriptor& descriptor) const
{
    descriptor = get_descriptor();
    return ReturnCode_t::RETCODE_OK;
}

std::string DynamicTypeMember::get_default_value() const
{
    // Fallback to annotation
    std::string res = MemberDescriptor::get_default_value();
    return res.empty() ? annotation_get_default() : res;
}

