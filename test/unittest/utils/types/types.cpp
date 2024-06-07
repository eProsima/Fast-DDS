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
 * @file types.cpp
 *
 */

#include <memory>

#include <fastdds/dds/xtypes/utils.hpp>

#include "types.hpp"

using namespace eprosima::fastdds::dds;

std::unique_ptr<IDataType> data_type_factory(
        const DataTypeKind data_kind)
{
    if (data_kind == DataTypeKind::HELLO_WORLD)
    {
        return std::unique_ptr<IDataType>(new DataType<DataTypeKind::HELLO_WORLD>());
    }

        if (data_kind == DataTypeKind::DATA_TEST)
    {
        return std::unique_ptr<IDataType>(new DataType<DataTypeKind::DATA_TEST>());
    }

    throw std::invalid_argument("Invalid data type kind");
}

