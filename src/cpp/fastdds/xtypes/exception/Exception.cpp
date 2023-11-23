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

#include <fastdds/dds/xtypes/exception/Exception.hpp>

#include <stdexcept>
#include <string>

namespace eprosima {
namespace fastdds {
namespace dds {
namespace xtypes {

InvalidArgumentError::InvalidArgumentError(
        const std::string& msg)
    : Exception()
    , std::invalid_argument(msg)
{
}

InvalidArgumentError::InvalidArgumentError(
        const InvalidArgumentError& src)
    : Exception()
    , std::invalid_argument(src.what())
{
}

InvalidArgumentError::~InvalidArgumentError() throw()
{
}

const char* InvalidArgumentError::what() const throw()
{
    return this->std::invalid_argument::what();
}

} // xtypes
} // dds
} // fastdds
} // eprosima
