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
 * @file ClientApp.hpp
 *
 */

#ifndef FASTDDS_EXAMPLES_CPP_REQUEST_REPLY__CLIENTAPP_HPP
#define FASTDDS_EXAMPLES_CPP_REQUEST_REPLY__CLIENTAPP_HPP

#include <array>
#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <limits>
#include <mutex>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "Application.hpp"
#include "CLIParser.hpp"

namespace eprosima {
namespace fastdds {
namespace examples {
namespace request_reply {

using namespace eprosima::fastdds::dds;

class ClientApp : public Application, public DataReaderListener, public DataWriterListener
{
public:

    ClientApp(
            const CLIParser::config& config,
            const std::string& service_name);

    ~ClientApp();

    //! Run subscriber
    void run() override;

    //! Trigger the end of execution
    void stop() override;

    //! Publication matched method
    void on_publication_matched(
            DataWriter* writer,
            const PublicationMatchedStatus& info) override;

    //! Subscription matched method
    void on_subscription_matched(
            DataReader* reader,
            const SubscriptionMatchedStatus& info) override;

    //! Reply received method
    void on_data_available(
            DataReader* reader) override;

private:

    struct RequestInput
    {
        RequestInput(const CLIParser::config& config);

        CLIParser::Operation operation = CLIParser::Operation::UNDEFINED;

        std::int16_t x = 0;

        std::int16_t y = 0;
    };

    RequestInput request_input_;

    DomainParticipant* participant_;

    TypeSupport request_type_;

    Topic* request_topic_;

    Publisher* publisher_;

    DataWriter* request_writer_;

    TypeSupport reply_type_;

    Topic* reply_topic_;

    Subscriber* subscriber_;

    DataReader* reply_reader_;

    mutable std::mutex mtx_;

    std::condition_variable cv_;

    template<std::size_t set_size>
    class Uint8CountSet
    {
    public:
        void increase(const size_t& position)
        {
            assert(position <= (set_size - 1));
            if (inner_set_[position] < std::numeric_limits<std::uint8_t>::max())
            {
                inner_set_[position] += 1;
            }
        }

        void decrease(const size_t& position)
        {
            assert(position <= (set_size - 1));
            if (inner_set_[position] > 0)
            {
                inner_set_[position] -= 1;
            }
        }

        bool all()
        {
            bool all_different_from_zero = true;

            for (auto i = 0; i < set_size; i++)
            {
                if (inner_set_[i] == 0)
                {
                    all_different_from_zero = false;
                    break;
                }
            }

            return all_different_from_zero;
        }

    private:
        std::array<std::uint8_t, set_size> inner_set_ = {0};
    };

    /**
     * @brief Set to represent the matched count of the request-reply endpoints:
     *
     * - Position 0 represents matching status of request writer
     * - Position 1 represents matching status of reply reader
     */
    Uint8CountSet<2> matched_state_;

    static constexpr std::size_t request_writer_position_ = 0;

    static constexpr std::size_t reply_reader_position_ = 1;
};

} // namespace request_reply
} // namespace examples
} // namespace fastdds
} // namespace eprosima

#endif /* FASTDDS_EXAMPLES_CPP_REQUEST_REPLY__CLIENTAPP_HPP */
