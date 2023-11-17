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

#ifndef DYNAMIC_TYPES_TYPEIDENTIFIERCDRAUX_IPP
#define DYNAMIC_TYPES_TYPEIDENTIFIERCDRAUX_IPP

namespace eprosima {
namespace fastcdr {
template<>
size_t calculate_serialized_size(
        eprosima::fastcdr::CdrSizeCalculator& calculator,
        const eprosima::fastrtps::types::TypeIdentifier& data,
        size_t& current_alignment)
{
#if FASTCDR_VERSION_MAJOR == 1

    size_t initial_alignment = current_alignment;

    current_alignment += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    switch (data._d())
    {
        case eprosima::fastrtps::types::TI_STRING8_SMALL:
        case eprosima::fastrtps::types::TI_STRING16_SMALL:
            calculate_serialized_size(calculator, data.string_sdefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_STRING8_LARGE:
        case eprosima::fastrtps::types::TI_STRING16_LARGE:
            calculate_serialized_size(calculator, data.string_ldefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_PLAIN_SEQUENCE_SMALL:
            calculate_serialized_size(calculator, data.seq_sdefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_PLAIN_SEQUENCE_LARGE:
            calculate_serialized_size(calculator, data.seq_ldefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_PLAIN_ARRAY_SMALL:
            calculate_serialized_size(calculator, data.array_sdefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_PLAIN_ARRAY_LARGE:
            calculate_serialized_size(calculator, data.array_ldefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_PLAIN_MAP_SMALL:
            calculate_serialized_size(calculator, data.map_sdefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_PLAIN_MAP_LARGE:
            calculate_serialized_size(calculator, data.map_ldefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_STRONGLY_CONNECTED_COMPONENT:
            calculate_serialized_size(calculator, data.sc_component_id(), current_alignment);

            break;
        case eprosima::fastrtps::types::EK_COMPLETE:
        case eprosima::fastrtps::types::EK_MINIMAL:
            current_alignment += 14 + eprosima::fastcdr::Cdr::alignment(current_alignment, 14);

            break;
        default:
            calculate_serialized_size(calculator, data.extended_defn(), current_alignment);
            break;
    }

    return current_alignment - initial_alignment;

#else

    size_t calculated_size {calculator.begin_calculate_type_serialized_size(
                                eprosima::fastcdr::EncodingAlgorithmFlag::PLAIN_CDR2, current_alignment)};

    calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                        0), data._d(), current_alignment);

    calculated_size += 1 + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);

    switch (data._d())
    {
        case eprosima::fastrtps::types::TI_STRING8_SMALL:
        case eprosima::fastrtps::types::TI_STRING16_SMALL:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                1), data.string_sdefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_STRING8_LARGE:
        case eprosima::fastrtps::types::TI_STRING16_LARGE:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                2), data.string_ldefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_PLAIN_SEQUENCE_SMALL:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                3), data.seq_sdefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_PLAIN_SEQUENCE_LARGE:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                4), data.seq_ldefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_PLAIN_ARRAY_SMALL:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                5), data.array_sdefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_PLAIN_ARRAY_LARGE:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                6), data.array_ldefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_PLAIN_MAP_SMALL:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                7), data.map_sdefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_PLAIN_MAP_LARGE:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                8), data.map_ldefn(), current_alignment);

            break;
        case eprosima::fastrtps::types::TI_STRONGLY_CONNECTED_COMPONENT:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                9), data.sc_component_id(), current_alignment);

            break;
        case eprosima::fastrtps::types::EK_COMPLETE:
        case eprosima::fastrtps::types::EK_MINIMAL:
            calculated_size += 14 + eprosima::fastcdr::Cdr::alignment(current_alignment, 14);

            break;
        default:
            calculated_size += calculator.calculate_member_serialized_size(eprosima::fastcdr::MemberId(
                                11), data.extended_defn(), current_alignment);
            break;
    }

    return calculated_size;

#endif // FASTCDR_VERSION_MAJOR == 1
}

