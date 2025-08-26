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

#include "ReqRepHelloWorldReplier.hpp"
#include "ReqRepHelloWorldRequester.hpp"

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

/**
 * RPC enhanced discovery algotithm.
 *
 * This test checks that the requester correctly behaves when
 * the replier is still unmatched and the request is sent.
 */
TEST(RPC, replier_unmatched_before_sending_request)
{
    ReqRepHelloWorldRequester requester;
    ReqRepHelloWorldReplier replier;

    // Initialize the requester and replier
    requester.init();
    ASSERT_TRUE(requester.isInitialized());

    // Write a request, expecting it to fail
    requester.send(0, [](
                eprosima::fastdds::dds::rpc::Requester* requester,
                eprosima::fastdds::dds::rpc::RequestInfo* info,
                void* request)
            {
                ASSERT_EQ(requester->send_request(request,
                *info), eprosima::fastdds::dds::RETCODE_PRECONDITION_NOT_MET);
            });

    auto future_send = std::async(std::launch::async, [&requester]()
                    {
                        // Write a request, this time matching after 300ms
                        requester.send(0);
                    });

    // At the same time, initialize the replier
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    replier.init();
    ASSERT_TRUE(replier.isInitialized());

    // The requester should now be able to receive the reply
    requester.block(std::chrono::seconds(5));
}

/**
 * RPC enhanced discovery algotithm.
 *
 * Requester is unmatched during request processing in the server side.
 * This test checks that after waiting for the timeout, the send_reply()
 * fails.
 */
TEST(RPC, requester_unmatched_during_request_processing)
{
    std::shared_ptr<ReqRepHelloWorldRequester> requester = std::make_shared<ReqRepHelloWorldRequester>();

    std::condition_variable replier_finished_cv;
    std::atomic<bool> finished{false};
    std::mutex replier_finished_mutex;
    eprosima::fastdds::dds::Duration_t reply_elapsed;

    // Simulate a Replier with heavy processing
    ReqRepHelloWorldReplier replier
        ([&replier_finished_cv, &finished, &reply_elapsed](eprosima::fastdds::dds::rpc::RequestInfo& info,
            eprosima::fastdds::dds::rpc::Replier* replier,
            const void* const request)
            {
                // Simulate heavy processing
                std::this_thread::sleep_for(std::chrono::seconds(2));
                const HelloWorld* hello_request = static_cast<const HelloWorld*>(request);
                ASSERT_EQ(hello_request->message().compare("HelloWorld"), 0);
                HelloWorld reply;

                Duration_t t0, t1;
                Duration_t::now(t0);

                // send_reply() should fail because the requester will be unmatched
                ASSERT_EQ(replier->send_reply((void*)&reply, info), eprosima::fastdds::dds::RETCODE_NO_DATA);
                finished.store(true);
                Duration_t::now(t1);
                reply_elapsed = t1 - t0;
                replier_finished_cv.notify_one();
            });

    // Initialize the requester and replier
    requester->init();
    ASSERT_TRUE(requester->isInitialized());
    replier.init();
    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery
    requester->wait_discovery();
    replier.wait_discovery();

    // Write a request
    requester->send(0);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    requester.reset();

    // Wait for the replier to finish processing
    std::unique_lock<std::mutex> lock(replier_finished_mutex);
    replier_finished_cv.wait(lock, [&finished]()
            {
                return finished.load();
            });
    ASSERT_TRUE(finished.load());
    // Check that the reply took at least the wait_matching timeout (3 secs)
    ASSERT_GT(reply_elapsed, Duration_t{2});
}

/**
 * Test RPC communication with multiple requesters and one replier.
 *
 * This test checks that multiple requesters can send requests to a single replier
 * and receive replies correctly.
 */
TEST(RPC, multiple_requesters_one_replier)
{
    ReqRepHelloWorldRequester requester_1;
    ReqRepHelloWorldRequester requester_2;
    ReqRepHelloWorldReplier replier;

    // Initialize the requesters and the replier
    requester_1.init();
    ASSERT_TRUE(requester_1.isInitialized());
    requester_2.init();
    ASSERT_TRUE(requester_2.isInitialized());
    replier.init();
    ASSERT_TRUE(replier.isInitialized());

    // Wait for discovery
    requester_1.wait_discovery();
    requester_2.wait_discovery();
    replier.wait_discovery(2, 2);

    // Send requests from both requesters
    requester_1.send(1);
    requester_2.send(2);

    // Block to wait for replies
    requester_1.block(std::chrono::seconds(5));
    requester_2.block(std::chrono::seconds(5));
}
