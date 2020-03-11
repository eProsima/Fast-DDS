// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTDDS_SHAREDMEM_LOG_H_
#define _FASTDDS_SHAREDMEM_LOG_H_

#include <fastdds/rtps/common/Locator.h>
#include <fastrtps/utils/DBQueue.h>
#include <rtps/transport/shared_mem/SharedMemManager.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

class SHMPacketFileLogger
{
private:

    uint16_t dump_id_ = 0;
    FILE* f_;
    std::unique_ptr<SharedMemSegment::named_mutex> f_mutex_;

public:

    SHMPacketFileLogger(
            const std::string& filename,
            uint16_t dump_id)
        : dump_id_(dump_id)
    {
#if defined( _MSC_VER )
        // In Windows, shared-access specification is needed
        f_ = _fsopen(filename.c_str(), "a", _SH_DENYNO);
#else
        f_ = fopen(filename.c_str(), "a");
#endif

        if (f_ != nullptr)
        {
            std::hash<std::string> hash_fn;
            size_t filename_hash = hash_fn(filename);
            std::string mutex_name;

            try
            {
                mutex_name = "SHMPacketFile" + std::to_string(filename_hash) + "_mutex";

                f_mutex_ = SharedMemSegment::open_or_create_and_lock_named_mutex(mutex_name);

                f_mutex_->unlock();

            }
            catch (const std::exception& e)
            {
                logError(RTPS_TRANSPORT_SHM, "Failed to open/create interprocess mutex for packet_file_log: "
                        << filename << " named: " << mutex_name << " with err: "<< e.what());

                fclose(f_);
                f_ = nullptr;
            }
        }
        else
        {
            logError(RTPS_TRANSPORT_SHM, "Failed to open packet_file_log: " << filename);
        }
    }

    virtual ~SHMPacketFileLogger()
    {
        if (f_)
        {
            fclose(f_);
        }
    }

    void dump_packet(
            const std::string timestamp,
            const fastrtps::rtps::Locator_t& from,
            const fastrtps::rtps::Locator_t& to,
            const fastrtps::rtps::octet* buf,
            const uint32_t len)
    {
        try
        {
            if (f_ != NULL )
            {
                std::lock_guard<SharedMemSegment::named_mutex> interprocess_file_lock(*f_mutex_);

                uint32_t ipSize = len + 28;
                uint32_t udpSize = len + 8;

                // Timestamp in format '%H:%M:%S.'
                fprintf(f_, "%s ", timestamp.c_str());

                // IP header
                fprintf(f_, "000000 45 00 %02x %02x %02x %02x 00 00 11 11 00 00\n", (ipSize >> 8) & 0xFF, ipSize & 0xFF,
                        (dump_id_ >> 8) & 0xFF, dump_id_ & 0xFF);

                if (from.kind == 1 && fastrtps::rtps::IsAddressDefined(from))
                {
                    fprintf(f_, "00000c %02x %02x %02x %02x\n", from.address[12], from.address[13], from.address[14],
                            from.address[15]);
                }
                else
                {
                    std::stringstream ss;
                    ss << std::this_thread::get_id();
                    uint32_t thread_id = std::atoi(ss.str().c_str());
                    auto addr = reinterpret_cast<uint8_t*>(&thread_id);
                    fprintf(f_, "00000c %02x %02x %02x %02x\n", addr[0], addr[1], addr[2], addr[3]);
                }

                if (to.kind == 1 && fastrtps::rtps::IsAddressDefined(to))
                {
                    fprintf(f_, "000010 %02x %02x %02x %02x\n", to.address[12], to.address[13], to.address[14],
                            to.address[15]);
                }
                else
                {
                    fprintf(f_, "000010 %02x %02x %02x %02x\n", 0, 0, 0, 0);
                }

                // UDP header
                fprintf(f_, "000014 %02x %02x %02x %02x\n", (from.port >> 8) & 0xFF, from.port & 0xFF,
                        (to.port >> 8) & 0xFF,
                        to.port & 0xFF);
                fprintf(f_, "000018 %02x %02x 00 00", (udpSize >> 8) & 0xFF, udpSize & 0xFF);

                // Data
                for (uint32_t i = 0; i < len; i++)
                {
                    if ((i & 15) == 0)
                    {
                        fprintf(f_, "\n%06x", i + 28);
                    }
                    fprintf(f_, " %02x", buf[i]);
                }

                fprintf(f_, "\n\n");
                fflush(f_);
            }
        }
        catch (const std::exception&)
        {
            logError(RTPS_TRANSPORT_SHM, "Failed to lock interprocess mutex packet_file_log");
            return;
        }
    }
};

class SHMPacketFileConsumer
{
public:

    struct Pkt
    {
        std::string timestamp;
        fastrtps::rtps::Locator_t from;
        fastrtps::rtps::Locator_t to;
        std::shared_ptr<SharedMemManager::Buffer> buffer;
    };

    SHMPacketFileConsumer(
            const std::string& filename)
        : file_logger_(filename, 1)
    {
    }

    void Consume(
            const Pkt& packet)
    {
        file_logger_.dump_packet(packet.timestamp, packet.from, packet.to,
                static_cast<uint8_t*>(packet.buffer->data()), packet.buffer->size());
    }

private:

    SHMPacketFileLogger file_logger_;
};


/**
 * PacketLogger
 */
template<class TPacketConsumer>
class PacketsLog
{
public:

    ~PacketsLog()
    {
        Flush();
        KillThread();
    }

    //! Returns the logging engine to configuration defaults.
    void Reset()
    {
        std::unique_lock<std::mutex> configGuard(resources_.config_mutex);
        resources_.consumers.clear();
    }

