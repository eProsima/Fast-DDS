
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

#ifndef DYNAMIC_TYPES_TYPEOBJECTHASHIDCDRAUX_IPP
#define DYNAMIC_TYPES_TYPEOBJECTHASHIDCDRAUX_IPP

namespace eprosima {
namespace fastcdr {
template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::TypeObjectHashId& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    static_cast<void>(calculator);
    size_t initial_alignment = current_alignment;

    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    switch (data._d())
    {
        case eprosima::fastrtps::types::EK_COMPLETE:
        case eprosima::fastrtps::types::EK_MINIMAL:
            current_alignment += ((14) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1); break;
        default:
            break;
    }

    return current_alignment - initial_alignment;

#else

    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data._d(), current_alignment);

    switch (data._d())
    {
        case eprosima::fastrtps::types::EK_COMPLETE:
        case eprosima::fastrtps::types::EK_MINIMAL:
            calculated_size += ((14) * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1); break;
        default:
            break;
    }

    calculated_size += calculator.end_calculate_type_serialized_size(
        eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment);

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::TypeObjectHashId& data)
{
    scdr << data._d();

    switch (data._d())
    {
        case eprosima::fastrtps::types::EK_COMPLETE:
        case eprosima::fastrtps::types::EK_MINIMAL:
            for (int i = 0; i < 14; ++i)
            {
                scdr << data.hash()[i];
            }
            break;
        default:
            break;
    }
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::TypeObjectHashId& data)
{
    dcdr >> data._d();

    switch (data._d())
    {
        case eprosima::fastrtps::types::EK_COMPLETE:
        case eprosima::fastrtps::types::EK_MINIMAL:
            for (int i = 0; i < 14; ++i)
            {
                dcdr >> data.hash()[i];
            }
            break;
        default:
            break;
    }
}

} // namespace fastcdr
} // namespace eprosima

#endif // DYNAMIC_TYPES_TYPEOBJECTHASHIDCDRAUX_IPP
