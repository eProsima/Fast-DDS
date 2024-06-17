// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file Token.cpp
 */

#include <fastdds/rtps/common/Token.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

std::string* DataHolderHelper::find_property_value(
        DataHolder& data_holder,
        const std::string& name)
{
    std::string* returnedValue = nullptr;

    for (auto property = data_holder.properties().begin(); property != data_holder.properties().end(); ++property)
    {
        if (property->name().compare(name) == 0)
        {
            returnedValue = &property->value();
            break;
        }
    }

    return returnedValue;
}

const std::string* DataHolderHelper::find_property_value(
        const DataHolder& data_holder,
        const std::string& name)
{
    const std::string* returnedValue = nullptr;

    for (auto property = data_holder.properties().begin(); property != data_holder.properties().end(); ++property)
    {
        if (property->name().compare(name) == 0)
        {
            returnedValue = &property->value();
            break;
        }
    }

    return returnedValue;
}

Property* DataHolderHelper::find_property(
        DataHolder& data_holder,
        const std::string& name)
{
    Property* returnedValue = nullptr;

    for (auto property = data_holder.properties().begin(); property != data_holder.properties().end(); ++property)
    {
        if (property->name().compare(name) == 0)
        {
            returnedValue = &(*property);
            break;
        }
    }

    return returnedValue;
}

const Property* DataHolderHelper::find_property(
        const DataHolder& data_holder,
        const std::string& name)
{
    const Property* returnedValue = nullptr;

    for (auto property = data_holder.properties().begin(); property != data_holder.properties().end(); ++property)
    {
        if (property->name().compare(name) == 0)
        {
            returnedValue = &(*property);
            break;
        }
    }

    return returnedValue;
}

std::vector<uint8_t>* DataHolderHelper::find_binary_property_value(
        DataHolder& data_holder,
        const std::string& name)
{
    std::vector<uint8_t>* returnedValue = nullptr;

    for (auto property = data_holder.binary_properties().begin(); property != data_holder.binary_properties().end();
            ++property)
    {
        if (property->name().compare(name) == 0)
        {
            returnedValue = &property->value();
            break;
        }
    }

    return returnedValue;
}

const std::vector<uint8_t>* DataHolderHelper::find_binary_property_value(
        const DataHolder& data_holder,
        const std::string& name)
{
    const std::vector<uint8_t>* returnedValue = nullptr;

    for (auto property = data_holder.binary_properties().begin(); property != data_holder.binary_properties().end();
            ++property)
    {
        if (property->name().compare(name) == 0)
        {
            returnedValue = &property->value();
            break;
        }
    }

    return returnedValue;
}

BinaryProperty* DataHolderHelper::find_binary_property(
        DataHolder& data_holder,
        const std::string& name)
{
    BinaryProperty* returnedValue = nullptr;

    for (auto property = data_holder.binary_properties().begin(); property != data_holder.binary_properties().end();
            ++property)
    {
        if (property->name().compare(name) == 0)
        {
            returnedValue = &(*property);
            break;
        }
    }

    return returnedValue;
}

const BinaryProperty* DataHolderHelper::find_binary_property(
        const DataHolder& data_holder,
        const std::string& name)
{
    const BinaryProperty* returnedValue = nullptr;

    for (auto property = data_holder.binary_properties().begin(); property != data_holder.binary_properties().end();
            ++property)
    {
        if (property->name().compare(name) == 0)
        {
            returnedValue = &(*property);
            break;
        }
    }

    return returnedValue;
}

size_t DataHolderHelper::serialized_size(
        const DataHolder& data_holder,
        size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + alignment(current_alignment, 4) + data_holder.class_id().size() + 1;
    current_alignment += PropertyHelper::serialized_size(data_holder.properties(), current_alignment);
    current_alignment += BinaryPropertyHelper::serialized_size(data_holder.binary_properties(), current_alignment);

    return current_alignment - initial_alignment;
}

size_t DataHolderHelper::serialized_size(
        const DataHolderSeq& data_holders,
        size_t current_alignment)
{
    size_t initial_alignment = current_alignment;

    current_alignment += 4 + alignment(current_alignment, 4);
    for (auto data_holder = data_holders.begin(); data_holder != data_holders.end(); ++data_holder)
    {
        current_alignment += serialized_size(*data_holder, current_alignment);
    }

    return current_alignment - initial_alignment;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
