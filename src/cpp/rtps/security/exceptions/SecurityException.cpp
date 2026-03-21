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

#include "SecurityException.h"

using namespace eprosima::fastdds::rtps::security;

SecurityException::SecurityException(
        const SecurityException& ex)
    : Exception(ex)
{
}

SecurityException::SecurityException(
        SecurityException&& ex)
    : Exception(std::move(ex))
{
}

SecurityException& SecurityException::operator =(
        const SecurityException& ex)
{
    if (this != &ex)
    {
        Exception::operator =(
                ex);
    }

    return *this;
}

SecurityException& SecurityException::operator =(
        SecurityException&& ex)
{
    if (this != &ex)
    {
        Exception::operator =(
                std::move(ex));
    }

    return *this;
}

SecurityException::~SecurityException() throw()
{
}

void SecurityException::raise() const
{
    throw *this;
}