    void RegisterConsumer(
            std::unique_ptr<TPacketConsumer>&& consumer)
    {
        std::unique_lock<std::mutex> guard(resources_.config_mutex);
        resources_.consumers.emplace_back(std::move(consumer));
    }

    void ClearConsumers()
    {
        std::unique_lock<std::mutex> working(resources_.cv_mutex);
        resources_.cv.wait(working,
                [&]()
                    {
                        return resources_.logs.BothEmpty();
                    });
        std::unique_lock<std::mutex> guard(resources_.config_mutex);
        resources_.consumers.clear();
    }

    //! Waits until no more log info is availabel
    void Flush()
    {
        std::unique_lock<std::mutex> guard(resources_.cv_mutex);

        if (!resources_.logging && !resources_.logging_thread)
        {
            // already killed
            return;
        }

        /*   Flush() two steps strategy:

            I must assure Log::Run swaps the queues because only swapping the queues the background content
            will be consumed (first Run() loop).

            Then, I must assure the new front queue content is consumed (second Run() loop).
         */

        int last_loop = -1;

        for (int i = 0; i < 2; ++i)
        {
            resources_.cv.wait(guard,
                    [&]()
                        {
                            /* I must avoid:
                             + the two calls be processed without an intermediate Run() loop (by using last_loop sequence number)
                             + deadlock by absence of Run() loop activity (by using BothEmpty() call)
                             */
                            return !resources_.logging ||
                            ( resources_.logs.Empty() &&
                            ( last_loop != resources_.current_loop || resources_.logs.BothEmpty()) );
                        });

            last_loop = resources_.current_loop;

        }
    }

    //! Stops the logging thread. It will re-launch on the next call to a successful log macro.
    void KillThread()
    {
        {
            std::unique_lock<std::mutex> guard(resources_.cv_mutex);
            resources_.logging = false;
            resources_.work = false;
        }

        if (resources_.logging_thread)
        {
            resources_.cv.notify_all();
            // The #ifdef workaround here is due to an unsolved MSVC bug, which Microsoft has announced
            // they have no intention of solving: https://connect.microsoft.com/VisualStudio/feedback/details/747145
            // Each VS version deals with post-main deallocation of threads in a very different way.
    #if !defined(_WIN32) || defined(FASTRTPS_STATIC_LINK) || _MSC_VER >= 1800
            resources_.logging_thread->join();
    #endif
            resources_.logging_thread.reset();
        }
    }

    // Note: In VS2013, if you're linking this class statically, you will have to call KillThread before leaving
    // main, due to an unsolved MSVC bug.

    void QueueLog(
            const typename TPacketConsumer::Pkt& packet)
    {
        {
            std::unique_lock<std::mutex> guard(resources_.cv_mutex);
            if (!resources_.logging && !resources_.logging_thread)
            {
                resources_.logging = true;
                resources_.logging_thread.reset(new std::thread(&PacketsLog<TPacketConsumer>::run, this));
            }
        }

        resources_.logs.Push(packet);
        {
            std::unique_lock<std::mutex> guard(resources_.cv_mutex);
            resources_.work = true;
        }
        resources_.cv.notify_all();
    }

    std::string now()
    {
        std::stringstream stream;
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::chrono::system_clock::duration tp = now.time_since_epoch();
        tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);
        auto ms = static_cast<unsigned>(tp / std::chrono::milliseconds(1));

    #if defined(_WIN32)
        struct tm timeinfo;
        localtime_s(&timeinfo, &now_c);
        stream << std::put_time(&timeinfo, "%T") << "." << std::setw(3) << std::setfill('0') << ms << " ";
        //#elif defined(__clang__) && !defined(std::put_time) // TODO arm64 doesn't seem to support std::put_time
        //    (void)now_c;
        //    (void)ms;
    #else
        stream << std::put_time(localtime(&now_c), "%T") << "." << std::setw(3) << std::setfill('0') << ms << " ";
    #endif
        return stream.str();
    }

private:

    struct Resources
    {
        eprosima::fastrtps::DBQueue<typename TPacketConsumer::Pkt> logs;
        std::vector<std::unique_ptr<SHMPacketFileConsumer> > consumers;
        std::unique_ptr<std::thread> logging_thread;

        // Condition variable segment.
        std::condition_variable cv;
        std::mutex cv_mutex;
        bool logging;
        bool work;
        int current_loop;

        // Context configuration.
        std::mutex config_mutex;

        Resources()
            : logging(false)
            ,work(false)
            ,current_loop(0)
        {
        }
    };

    Resources resources_;

    void run()
    {
        std::unique_lock<std::mutex> guard(resources_.cv_mutex);

        while (resources_.logging)
        {
            resources_.cv.wait(guard,
                    [&]()
                        {
                            return !resources_.logging || resources_.work;
                        });

            resources_.work = false;

            guard.unlock();
            {
                resources_.logs.Swap();
                while (!resources_.logs.Empty())
                {
                    std::unique_lock<std::mutex> configGuard(resources_.config_mutex);
                    for (auto& consumer : resources_.consumers)
                    {
                        consumer->Consume(resources_.logs.Front());
                    }

                    resources_.logs.Pop();
                }
            }
            guard.lock();

            // avoid overflow
            if (++resources_.current_loop > 10000)
            {
                resources_.current_loop = 0;
            }

            resources_.cv.notify_all();
        }
    }
};

} // eprosima
} // fastdds
} // rtps

#endif // _FASTDDS_SHAREDMEM_LOG_H_