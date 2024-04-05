// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file identifiers.hpp
 *
 * Note: this is an implementation file, designed to be included inside the
 * DDSFilterExpressionParser.hpp file of the parent folder.
 */

struct CurrentIdentifierState
{
    const std::shared_ptr<xtypes::TypeObject> type_object;
    std::shared_ptr<xtypes::TypeIdentifier> current_type;
    std::vector<DDSFilterField::FieldAccessor> access_path;
};

struct identifier_processor
    : parse_tree::apply< identifier_processor >
{
    template<typename _TSize>
    static constexpr size_t process_bound(
            _TSize bound)
    {
        return 0 == bound ? std::numeric_limits<size_t>::max() : static_cast<size_t>(bound);
    }

    template<typename _BSeq>
    static size_t process_bounds(
            const _BSeq& bound_seq)
    {
        if (1 != bound_seq.size())
        {
            return 0;
        }

        return process_bound(bound_seq[0]);
    }

    static bool type_should_be_indexed(
            const xtypes::TypeIdentifier& ti,
            std::shared_ptr<xtypes::TypeIdentifier>& out_type,
            size_t& max_size)
    {
        max_size = 0;

        switch (ti._d())
        {
            case xtypes::TI_PLAIN_ARRAY_SMALL:
                out_type = std::make_shared<xtypes::TypeIdentifier>(*ti.array_sdefn().element_identifier());
                max_size = process_bounds(ti.array_sdefn().array_bound_seq());
                return true;

            case xtypes::TI_PLAIN_ARRAY_LARGE:
                out_type = std::make_shared<xtypes::TypeIdentifier>(*ti.array_ldefn().element_identifier());
                max_size = process_bounds(ti.array_ldefn().array_bound_seq());
                return true;

            case xtypes::TI_PLAIN_SEQUENCE_SMALL:
                out_type = std::make_shared<xtypes::TypeIdentifier>(*ti.seq_sdefn().element_identifier());
                max_size = process_bound(ti.seq_sdefn().bound());
                return true;

            case xtypes::TI_PLAIN_SEQUENCE_LARGE:
                out_type = std::make_shared<xtypes::TypeIdentifier>(*ti.seq_ldefn().element_identifier());
                max_size = process_bound(ti.seq_ldefn().bound());
                return true;

            default:
                out_type = std::make_shared<xtypes::TypeIdentifier>(ti);
        }

        return false;
    }

    static void add_member_access(
            std::unique_ptr< ParseNode >& n,
            CurrentIdentifierState& identifier_state,
            const xtypes::CompleteTypeObject& complete)
    {
        if (xtypes::TK_STRUCTURE != complete._d())
        {
            throw parse_error("trying to access field on a non-struct type", n->begin());
        }

        const ParseNode& name_node = n->left();
        std::string name = name_node.content();
        size_t member_index;
        const xtypes::CompleteStructMemberSeq& members = complete.struct_type().member_seq();
        for (member_index = 0; member_index < members.size(); ++member_index)
        {
            if (members[member_index].detail().name() == name)
            {
                break;
            }
        }

        if (member_index == members.size())
        {
            throw parse_error("field not found", name_node.begin());
        }

        const xtypes::TypeIdentifier& ti = members[member_index].common().member_type_id();
        bool has_index = n->children.size() > 1;
        size_t max_size = 0;
        size_t array_index = std::numeric_limits<size_t>::max();
        if (type_should_be_indexed(ti, identifier_state.current_type, max_size))
        {
            if (!has_index)
            {
                throw parse_error("field should have an index (i.e. [n])", n->left().end());
            }

            array_index = static_cast<size_t>(std::stoul(n->right().left().content()));
            if (max_size <= array_index)
            {
                throw parse_error("index is greater than maximum size", n->right().end());
            }
        }
        else
        {
            if (has_index)
            {
                throw parse_error("field is not an array or sequence", n->right().begin());
            }
        }

        identifier_state.access_path.emplace_back(DDSFilterField::FieldAccessor{ member_index, array_index });
    }

    static DDSFilterValue::ValueKind get_value_kind(
            const xtypes::TypeIdentifier& ti,
            const position& pos)
    {
        switch (ti._d())
        {
            case xtypes::TK_BOOLEAN:
                return DDSFilterValue::ValueKind::BOOLEAN;

            case xtypes::TK_CHAR8:
                return DDSFilterValue::ValueKind::CHAR;

            case xtypes::TK_STRING8:
            case xtypes::TI_STRING8_SMALL:
            case xtypes::TI_STRING8_LARGE:
                return DDSFilterValue::ValueKind::STRING;

            case xtypes::TK_INT8:
            case xtypes::TK_INT16:
            case xtypes::TK_INT32:
            case xtypes::TK_INT64:
                return DDSFilterValue::ValueKind::SIGNED_INTEGER;

            case xtypes::TK_BYTE:
            case xtypes::TK_UINT8:
            case xtypes::TK_UINT16:
            case xtypes::TK_UINT32:
            case xtypes::TK_UINT64:
                return DDSFilterValue::ValueKind::UNSIGNED_INTEGER;

            case xtypes::TK_FLOAT32:
                return DDSFilterValue::ValueKind::FLOAT_FIELD;

            case xtypes::TK_FLOAT64:
                return DDSFilterValue::ValueKind::DOUBLE_FIELD;

            case xtypes::TK_FLOAT128:
                return DDSFilterValue::ValueKind::LONG_DOUBLE_FIELD;

            case xtypes::EK_COMPLETE:
            {
                std::shared_ptr<xtypes::TypeObject> type_object = std::make_shared<xtypes::TypeObject>();
                if (RETCODE_OK == DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                            ti, *type_object) && xtypes::EK_COMPLETE == type_object->_d())
                {
                    if (xtypes::TK_ENUM == type_object->complete()._d())
                    {
                        return DDSFilterValue::ValueKind::ENUM;
                    }
                    if (xtypes::TK_ALIAS == type_object->complete()._d())
                    {
                        const xtypes::TypeIdentifier& aliasedId =
                                type_object->complete().alias_type().body().common().related_type();
                        return get_value_kind(aliasedId, pos);
                    }
                }
            }
            break;

        }
        throw parse_error("type is not primitive", pos);
    }

    template< typename ... States >
    static void transform(
            std::unique_ptr< ParseNode >& n,
            CurrentIdentifierState& state,
            States&&... /*st*/)
    {
        if (n->is<fieldname>())
        {
            // Set data for fieldname node
            n->field_kind = get_value_kind(*state.current_type, n->end());
            n->field_access_path = state.access_path;
            n->type_id = state.current_type;

            // Reset parser state
            state.access_path.clear();
            state.current_type.reset();
        }
        else
        {
            if (!state.current_type)
            {
                add_member_access(n, state, state.type_object->complete());
            }
            else
            {
                if (xtypes::EK_COMPLETE != state.current_type->_d())
                {
                    throw parse_error("trying to access field on a non-complete type", n->begin());
                }

                std::shared_ptr<xtypes::TypeObject> type_object = std::make_shared<xtypes::TypeObject>();
                ReturnCode_t ret = DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                    *state.current_type, *type_object);
                if (RETCODE_BAD_PARAMETER == ret)
                {
                    throw parse_error("could not find type object definition", n->begin());
                }

                add_member_access(n, state, type_object->complete());
            }
        }

        n->children.clear();
    }

};
