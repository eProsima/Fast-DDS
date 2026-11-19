// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/rtps/builtin/data/ParticipantBuiltinTopicData.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include "BlackboxTests.hpp"
#include "../types/HelloWorldPubSubTypes.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

static unsigned env_or(
        const char* name,
        unsigned def)
{
    const char* value = std::getenv(name);
    return (nullptr != value) ? static_cast<unsigned>(std::atoi(value)) : def;
}

static rtps::Locator_t udp_locator(
        uint32_t port)
{
    rtps::Locator_t locator;
    rtps::IPLocator::setIPv4(locator, 127, 0, 0, 1);
    locator.port = port;
    return locator;
}

static rtps::Locator_t multicast_locator(
        uint32_t port)
{
    rtps::Locator_t locator;
    rtps::IPLocator::setIPv4(locator, 239, 255, 0, 1);
    locator.port = port;
    return locator;
}

class CountingListener : public DomainParticipantListener
{
public:

    void on_participant_discovery(
            DomainParticipant*,
            rtps::ParticipantDiscoveryStatus status,
            const ParticipantBuiltinTopicData&,
            bool&) override
    {
        if (rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT == status)
        {
            ++discovered;
        }
    }

    std::atomic<unsigned> discovered{0};
};

/**
 * Reproducer for the ABBA deadlock between a participant's EDP temporary `ProxyPool` and the
 * mutexes of its own builtin SEDP readers.
 *
 * Two opposite lock orders exist over those two resources:
 *
 *  - `EDPBaseSUBListener::add_reader_from_change()` takes a pool slot while the SEDP reader
 *    mutex is already held (the listener runs from `StatefulReader::processDataMsg()` with
 *    that mutex locked) and only releases the mutex afterwards.           [mutex -> pool]
 *  - `EDPSimple::assignRemoteEndpoints()` takes a pool slot and keeps it across
 *    `matched_writer_add_edp()`, which locks that very same reader mutex.  [pool -> mutex]
 *
 * `PDP::temp_reader_proxies_` has 4 slots and `ProxyPool::get()` waits on a condition variable
 * with no timeout, so once the service participant's metatraffic reception threads hold every
 * slot while waiting for the mutex, and one more thread holds the mutex while waiting for a
 * slot, the process is stuck for good.
 *
 * Under `INTRAPROCESS_FULL` (the default) an application thread deterministically ends up on
 * the `[mutex -> pool]` side: creating a DataReader on one participant delivers its DATA(r)
 * synchronously, on the calling thread, into the SEDP reader of another participant of the
 * same process.
 *
 * The service participant is given several metatraffic unicast locators on purpose. Each one
 * is served by its own reception thread, and each noise participant announces itself straight
 * to one of them through `initialPeersList`, so that several threads run
 * `assignRemoteEndpoints()` at the same time. A single metatraffic receiver would process the
 * DATA(p) messages one after another and could never hold more than one slot at a time.
 */
