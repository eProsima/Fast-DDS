// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/**
 * @file SubscriberApp.hpp
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_BENCHMARK__SUBSCRIBERAPP_HPP
#define FASTDDS_EXAMPLES_CPP_BENCHMARK__SUBSCRIBERAPP_HPP

#include <condition_variable>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"
#include "types/Benchmark.hpp"
#include "types/Benchmark_small.hpp"
#include "types/Benchmark_medium.hpp"
#include "types/Benchmark_big.hpp"

using namespace eprosima::fastdds::dds;

namespace eprosima {
namespace fastdds {
namespace examples {
namespace benchmark {

class SubscriberApp : public Application, public DataWriterListener, public DataReaderListener
{
public:

    SubscriberApp(
            const CLIParser::subscriber_config& config);

    ~SubscriberApp();

    //! Publisher matched method
    void on_publication_matched(
            DataWriter* writer,
            const PublicationMatchedStatus& info) override;

    //! Subscriber matched method
    void on_subscription_matched(
            DataReader* reader,
            const SubscriptionMatchedStatus& info) override;

    //! Subscription callback
    void on_data_available(
            DataReader* reader) override;

    //! Run publisher
    void run() override;

    //! Stop publisher
    void stop() override;

private:

    //! Return the current state of execution
    bool is_stopped();

    BenchMark benchmark_;

    BenchMarkSmall benchmark_small_;

    BenchMarkMedium benchmark_medium_;

    BenchMarkBig benchmark_big_;

    DomainParticipant* participant_;

    Publisher* publisher_;

    Topic* topic_pub_;

    DataWriter* writer_;

    Subscriber* subscriber_;

    Topic* topic_sub_;

    DataReader* reader_;

    TypeSupport type_;

    uint16_t samples_;

    int16_t matched_;

    std::mutex mutex_;

    std::condition_variable cv_;

    std::atomic<bool> stop_;

    CLIParser::MsgSizeKind msg_size_;
};

} // namespace benchmark
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_EXAMPLES_CPP_BENCHMARK__SUBSCRIBERAPP_HPP
