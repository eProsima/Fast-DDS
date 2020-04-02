/*
 * Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
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
 */

#ifndef OMG_DDS_CORE_EXCEPTION_HPP_
#define OMG_DDS_CORE_EXCEPTION_HPP_

#include <dds/core/macros.hpp>

#include <stdexcept>
#include <string>

#if defined _MSC_VER
#   pragma warning (push)
#   pragma warning (disable:4275) // non dll-interface class 'std::foo_error' used as base for dll-interface class 'dds::core::BarError'
#endif

namespace dds {
namespace core {

/**
 * @brief
 * %Exception: base class for specified DDS Exceptions.
 *
 * DDS PIM Return Code            | DDS-PSM-CXX Exception Class   | std C++ Parent Exception
 * -------------------            | ---------------------------   | ------------------------
 * RETCODE_OK                     | Normal return; no exception   | N/A
 * RETCODE_NO_DATA                | Normal return with informational state attached    | N/A
 * RETCODE_ERROR                  | Error                         | std::logic_error
 * RETCODE_BAD_PARAMETER          | InvalidArgumentError          | std::invalid_argument
 * RETCODE_TIMEOUT                | TimeoutError                  | std::runtime_error
 * RETCODE_UNSUPPORTED            | UnsupportedError              | std::logic_error
 * RETCODE_ALREADY_DELETED        | AlreadyClosedError            | std::logic_error
 * RETCODE_ILLEGAL_OPERATION      | IllegalOperationError         | std::logic_error
 * RETCODE_NOT_ENABLED            | NotEnabledError               | std::logic_error
 * RETCODE_PRECONDITION_NOT_MET   | PreconditionNotMetError       | std::logic_error
 * RETCODE_IMMUTABLE_POLICY       | ImmutablePolicyError          | std::logic_error
 * RETCODE_INCONSISTENT_POLICY    | InconsistentPolicyError       | std::logic_error
 * RETCODE_OUT_OF_RESOURCES       | OutOfResourcesError           | std::runtime_error
 *
 * The DDS-PSM-Cxx maps error codes to C++ exceptions defined in the dds::core namespace and
 * inheriting from a base Exception class and the appropriate standard C++ exception.
 * Table 7.3 lists the mapping between error codes as defined in the DDS PIM and C++ exceptions
 * as used in this specification. Exceptions have value semantics; this means that they must
 * always have deep copy semantics.
 * The full list of exceptions is included in the file dds/core/Exceptions.hpp.
 *
 */
class Exception
{
protected:

    OMG_DDS_API Exception()
    {
    }

public:

    /** @cond */
    OMG_DDS_API virtual ~Exception() throw()
    {
    }

    /** @endcond */

public:

    /**
     * Retrieve information about the exception that was thrown.
     *
     * Example
     * @code{.cpp}
     * try {
     *     // Do something that will trigger a dds exception, like:
     *     dds::domain::DomainParticipant participant = dds::core::null;
     *     participant.domain_id();
     * } catch (const dds::core::Exception& e) {
     *     std::cout << e.what() << std::endl;
     * }
     * @endcode
     * %Exception information (of the NullReferenceError in this case)
     * @code
     * Null reference: Reference[157] == dds::core::null
     * ========================================================================================
     * Context     : dds::domain::DomainParticipant::domain_id
     * Date        : Wed Oct 21 19:28:00 CET 2015
     * Node        : DeLorean
     * Process     : flux_capacitor <15423>
     * Thread      : mr_fusion b6f25700
     * Internals   : ReferenceImpl.hpp/157/V6.6.0
     * ----------------------------------------------------------------------------------------
     * Report      : Null reference: Reference[157] == dds::core::null
     * Internals   : dds::core::Reference<DELEGATE>::delegate/ReferenceImpl.hpp/157
     * @endcode
     *
     * @return Exception information
     */
    OMG_DDS_API virtual const char* what() const throw() = 0;
};

/**
 * @brief
 * %Exception: Generic, unspecified error.
 */
class Error : public Exception, public std::logic_error
{
    /** @cond */

public:

