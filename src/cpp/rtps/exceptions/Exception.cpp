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

#include "Exception.h"

namespace eprosima {
namespace fastdds {
namespace rtps {

Exception::Exception(
        const char* const& message)
    : message_(message)
    , minor_(0)
{
}

Exception::Exception(
        const Exception& ex)
    : message_(ex.message_)
    , minor_(ex.minor_)
{
}

Exception::Exception(
        Exception&& ex)
    : message_(std::move(ex.message_))
    , minor_(ex.minor_)
{
}

Exception::Exception(
        const char* const& message,
        const int32_t minor)
    : message_(message)
    , minor_(minor)
{
}

Exception& Exception::operator =(
        const Exception& ex)
{
    message_ = ex.message_;
    minor_ = ex.minor_;
    return *this;
}

Exception& Exception::operator =(
        Exception&& ex)
{
    message_ = std::move(ex.message_);
    minor_ = ex.minor_;
    return *this;
}

Exception::~Exception() throw()
{
}

const int32_t& Exception::minor() const
{
    return minor_;
}

void Exception::minor(
        const int32_t& minor)
{
    minor_ = minor;
}

const char* Exception::what() const throw()
{
    return message_.c_str();
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
