// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_BITBOUNDANNOTATION_HPP
#define FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_BITBOUNDANNOTATION_HPP

#include <exception>
#include <map>
#include <string>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/MemberDescriptor.hpp>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>

#include "../../TypeValueConverter.hpp"
#include "../IdlParserTags.hpp"
#include "BuiltinAnnotation.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {
namespace idlparser {

/**
 * @brief @bit_bound annotation
 */
class BitBoundAnnotation final : public BuiltinAnnotation
{
public:

    BitBoundAnnotation()
        : BuiltinAnnotation(IDL_BUILTIN_ANN_BIT_BOUND_TAG)
        , initialized_(false)
    {
    }

    ~BitBoundAnnotation() = default;

    bool initialize() override
    {
        if (!initialized_)
        {
            initialized_ = add_primitive_or_string_member(
                                IDL_VALUE_TAG,
                                DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_UINT16));
        }

        return initialized_;
    }

protected:

    bool apply_to_type(
            TypeDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Missing required parameter '" << IDL_VALUE_TAG
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_BIT_BOUND_TAG <<
                            "'.");
            return false;
        }

        if ((TK_BITSET != descriptor->kind()) && (TK_BITMASK != descriptor->kind()))
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "TypeDescriptor can only be annotated with '" << IDL_BUILTIN_ANN_BIT_BOUND_TAG
                                                                  << "' for bitset/bitmask types.");
            return false;
        }

        try
        {
            TypeForKind<TK_UINT16> value = TypeValueConverter::sto(parameters.at(IDL_VALUE_TAG));
            descriptor->bound({value});
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Failed to convert value '" << parameters.at(
                        IDL_VALUE_TAG)
                                                << "' for annotation '" << IDL_BUILTIN_ANN_BIT_BOUND_TAG << "': " <<
                            e.what());
            return false;
        }

        return true;
    }

    bool apply_to_member(
            MemberDescriptor::_ref_type& descriptor,
            const std::map<std::string, std::string>& parameters) const override
    {
        assert(descriptor != nullptr);

        if (parameters.find(IDL_VALUE_TAG) == parameters.end())
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Missing required parameter '" << IDL_VALUE_TAG
                                                   << "' for annotation '" << IDL_BUILTIN_ANN_BIT_BOUND_TAG <<
                            "'.");
            return false;
        }

        TypeForKind<TK_UINT16> value;

        try
        {
            value = TypeValueConverter::sto(parameters.at(IDL_VALUE_TAG));
        }
        catch (const std::exception& e)
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Failed to convert value '" << parameters.at(
                        IDL_VALUE_TAG)
                                                << "' for annotation '" << IDL_BUILTIN_ANN_BIT_BOUND_TAG << "': " <<
                            e.what());
            return false;
        }

        DynamicType::_ref_type member_type;

        if (value == 8)
        {
            member_type = DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT8);
        }
        else if (value == 16)
        {
            member_type = DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT16);
        }
        else if (value == 32)
        {
            member_type = DynamicTypeBuilderFactory::get_instance()->get_primitive_type(TK_INT32);
        }
        else
        {
            EPROSIMA_LOG_ERROR(IDL_PARSER,
                    "Invalid bit bound value '" << value << "' for annotation '"
                                                << IDL_BUILTIN_ANN_BIT_BOUND_TAG << "'.");
            return false;
        }

        descriptor->type(member_type);

        return true;
    }

    bool initialized_;
};

} // namespace idlparser
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_XTYPES_DYNAMIC_TYPES_IDL_PARSER_ANNOTATIONS_BITBOUNDANNOTATION_HPP