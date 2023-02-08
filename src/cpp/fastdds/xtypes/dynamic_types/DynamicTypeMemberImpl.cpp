// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <cassert>
#include <iomanip>
#include <tuple>

#include <fastdds/dds/log/Log.hpp>
#include "AnnotationDescriptorImpl.hpp"
#include "DynamicTypeBuilderFactoryImpl.hpp"
#include "DynamicTypeMemberImpl.hpp"
#include "MemberDescriptorImpl.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

bool DynamicTypeMemberImpl::operator ==(
        const DynamicTypeMemberImpl& other) const
{
    return get_descriptor() == other.get_descriptor() && AnnotationManager::operator ==(other);
}

std::string DynamicTypeMemberImpl::get_default_value() const
{
    // Fallback to annotation
    std::string res = MemberDescriptorImpl::get_default_value();
    return res.empty() ? annotation_get_default() : res;
}

bool DynamicTypeMemberImpl::is_consistent(
        TypeKind parentKind) const
{
    if (!MemberDescriptorImpl::is_consistent(parentKind))
    {
        return false;
    }

    // checks based on annotations

    // Structures and unions allow it for @external. This condition can only
    // be check in the DynamicTypeMemberImpl override
    if ((parentKind == TK_STRUCTURE || parentKind == TK_UNION) &&
            !type_ && !annotation_is_external())
    {
        return false;
    }

    // Bitset non-anonymous elements must have position and bound
    if (parentKind == TK_BITSET && !name_.empty() &&
            (!annotation_is_bit_bound() || !annotation_is_position()))
    {
        return false;
    }

    return true;
}

std::ostream& operator <<(
        std::ostream& os,
        const DynamicTypeMemberImpl& dm)
{
    using namespace std;

    // delegate into the base class
    os << static_cast<const MemberDescriptorImpl&>(dm);

    // Show the annotations if any
    if (dm.get_annotation_count())
    {
        // indentation increment
        ++os.iword(DynamicTypeBuilderFactoryImpl::indentation_index);

        auto manips = [](ostream& os) -> ostream&
                {
                    long indent = os.iword(DynamicTypeBuilderFactoryImpl::indentation_index);
                    return os << string(indent, '\t') << setw(10) << left;
                };

        os << manips << "member annotations:" << endl;
        for (const AnnotationDescriptorImpl& d : dm.get_all_annotations())
        {
            os << d;
        }

        // indentation decrement
        --os.iword(DynamicTypeBuilderFactoryImpl::indentation_index);
    }

    return os;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