template<>
void serialize(
        eprosima::fastcdr::Cdr& scdr,
        const eprosima::fastrtps::types::TypeIdentifier& data)
{
    scdr << data._d();

    switch (data._d())
    {
        case eprosima::fastrtps::types::TK_NONE:
            break;
        case eprosima::fastrtps::types::TI_STRING8_SMALL:
        case eprosima::fastrtps::types::TI_STRING16_SMALL:
            scdr << data.string_sdefn();
            break;
        case eprosima::fastrtps::types::TI_STRING8_LARGE:
        case eprosima::fastrtps::types::TI_STRING16_LARGE:
            scdr << data.string_ldefn();
            break;
        case eprosima::fastrtps::types::TI_PLAIN_SEQUENCE_SMALL:
            scdr << data.seq_sdefn();
            break;
        case eprosima::fastrtps::types::TI_PLAIN_SEQUENCE_LARGE:
            scdr << data.seq_ldefn();
            break;
        case eprosima::fastrtps::types::TI_PLAIN_ARRAY_SMALL:
            scdr << data.array_sdefn();
            break;
        case eprosima::fastrtps::types::TI_PLAIN_ARRAY_LARGE:
            scdr << data.array_ldefn();
            break;
        case eprosima::fastrtps::types::TI_PLAIN_MAP_SMALL:
            scdr << data.map_sdefn();
            break;
        case eprosima::fastrtps::types::TI_PLAIN_MAP_LARGE:
            scdr << data.map_ldefn();
            break;
        case eprosima::fastrtps::types::TI_STRONGLY_CONNECTED_COMPONENT:
            scdr << data.sc_component_id();
            break;
        case eprosima::fastrtps::types::EK_COMPLETE:
        case eprosima::fastrtps::types::EK_MINIMAL:
            for (int i = 0; i < 14; ++i)
            {
                scdr << data.equivalence_hash()[i];
            }
            break;
        default:
            scdr << data.extended_defn();
            break;
    }
}

template<>
void deserialize(
        eprosima::fastcdr::Cdr& dcdr,
        eprosima::fastrtps::types::TypeIdentifier& data)
{
    dcdr >> data._d();

    switch (data._d())
    {
        case eprosima::fastrtps::types::TK_NONE:
            break;
        case eprosima::fastrtps::types::TI_STRING8_SMALL:
        case eprosima::fastrtps::types::TI_STRING16_SMALL:
            dcdr >> data.string_sdefn();
            break;
        case eprosima::fastrtps::types::TI_STRING8_LARGE:
        case eprosima::fastrtps::types::TI_STRING16_LARGE:
            dcdr >> data.string_ldefn();
            break;
        case eprosima::fastrtps::types::TI_PLAIN_SEQUENCE_SMALL:
            dcdr >> data.seq_sdefn();
            break;
        case eprosima::fastrtps::types::TI_PLAIN_SEQUENCE_LARGE:
            dcdr >> data.seq_ldefn();
            break;
        case eprosima::fastrtps::types::TI_PLAIN_ARRAY_SMALL:
            dcdr >> data.array_sdefn();
            break;
        case eprosima::fastrtps::types::TI_PLAIN_ARRAY_LARGE:
            dcdr >> data.array_ldefn();
            break;
        case eprosima::fastrtps::types::TI_PLAIN_MAP_SMALL:
            dcdr >> data.map_sdefn();
            break;
        case eprosima::fastrtps::types::TI_PLAIN_MAP_LARGE:
            dcdr >> data.map_ldefn();
            break;
        case eprosima::fastrtps::types::TI_STRONGLY_CONNECTED_COMPONENT:
            dcdr >> data.sc_component_id();
            break;
        case eprosima::fastrtps::types::EK_COMPLETE:
        case eprosima::fastrtps::types::EK_MINIMAL:
            for (int i = 0; i < 14; ++i)
            {
                dcdr >> data.equivalence_hash()[i];
            }
            break;
        default:
            dcdr >> data.extended_defn();
            break;
    }
}

} // namespace fastcdr
} // namespace eprosima

#endif // DYNAMIC_TYPES_TYPEIDENTIFIERCDRAUX_IPP
