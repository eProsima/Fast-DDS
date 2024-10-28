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

#include <future>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fastdds/rtps/reader/RTPSReader.hpp>

#include <rtps/builtin/data/WriterProxyData.hpp>
#include <rtps/participant/RTPSParticipantImpl.hpp>
#include <rtps/reader/StatefulReader.hpp>
#include <rtps/reader/WriterProxy.h>
#include <rtps/reader/WriterProxy.cpp>
#include <rtps/resources/ResourceEvent.h>
#include <rtps/resources/TimedEvent.h>

namespace testing {
namespace internal {
using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds;

class WriterProxyTest : public WriterProxy
{
public:

    WriterProxyTest(
            StatefulReader* reader,
            const RemoteLocatorsAllocationAttributes& loc_alloc,
            const ResourceLimitedContainerConfig& changes_allocation,
            eprosima::fastdds::rtps::ResourceEvent& service,
            std::promise<void>& promise)
        : WriterProxy(reader, loc_alloc, changes_allocation)
        , reader_(reader)
    {
        auto acknack_lambda_test = [this, &promise]() -> bool
                {
                    promise.set_value();
                    perform_initial_ack_nack();
                    return false;
                };

        initial_acknack_test_ = new TimedEvent(service, acknack_lambda_test, 0);
    }

    ~WriterProxyTest()
    {
        delete(initial_acknack_test_);
    }

    void start(
            const WriterProxyData& attributes,
            const SequenceNumber_t& initial_sequence)
    {
        WriterProxy::start(attributes, initial_sequence);
        initial_acknack_test_->update_interval(reader_->getTimes().initial_acknack_delay);
        initial_acknack_test_->restart_timer();
    }

    void stop()
    {
        initial_acknack_test_->cancel_timer();
        WriterProxy::stop();
    }

private:

    StatefulReader* reader_;
    TimedEvent* initial_acknack_test_;
};

} // namespace internal
} // namespace testing

namespace eprosima {
namespace fastdds {
namespace rtps {

/**
 * This test checks that stopping a WriterProxy while one of its TimedEvents is being executed is thread safe.
 * In other words, no data races should be reported when running this test with a thread sanitizer.
 */
TEST(WriterProxyTests, WriterProxyStop)
{
    // Create actual events service
    eprosima::fastdds::rtps::ResourceEvent* service;
    service = new eprosima::fastdds::rtps::ResourceEvent();
    service->init_thread();

    // Synchronization primitives
    std::promise<void> promise;
    std::future<void> future = promise.get_future();

    // Create and initialize WriterProxyTest
    WriterProxyData wattr(4u, 1u);
    StatefulReader readerMock;

    ON_CALL(readerMock, getEventResource())
            .WillByDefault(::testing::ReturnRef(*service));
    EXPECT_CALL(readerMock, getEventResource()).Times(1u);
    testing::internal::WriterProxyTest* wproxy = new testing::internal::WriterProxyTest(&readerMock,
                    RemoteLocatorsAllocationAttributes(), ResourceLimitedContainerConfig(), *service, promise);
    wproxy->start(wattr, SequenceNumber_t());

    // Stopping a proxy in the middle of a TimedEvent execution is thread safe
    future.wait();
    wproxy->stop();

    delete wproxy;
    delete service;
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
