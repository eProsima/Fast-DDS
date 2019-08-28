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
    return nullptr;
}


Error::Error(
        const std::string& msg)
    : std::logic_error (msg)
{
    (void) msg;
}

Error::Error(
        const Error& src)
    : std::logic_error (src)
{
    (void) src;
}

Error::~Error() throw()
{
}

const char* Error::what() const throw()
{
    return nullptr;
}


AlreadyClosedError::AlreadyClosedError(
        const std::string& msg)
    : std::logic_error (msg)
{
    (void) msg;
}

AlreadyClosedError::AlreadyClosedError(
        const AlreadyClosedError& src)
    : std::logic_error (src)
{
    (void) src;
}

AlreadyClosedError::~AlreadyClosedError() throw()
{
}


const char* AlreadyClosedError::what() const throw()
{
    return nullptr;
}


IllegalOperationError::IllegalOperationError(
        const std::string& msg)
    : std::logic_error (msg)
{
    (void) msg;
}

IllegalOperationError::IllegalOperationError(
        const IllegalOperationError& src)
    : std::logic_error (src)
{
    (void) src;
}

IllegalOperationError::~IllegalOperationError() throw()
{
}

const char* IllegalOperationError::what() const throw()
{
    return nullptr;
}

ImmutablePolicyError::ImmutablePolicyError(
        const std::string& msg)
    : std::logic_error (msg)
{
    (void) msg;
}

ImmutablePolicyError::ImmutablePolicyError(
        const ImmutablePolicyError& src)
    : std::logic_error (src)
{
    (void) src;
}

ImmutablePolicyError::~ImmutablePolicyError() throw()
{
}

const char* ImmutablePolicyError::what() const throw()
{
    return nullptr;
}

InconsistentPolicyError::InconsistentPolicyError(
        const std::string& msg)
    : std::logic_error (msg)
{
    (void) msg;
}

InconsistentPolicyError::InconsistentPolicyError(
        const InconsistentPolicyError& src)
    : std::logic_error (src)
{
    (void) src;
}

InconsistentPolicyError::~InconsistentPolicyError() throw()
{
}

const char* InconsistentPolicyError::what() const throw()
{
    return nullptr;
}

InvalidArgumentError::InvalidArgumentError(
        const std::string& msg)
    : std::invalid_argument (msg)
{
    (void) msg;
}

InvalidArgumentError::InvalidArgumentError(
        const InvalidArgumentError& src)
    : std::invalid_argument (src)
{
    (void) src;
}

InvalidArgumentError::~InvalidArgumentError() throw()
{
}

const char* InvalidArgumentError::what() const throw()
{
    return nullptr;
}

NotEnabledError::NotEnabledError(
        const std::string& msg)
    : std::logic_error (msg)
{
    (void) msg;
}

NotEnabledError::NotEnabledError(
        const NotEnabledError& src)
    : std::logic_error (src)
{
    (void) src;
}

NotEnabledError::~NotEnabledError() throw()
{
}

const char* NotEnabledError::what() const throw()
{
    return nullptr;
}

OutOfResourcesError::OutOfResourcesError(
        const std::string& msg)
    : std::runtime_error (msg)
{
    (void) msg;
}

OutOfResourcesError::OutOfResourcesError(
        const OutOfResourcesError& src)
    : std::runtime_error (src)
{
    (void) src;
}

OutOfResourcesError::~OutOfResourcesError() throw()
{
}

const char* OutOfResourcesError::what() const throw()
{
    return nullptr;
}

PreconditionNotMetError::PreconditionNotMetError(
        const std::string& msg)
    : std::logic_error (msg)
{
    (void) msg;
}

PreconditionNotMetError::PreconditionNotMetError(
        const PreconditionNotMetError& src)
    : std::logic_error (src)
{
    (void) src;
}

PreconditionNotMetError::~PreconditionNotMetError() throw()
{
}

const char* PreconditionNotMetError::what() const throw()
{
    return nullptr;
}

TimeoutError::TimeoutError(
        const std::string& msg)
    : std::runtime_error (msg)
{
    (void) msg;
}

TimeoutError::TimeoutError(
        const TimeoutError& src)
    : std::runtime_error (src)
{
    (void) src;
}

TimeoutError::~TimeoutError() throw()
{
}

const char* TimeoutError::what() const throw()
{
    return nullptr;
}

UnsupportedError::UnsupportedError(
        const std::string& msg)
    : std::logic_error (msg)
{
    (void) msg;
}

UnsupportedError::UnsupportedError(
        const UnsupportedError& src)
    : std::logic_error (src)
{
    (void) src;
}

UnsupportedError::~UnsupportedError() throw()
{
}

const char* UnsupportedError::what() const throw()
{
    return nullptr;
}

InvalidDowncastError::InvalidDowncastError(
        const std::string& msg)
    : std::runtime_error (msg)
{
    (void) msg;
}

InvalidDowncastError::InvalidDowncastError(
        const InvalidDowncastError& src)
    : std::runtime_error (src)
{
    (void) src;
}

InvalidDowncastError::~InvalidDowncastError() throw()
{
}

const char* InvalidDowncastError::what() const throw()
{
    return nullptr;
}


NullReferenceError::NullReferenceError(
        const std::string& msg)
    : std::runtime_error (msg)
{
    (void) msg;
}

NullReferenceError::NullReferenceError(
        const NullReferenceError& src)
    : std::runtime_error (src)
{
    (void) src;
}

NullReferenceError::~NullReferenceError() throw()
{
}

const char* NullReferenceError::what() const throw()
{
    return nullptr;
}

InvalidDataError::InvalidDataError(
        const std::string& msg)
    : std::logic_error (msg)
{
    (void) msg;
}

InvalidDataError::InvalidDataError(
        const InvalidDataError& src)
    : std::logic_error (src)
{
    (void) src;
}

InvalidDataError::~InvalidDataError() throw()
{
}

const char* InvalidDataError::what() const throw()
{
    return nullptr;
}


} //namespace core
} //namespace dds