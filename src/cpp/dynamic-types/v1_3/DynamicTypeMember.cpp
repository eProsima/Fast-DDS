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
#include <fastrtps/types/v1_3/AnnotationDescriptor.h>
#include <fastrtps/types/v1_3/DynamicType.h>
#include <fastrtps/types/v1_3/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/v1_3/DynamicTypeMember.h>
#include <fastrtps/types/v1_3/MemberDescriptor.h>

#include <cassert>
#include <iomanip>
#include <tuple>

using namespace eprosima::fastrtps::types::v1_3;

bool DynamicTypeMember::operator ==(
        const DynamicTypeMember& other) const
{
    return get_descriptor() == other.get_descriptor() && AnnotationManager::operator ==(other);
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

bool DynamicTypeMember::is_consistent(
        TypeKind parentKind) const
{
    if (!MemberDescriptor::is_consistent(parentKind))
    {
        return false;
    }

    // checks based on annotations

    // Structures and unions allow it for @external. This condition can only
    // be check in the DynamicTypeMember override
    if ((parentKind == TypeKind::TK_STRUCTURE || parentKind == TypeKind::TK_UNION) &&
            !type_ && !annotation_is_external())
    {
        return false;
    }

    // Bitset non-anonymous elements must have position and bound
    if (parentKind == TypeKind::TK_BITSET && !name_.empty() &&
            (!annotation_is_bit_bound() || !annotation_is_position()))
    {
        return false;
    }

    return true;
}

std::ostream& eprosima::fastrtps::types::v1_3::operator <<(
        std::ostream& os,
        const DynamicTypeMember& dm)
{
    using namespace std;

    // delegate into the base class
    os << static_cast<const MemberDescriptor&>(dm);

    // Show the annotations if any
    if (dm.get_annotation_count())
    {
        // indentation increment
        ++os.iword(DynamicTypeBuilderFactory::indentation_index);

        auto manips = [](ostream& os) -> ostream&
                {
                    long indent = os.iword(DynamicTypeBuilderFactory::indentation_index);
                    return os << string(indent, '\t') << setw(10) << left;
                };

        os << manips << "member annotations:" << endl;
        for (const AnnotationDescriptor& d : dm.get_all_annotations())
        {
            os << d;
        }

        // indentation decrement
        --os.iword(DynamicTypeBuilderFactory::indentation_index);
    }

    return os;
}