    OMG_DDS_API explicit Error(
            const std::string& msg);

    OMG_DDS_API Error(
            const Error& src);

    OMG_DDS_API virtual ~Error() throw();

public:

    OMG_DDS_API virtual const char* what() const throw();
    /** @endcond */
};

/**
 * @brief
 * %Exception: The object target of this operation has already been closed.
 */
class AlreadyClosedError : public Exception, public std::logic_error
{
    /** @cond */

public:

    OMG_DDS_API explicit AlreadyClosedError(
            const std::string& msg);

    OMG_DDS_API AlreadyClosedError(
            const AlreadyClosedError& src);

    OMG_DDS_API virtual ~AlreadyClosedError() throw();

public:

    OMG_DDS_API virtual const char* what() const throw();
    /** @endcond */
};

/**
 * @brief
 * %Exception: An operation was invoked on an inappropriate object or at an inappropriate time.
 *
 * This is determined by policies set by the specification or the Service implementation.
 *
 * There is no precondition that could be changed to make the operation succeed.
 */
class IllegalOperationError : public Exception, public std::logic_error
{
    /** @cond */

public:

    OMG_DDS_API explicit IllegalOperationError(
            const std::string& msg);

    OMG_DDS_API IllegalOperationError(
            const IllegalOperationError& src);

    OMG_DDS_API virtual ~IllegalOperationError() throw();

public:

    OMG_DDS_API virtual const char* what() const throw();
    /** @endcond */
};

/**
 * @brief
 * %Exception: Application attempted to modify an immutable QosPolicy.
 */
class ImmutablePolicyError : public Exception, public std::logic_error
{
    /** @cond */

public:

    OMG_DDS_API explicit ImmutablePolicyError(
            const std::string& msg);

    OMG_DDS_API ImmutablePolicyError(
            const ImmutablePolicyError& src);

    OMG_DDS_API virtual ~ImmutablePolicyError() throw();

public:

    OMG_DDS_API virtual const char* what() const throw();
    /** @endcond */
};

/**
 * @brief
 * %Exception: Application specified a set of policies that are not
 * consistent with each other.
 */
class InconsistentPolicyError : public Exception, public std::logic_error
{
    /** @cond */

public:

    OMG_DDS_API explicit InconsistentPolicyError(
            const std::string& msg);

    OMG_DDS_API InconsistentPolicyError(
            const InconsistentPolicyError& src);

    OMG_DDS_API virtual ~InconsistentPolicyError() throw();

public:

    OMG_DDS_API virtual const char* what() const throw();
    /** @endcond */
};

/**
 * @brief
 * %Exception: Application is passing an invalid argument.
 */
class InvalidArgumentError : public Exception, public std::invalid_argument
{
    /** @cond */

public:

    OMG_DDS_API explicit InvalidArgumentError(
            const std::string& msg);

    OMG_DDS_API InvalidArgumentError(
            const InvalidArgumentError& src);

    OMG_DDS_API virtual ~InvalidArgumentError() throw();

public:

    OMG_DDS_API virtual const char* what() const throw();
    /** @endcond */
};

/**
 * @brief
 * %Exception: Operation invoked on an Entity that is not yet enabled.
 */
class NotEnabledError : public Exception, public std::logic_error
{
    /** @cond */

public:

    OMG_DDS_API explicit NotEnabledError(
            const std::string& msg);

    OMG_DDS_API NotEnabledError(
            const NotEnabledError& src);

    OMG_DDS_API virtual ~NotEnabledError() throw();

public:

    OMG_DDS_API virtual const char* what() const throw();
    /** @endcond */
};

/**
 * @brief
 * %Exception: Service ran out of the resources needed to complete the
 * operation.
 */
class OutOfResourcesError : public Exception, public std::runtime_error
{
    /** @cond */

public:

