// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <gtest/gtest.h>

#include <fastdds/dds/rpc/exceptions.hpp>
#include <fastdds/dds/rpc/interfaces.hpp>
#include <fastdds/dds/rpc/RemoteExceptionCode_t.hpp>

namespace rpc = eprosima::fastdds::dds::rpc;

TEST(RPC, ExceptionCodes)
{
    rpc::RemoteInvalidArgumentError invalid_argument_error;
    EXPECT_EQ(invalid_argument_error.code(), rpc::RemoteExceptionCode_t::REMOTE_EX_INVALID_ARGUMENT);

    rpc::RemoteOutOfResourcesError out_of_resources_error;
    EXPECT_EQ(out_of_resources_error.code(), rpc::RemoteExceptionCode_t::REMOTE_EX_OUT_OF_RESOURCES);

    rpc::RemoteUnknownExceptionError unknown_exception_error;
    EXPECT_EQ(unknown_exception_error.code(), rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_EXCEPTION);

    rpc::RemoteUnknownOperationError unknown_operation_error;
    EXPECT_EQ(unknown_operation_error.code(), rpc::RemoteExceptionCode_t::REMOTE_EX_UNKNOWN_OPERATION);

    rpc::RemoteUnsupportedError unsupported_error;
    EXPECT_EQ(unsupported_error.code(), rpc::RemoteExceptionCode_t::REMOTE_EX_UNSUPPORTED);
}

TEST(RPC, Exceptions)
{
    rpc::RpcBrokenPipeException broken_pipe_exception(true);
    EXPECT_NE(broken_pipe_exception.what(), nullptr);

    rpc::RpcBrokenPipeException broken_pipe_exception_2(false);
    EXPECT_NE(broken_pipe_exception_2.what(), nullptr);

    rpc::RpcException rpc_exception("Generic exception");
    EXPECT_NE(rpc_exception.what(), nullptr);

    rpc::RpcException rpc_exception_2(std::string("Generic exception"));
    EXPECT_NE(rpc_exception_2.what(), nullptr);

    rpc::RpcFeedCancelledException feed_cancelled_exception(rpc::RPC_STATUS_CODE_OK);
    EXPECT_NE(feed_cancelled_exception.what(), nullptr);

    rpc::RpcOperationError operation_error("Operation error");
    EXPECT_NE(operation_error.what(), nullptr);

    rpc::RpcOperationError operation_error_2(std::string("Operation error"));
    EXPECT_NE(operation_error_2.what(), nullptr);

    rpc::RpcTimeoutException timeout_exception;
    EXPECT_NE(timeout_exception.what(), nullptr);

    rpc::RpcRemoteException remote_exception(rpc::RemoteExceptionCode_t::REMOTE_EX_OK, "Remote exception");
    EXPECT_NE(remote_exception.what(), nullptr);

    rpc::RemoteInvalidArgumentError invalid_argument_error("Invalid argument");
    EXPECT_NE(invalid_argument_error.what(), nullptr);

    rpc::RemoteOutOfResourcesError out_of_resources_error("Not enough memory");
    EXPECT_NE(out_of_resources_error.what(), nullptr);

    rpc::RemoteUnknownExceptionError unknown_exception_error("std::exception");
    EXPECT_NE(unknown_exception_error.what(), nullptr);

    rpc::RemoteUnknownOperationError unknown_operation_error("Operation hash 0123456789abcdef");
    EXPECT_NE(unknown_operation_error.what(), nullptr);

    rpc::RemoteUnsupportedError unsupported_error("Still not implemented");
    EXPECT_NE(unsupported_error.what(), nullptr);
}

TEST(RPC, ThrowExceptions)
{
    EXPECT_THROW(throw rpc::RpcException("Generic exception"), rpc::RpcException);
    EXPECT_THROW(throw rpc::RpcException(std::string("Generic exception")), rpc::RpcException);

    EXPECT_THROW(throw rpc::RpcBrokenPipeException(true), rpc::RpcBrokenPipeException);
    EXPECT_THROW(throw rpc::RpcBrokenPipeException(false), rpc::RpcBrokenPipeException);
    EXPECT_THROW(throw rpc::RpcBrokenPipeException(false), rpc::RpcException);

    EXPECT_THROW(throw rpc::RpcFeedCancelledException(rpc::RPC_STATUS_CODE_OK), rpc::RpcFeedCancelledException);
    EXPECT_THROW(throw rpc::RpcFeedCancelledException(rpc::RPC_STATUS_CODE_OK), rpc::RpcException);

    EXPECT_THROW(throw rpc::RpcOperationError("Operation error"), rpc::RpcOperationError);
    EXPECT_THROW(throw rpc::RpcOperationError(std::string("Operation error")), rpc::RpcOperationError);
    EXPECT_THROW(throw rpc::RpcOperationError("Operation error"), rpc::RpcException);

    EXPECT_THROW(throw rpc::RpcTimeoutException(), rpc::RpcTimeoutException);
    EXPECT_THROW(throw rpc::RpcTimeoutException(), rpc::RpcException);

    EXPECT_THROW(throw rpc::RpcRemoteException(rpc::RemoteExceptionCode_t::REMOTE_EX_OK,
            "Remote exception"), rpc::RpcRemoteException);
    EXPECT_THROW(throw rpc::RpcRemoteException(rpc::RemoteExceptionCode_t::REMOTE_EX_OK,
            "Remote exception"), rpc::RpcException);

    EXPECT_THROW(throw rpc::RemoteInvalidArgumentError("Invalid argument"), rpc::RemoteInvalidArgumentError);
    EXPECT_THROW(throw rpc::RemoteInvalidArgumentError("Invalid argument"), rpc::RpcRemoteException);
    EXPECT_THROW(throw rpc::RemoteInvalidArgumentError("Invalid argument"), rpc::RpcException);

    EXPECT_THROW(throw rpc::RemoteOutOfResourcesError("Not enough memory"), rpc::RemoteOutOfResourcesError);
    EXPECT_THROW(throw rpc::RemoteOutOfResourcesError("Not enough memory"), rpc::RpcRemoteException);
    EXPECT_THROW(throw rpc::RemoteOutOfResourcesError("Not enough memory"), rpc::RpcException);

    EXPECT_THROW(throw rpc::RemoteUnknownExceptionError("std::exception"), rpc::RemoteUnknownExceptionError);
    EXPECT_THROW(throw rpc::RemoteUnknownExceptionError("std::exception"), rpc::RpcRemoteException);
    EXPECT_THROW(throw rpc::RemoteUnknownExceptionError("std::exception"), rpc::RpcException);

    EXPECT_THROW(throw rpc::RemoteUnknownOperationError(
                "Operation hash 0123456789abcdef"), rpc::RemoteUnknownOperationError);
    EXPECT_THROW(throw rpc::RemoteUnknownOperationError("Operation hash 0123456789abcdef"), rpc::RpcRemoteException);
    EXPECT_THROW(throw rpc::RemoteUnknownOperationError("Operation hash 0123456789abcdef"), rpc::RpcException);

    EXPECT_THROW(throw rpc::RemoteUnsupportedError("Still not implemented"), rpc::RemoteUnsupportedError);
    EXPECT_THROW(throw rpc::RemoteUnsupportedError("Still not implemented"), rpc::RpcRemoteException);
    EXPECT_THROW(throw rpc::RemoteUnsupportedError("Still not implemented"), rpc::RpcException);
}
