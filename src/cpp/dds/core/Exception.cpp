/*
 * Copyright 2019, Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <dds/core/Exception.hpp>

#define DEFINE_LOGIC_ERROR_EXCEPTION(ERROR) \
    ERROR::ERROR( \
    const std::string& msg)  \
        : Exception() \
        , std::logic_error(msg){} \
    ERROR::ERROR( \
    const ERROR& src)  \
        : Exception() \
        , std::logic_error(src.what()){} \
    ERROR::~ERROR() throw(){} \
    const char* ERROR::what() const throw() { return this->std::logic_error::what();}

#define DEFINE_RUNTIME_ERROR_EXCEPTION(ERROR) \
    ERROR::ERROR( \
    const std::string& msg)  \
        : Exception() \
        , std::runtime_error(msg){} \
    ERROR::ERROR( \
    const ERROR& src)  \
        : Exception() \
        , std::runtime_error(src.what()){} \
    ERROR::~ERROR() throw(){} \
    const char* ERROR::what() const throw() { return this->std::runtime_error::what();}

#define DEFINE_INVALID_ARGUMENT_EXCEPTION(ERROR) \
    ERROR::ERROR( \
    const std::string& msg)  \
        : Exception() \
        , std::invalid_argument(msg){} \
    ERROR::ERROR( \
    const ERROR& src)  \
        : Exception() \
        , std::invalid_argument(src.what()){} \
    ERROR::~ERROR() throw(){} \
    const char* ERROR::what() const throw() { return this->std::invalid_argument::what();}

namespace dds {
namespace core {

Exception::Exception()
{
}

Exception::~Exception()
{
}

DEFINE_LOGIC_ERROR_EXCEPTION(Error)
DEFINE_LOGIC_ERROR_EXCEPTION(InvalidDataError)
DEFINE_LOGIC_ERROR_EXCEPTION(PreconditionNotMetError)
DEFINE_LOGIC_ERROR_EXCEPTION(UnsupportedError)
DEFINE_LOGIC_ERROR_EXCEPTION(NotEnabledError)
DEFINE_LOGIC_ERROR_EXCEPTION(InconsistentPolicyError)
DEFINE_LOGIC_ERROR_EXCEPTION(ImmutablePolicyError)
DEFINE_LOGIC_ERROR_EXCEPTION(AlreadyClosedError)
DEFINE_LOGIC_ERROR_EXCEPTION(IllegalOperationError)

DEFINE_RUNTIME_ERROR_EXCEPTION(OutOfResourcesError)
DEFINE_RUNTIME_ERROR_EXCEPTION(TimeoutError)
DEFINE_RUNTIME_ERROR_EXCEPTION(InvalidDowncastError)
DEFINE_RUNTIME_ERROR_EXCEPTION(NullReferenceError)

DEFINE_INVALID_ARGUMENT_EXCEPTION(InvalidArgumentError)

} // namespace core
} // namespace dds