    OMG_DDS_API explicit OutOfResourcesError(
            const std::string& msg);

    OMG_DDS_API OutOfResourcesError(
            const OutOfResourcesError& src);

    OMG_DDS_API virtual ~OutOfResourcesError() throw();

public:

    OMG_DDS_API virtual const char* what() const throw();
    /** @endcond */
};


/**
 * @brief
 * %Exception: A pre-condition for the operation was not met.
 */
class PreconditionNotMetError : public Exception, public std::logic_error
{
    /** @cond */

public:

    OMG_DDS_API explicit PreconditionNotMetError(
            const std::string& msg);

    OMG_DDS_API PreconditionNotMetError(
            const PreconditionNotMetError& src);

    OMG_DDS_API virtual ~PreconditionNotMetError() throw();

public:

    OMG_DDS_API virtual const char* what() const throw();
    /** @endcond */
};

/**
 * @brief
 * %Exception: The operation timed out.
 */
class TimeoutError : public Exception, public std::runtime_error
{
    /** @cond */

public:

    OMG_DDS_API explicit TimeoutError(
            const std::string& msg);

    OMG_DDS_API TimeoutError(
            const TimeoutError& src);

    OMG_DDS_API virtual ~TimeoutError() throw();

public:

    OMG_DDS_API virtual const char* what() const throw();
    /** @endcond */
};

/**
 * @brief
 * %Exception: Unsupported operation.
 *
 * This can only be thrown by operations that are optional.
 */
class UnsupportedError : public Exception, public std::logic_error
{
    /** @cond */

public:

    OMG_DDS_API explicit UnsupportedError(
            const std::string& msg);

    OMG_DDS_API UnsupportedError(
            const UnsupportedError& src);

    OMG_DDS_API virtual ~UnsupportedError() throw();

public:

    OMG_DDS_API virtual const char* what() const throw();
    /** @endcond */
};

/**
 * @brief
 * %Exception: Application has attempted to cast incompatible types.
 */
class InvalidDowncastError : public Exception, public std::runtime_error
{
    /** @cond */

public:

    OMG_DDS_API explicit InvalidDowncastError(
            const std::string& msg);

    OMG_DDS_API InvalidDowncastError(
            const InvalidDowncastError& src);

    OMG_DDS_API virtual ~InvalidDowncastError() throw();

public:

    OMG_DDS_API virtual const char* what() const throw();
    /** @endcond */
};

/**
 * @brief
 * %Exception: Application used a null reference.
 *
 * Very likely, the used DDS object is a dds::core::null object.
 * @code{.cpp}
 * dds::domain::DomainParticipant participant = dds::core::null;
 * try {
 *     participant.domain_id();
 * } catch (const dds::core::NullReferenceError& e) {
 *     std::cout << e.what() << std::endl;
 * }
 * @endcode
 */
class NullReferenceError : public Exception, public std::runtime_error
{
    /** @cond */

public:

    OMG_DDS_API explicit NullReferenceError(
            const std::string& msg);

    OMG_DDS_API NullReferenceError(
            const NullReferenceError& src);

    OMG_DDS_API virtual ~NullReferenceError() throw();

public:

    OMG_DDS_API virtual const char* what() const throw();
    /** @endcond */
};

/**
 * @brief
 * %Exception: Application provided invalid data
 */
class InvalidDataError : public Exception, public std::logic_error
{
    /** @cond */

public:

    OMG_DDS_API explicit InvalidDataError(
            const std::string& msg);

    OMG_DDS_API InvalidDataError(
            const InvalidDataError& src);

    OMG_DDS_API virtual ~InvalidDataError() throw();

public:

    OMG_DDS_API virtual const char* what() const throw();
    /** @endcond */
};

} //namespace core
} //namespace dds

#if defined _MSC_VER
#   pragma warning (pop)
#endif

#endif //OMG_DDS_CORE_EXCEPTION_HPP_

