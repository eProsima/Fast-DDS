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

namespace dds {
namespace core {

Exception::Exception()
{
}

Exception::~Exception() throw()
{
}

const char* Exception::what() const throw()
{
   throw "Not implemented";
}


Error::Error(
        const std::string& msg)
    : std::logic_error (msg)
{
    throw "Not implemented";
}

Error::Error(
        const Error& src)
    : std::logic_error (src)
{
    throw "Not implemented";
}

Error::~Error() throw()
{
}

const char* Error::what() const throw()
{
    throw "Not implemented";
}


AlreadyClosedError::AlreadyClosedError(
        const std::string& msg)
    : std::logic_error (msg)
{
    throw "Not implemented";
}

AlreadyClosedError::AlreadyClosedError(
        const AlreadyClosedError& src)
    : std::logic_error (src)
{
    throw "Not implemented";
}

AlreadyClosedError::~AlreadyClosedError() throw()
{
}


const char* AlreadyClosedError::what() const throw()
{
    throw "Not implemented";
}


IllegalOperationError::IllegalOperationError(
        const std::string& msg)
    : std::logic_error (msg)
{
    throw "Not implemented";
}

IllegalOperationError::IllegalOperationError(
        const IllegalOperationError& src)
    : std::logic_error (src)
{
    throw "Not implemented";
}

IllegalOperationError::~IllegalOperationError() throw()
{
}

const char* IllegalOperationError::what() const throw()
{
    throw "Not implemented";
}

ImmutablePolicyError::ImmutablePolicyError(
        const std::string& msg)
    : std::logic_error (msg)
{
    throw "Not implemented";
}

ImmutablePolicyError::ImmutablePolicyError(
        const ImmutablePolicyError& src)
    : std::logic_error (src)
{
    throw "Not implemented";
}

ImmutablePolicyError::~ImmutablePolicyError() throw()
{
}

const char* ImmutablePolicyError::what() const throw()
{
    throw "Not implemented";
}

InconsistentPolicyError::InconsistentPolicyError(
        const std::string& msg)
    : std::logic_error (msg)
{
    throw "Not implemented";
}

InconsistentPolicyError::InconsistentPolicyError(
        const InconsistentPolicyError& src)
    : std::logic_error (src)
{
    throw "Not implemented";
}

InconsistentPolicyError::~InconsistentPolicyError() throw()
{
}

const char* InconsistentPolicyError::what() const throw()
{
    throw "Not implemented";
}

InvalidArgumentError::InvalidArgumentError(
        const std::string& msg)
    : std::invalid_argument (msg)
{
    throw "Not implemented";
}

InvalidArgumentError::InvalidArgumentError(
        const InvalidArgumentError& src)
    : std::invalid_argument (src)
{
    throw "Not implemented";
}

InvalidArgumentError::~InvalidArgumentError() throw()
{
}

const char* InvalidArgumentError::what() const throw()
{
    throw "Not implemented";
}

NotEnabledError::NotEnabledError(
        const std::string& msg)
    : std::logic_error (msg)
{
    throw "Not implemented";
}

NotEnabledError::NotEnabledError(
        const NotEnabledError& src)
    : std::logic_error (src)
{
    throw "Not implemented";
}

NotEnabledError::~NotEnabledError() throw()
{
}

const char* NotEnabledError::what() const throw()
{
    throw "Not implemented";
}

OutOfResourcesError::OutOfResourcesError(
        const std::string& msg)
    : std::runtime_error (msg)
{
    throw "Not implemented";
}

OutOfResourcesError::OutOfResourcesError(
        const OutOfResourcesError& src)
    : std::runtime_error (src)
{
    throw "Not implemented";
}

OutOfResourcesError::~OutOfResourcesError() throw()
{
}

const char* OutOfResourcesError::what() const throw()
{
   throw "Not implemented";
}

PreconditionNotMetError::PreconditionNotMetError(
        const std::string& msg)
    : std::logic_error (msg)
{
    throw "Not implemented";
}

PreconditionNotMetError::PreconditionNotMetError(
        const PreconditionNotMetError& src)
    : std::logic_error (src)
{
    throw "Not implemented";
}

PreconditionNotMetError::~PreconditionNotMetError() throw()
{
}

const char* PreconditionNotMetError::what() const throw()
{
    throw "Not implemented";
}

TimeoutError::TimeoutError(
        const std::string& msg)
    : std::runtime_error (msg)
{
    throw "Not implemented";
}

TimeoutError::TimeoutError(
        const TimeoutError& src)
    : std::runtime_error (src)
{
    throw "Not implemented";
}

TimeoutError::~TimeoutError() throw()
{
}

const char* TimeoutError::what() const throw()
{
    throw "Not implemented";
}

UnsupportedError::UnsupportedError(
        const std::string& msg)
    : std::logic_error (msg)
{
    throw "Not implemented";
}

UnsupportedError::UnsupportedError(
        const UnsupportedError& src)
    : std::logic_error (src)
{
    throw "Not implemented";
}

UnsupportedError::~UnsupportedError() throw()
{
}

const char* UnsupportedError::what() const throw()
{
    throw "Not implemented";
}

InvalidDowncastError::InvalidDowncastError(
        const std::string& msg)
    : std::runtime_error (msg)
{
    throw "Not implemented";
}

InvalidDowncastError::InvalidDowncastError(
        const InvalidDowncastError& src)
    : std::runtime_error (src)
{
    throw "Not implemented";
}

InvalidDowncastError::~InvalidDowncastError() throw()
{
}

const char* InvalidDowncastError::what() const throw()
{
    throw "Not implemented";
}


NullReferenceError::NullReferenceError(
        const std::string& msg)
    : std::runtime_error (msg)
{
    throw "Not implemented";
}

NullReferenceError::NullReferenceError(
        const NullReferenceError& src)
    : std::runtime_error (src)
{
    throw "Not implemented";
}

NullReferenceError::~NullReferenceError() throw()
{
}

const char* NullReferenceError::what() const throw()
{
    throw "Not implemented";
}

InvalidDataError::InvalidDataError(
        const std::string& msg)
    : std::logic_error (msg)
{
    throw "Not implemented";
}

InvalidDataError::InvalidDataError(
        const InvalidDataError& src)
    : std::logic_error (src)
{
    throw "Not implemented";
}

InvalidDataError::~InvalidDataError() throw()
{
}

const char* InvalidDataError::what() const throw()
{
    throw "Not implemented";
}


} //namespace core
} //namespace dds