TEST(DDSEdpDeadlock, edp_proxy_pool_vs_sedp_reader_mutex)
{
    const unsigned receivers = env_or("FASTDDS_REPRO_RECEIVERS", 8u);
    const unsigned noise_count = env_or("FASTDDS_REPRO_NOISE", 2u);
    const unsigned run_seconds = env_or("FASTDDS_REPRO_SECONDS", 60u);
    const unsigned stall_seconds = env_or("FASTDDS_REPRO_STALL", 20u);
    const uint32_t domain_id = static_cast<uint32_t>(GET_PID() % 230);
    // Keep the service participant's ports clear of the 7400 + 250 * domain family.
    const uint32_t base_port = 31000u + static_cast<uint32_t>(GET_PID() % 200) * 32u;

    auto factory = DomainParticipantFactory::get_shared_instance();
    LibrarySettings old_library_settings;
    factory->get_library_settings(old_library_settings);
    {
        LibrarySettings library_settings;
        library_settings.intraprocess_delivery = IntraprocessDeliveryType::INTRAPROCESS_FULL;
        factory->set_library_settings(library_settings);
    }

    // A stalled run means the deadlock reproduced. The main thread cannot report it (it is one
    // of the blocked threads), so a watchdog aborts the process: the test fails and every
    // thread's stack stays in place for a debugger.
    std::atomic<uint64_t> progress{0};
    std::atomic<bool> stop_watchdog{false};
    std::thread watchdog([&]()
            {
                uint64_t last_seen = 0;
                auto last_change = std::chrono::steady_clock::now();
                while (!stop_watchdog.load())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    const uint64_t seen = progress.load();
                    if (seen != last_seen)
                    {
                        last_seen = seen;
                        last_change = std::chrono::steady_clock::now();
                        continue;
                    }
                    if (std::chrono::steady_clock::now() - last_change >
                    std::chrono::seconds(stall_seconds))
                    {
                        std::cerr << "\nDEADLOCK: no progress for " << stall_seconds << "s after "
                                  << last_seen << " DataReader create/delete steps.\n"
                                  << "Expect one thread in ProxyPool::get and several in "
                                  << "StatefulReader::matched_writer_add_edp.\n";
                        std::cerr.flush();
                        std::abort();
                    }
                }
            });

    {
        // Service participant: plays ROS2_SERVICE from the original report. Several metatraffic
        // unicast locators, so it has several metatraffic reception threads.
        DomainParticipantQos service_qos = PARTICIPANT_QOS_DEFAULT;
        for (unsigned i = 0; i < receivers; ++i)
        {
            service_qos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(
                udp_locator(base_port + i));
        }
        // Keep the standard multicast metatraffic locator, so the default participant still
        // discovers this one the usual way.
        service_qos.wire_protocol().builtin.metatrafficMulticastLocatorList.push_back(
            multicast_locator(7400u + 250u * domain_id));

        CountingListener service_listener;
        DomainParticipant* service = factory->create_participant(domain_id, service_qos,
                        &service_listener);
        ASSERT_NE(nullptr, service);
        DomainParticipant* def = factory->create_participant(domain_id, PARTICIPANT_QOS_DEFAULT);
        ASSERT_NE(nullptr, def);

        TypeSupport type_support(new HelloWorldPubSubType());
        ASSERT_EQ(RETCODE_OK, type_support.register_type(service, "HelloWorld"));
        ASSERT_EQ(RETCODE_OK, type_support.register_type(def, "HelloWorld"));

        // Endpoints on both sides, so the two participants match over SEDP and the
        // cross-participant intraprocess EDP path is in place.
        Subscriber* service_sub = service->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
        Publisher* service_pub = service->create_publisher(PUBLISHER_QOS_DEFAULT);
        ASSERT_NE(nullptr, service_sub);
        ASSERT_NE(nullptr, service_pub);
        Topic* service_topic = service->create_topic("edp_deadlock", "HelloWorld", TOPIC_QOS_DEFAULT);
        ASSERT_NE(nullptr, service_topic);
        ASSERT_NE(nullptr, service_sub->create_datareader(service_topic, DATAREADER_QOS_DEFAULT));
        ASSERT_NE(nullptr, service_pub->create_datawriter(service_topic, DATAWRITER_QOS_DEFAULT));

        Subscriber* def_sub = def->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
        ASSERT_NE(nullptr, def_sub);
        Topic* def_topic = def->create_topic("edp_deadlock", "HelloWorld", TOPIC_QOS_DEFAULT);
        ASSERT_NE(nullptr, def_topic);

        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "service discovered " << service_listener.discovered.load()
                  << " participant(s) before the noise starts (expect 1: the default one)\n";

        // Noise participants: each announces itself directly to one of the service
        // participant's metatraffic locators, so a different reception thread of the service
        // participant runs assignRemoteEndpoints() for each of them.
        std::atomic<bool> stop_noise{false};
        std::vector<std::thread> noise_threads;
        for (unsigned r = 0; r < receivers; ++r)
        {
            noise_threads.emplace_back([&, r]()
                    {
                        // One thread per metatraffic locator of the service participant, so
                        // that a different reception thread of that participant runs
                        // assignRemoteEndpoints() for each of them at the same time.
                        DomainParticipantQos noise_qos = PARTICIPANT_QOS_DEFAULT;
                        noise_qos.wire_protocol().builtin.initialPeersList.push_back(
                            udp_locator(base_port + r));

                        while (!stop_noise.load())
                        {
                            std::vector<DomainParticipant*> burst;
                            for (unsigned i = 0; i < noise_count && !stop_noise.load(); ++i)
                            {
                                DomainParticipant* participant =
                                factory->create_participant(domain_id, noise_qos);
                                if (nullptr != participant)
                                {
                                    burst.push_back(participant);
                                }
                            }
                            for (DomainParticipant* participant : burst)
                            {
                                participant->delete_contained_entities();
                                factory->delete_participant(participant);
                            }
                        }
                    });
        }

        // Main thread: the [mutex -> pool] side of the cycle.
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(run_seconds);
        while (std::chrono::steady_clock::now() < deadline)
        {
            DataReader* reader = def_sub->create_datareader(def_topic, DATAREADER_QOS_DEFAULT);
            ASSERT_NE(nullptr, reader);
            progress.fetch_add(1);
            ASSERT_EQ(RETCODE_OK, def_sub->delete_datareader(reader));
            progress.fetch_add(1);
        }

        stop_noise.store(true);
        for (std::thread& thread : noise_threads)
        {
            thread.join();
        }

        std::cout << "Completed " << (progress.load() / 2) << " create/delete cycles.\n";
        std::cout << "service discovered " << service_listener.discovered.load()
                  << " participant(s) in total\n";

        def->delete_contained_entities();
        service->delete_contained_entities();
        factory->delete_participant(def);
        factory->delete_participant(service);
    }

    stop_watchdog.store(true);
    watchdog.join();

    factory->set_library_settings(old_library_settings);
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
