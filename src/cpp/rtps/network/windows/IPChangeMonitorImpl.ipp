/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */

#include <atomic>

#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>

#include <fastdds/dds/log/Log.hpp>

#include <utils/thread.hpp>
#include <utils/threading.hpp>

// Need to link with Iphlpapi.lib and Ws2_32.lib
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

namespace eprosima {
namespace fastdds {
namespace rtps {

struct IPChangeMonitorImpl::IPChangeMonitorImplData
{
    explicit IPChangeMonitorImplData(
            IPChangeMonitorImpl& parent)
        : parent_(parent)
        , watch_thread_(
            create_thread(std::bind(&IPChangeMonitorImplData::run, this), ThreadSettings(), "dds.ip_monitor"))
    {
    }

    ~IPChangeMonitorImplData()
    {
        run_thread_ = false;
        // Cancel any pending NotifyAddrChange call
        CancelIPChangeNotify(&overlap_);
        if (watch_thread_.joinable())
        {
            watch_thread_.join();
        }
    }

private:

    void run()
    {
        overlap_.hEvent = WSACreateEvent();
        while (run_thread_)
        {
            HANDLE hand = NULL;
            DWORD ret = NotifyAddrChange(&hand, &overlap_);
            if (ret != ERROR_IO_PENDING)
            {
                // Error occurred, exit the thread
                EPROSIMA_LOG_ERROR(RTPSDOMAIN,
                        "Failed registering for IP change notifications, stopped monitoring IP changes");
                break;
            }
            // Wait for the event to be signaled
            if (run_thread_ && (WAIT_OBJECT_0 != WaitForSingleObject(overlap_.hEvent, INFINITE)))
            {
                // Error occurred, exit the thread
                EPROSIMA_LOG_ERROR(RTPSDOMAIN,
                        "Error waiting for IP change notification, stopped monitoring IP changes");
                break;
            }

            if (run_thread_)
            {
                // IP change detected, call the callback function
                parent_.ip_change_detected();
            }
        }

        WSACloseEvent(overlap_.hEvent);
    }

    std::atomic<bool> run_thread_{ true };
    IPChangeMonitorImpl& parent_;
    OVERLAPPED overlap_{};
    eprosima::thread watch_thread_;
};

IPChangeMonitorImpl::IPChangeMonitorImpl()
{
}

void IPChangeMonitorImpl::start_monitoring()
{
    if (!impl_data_)
    {
        impl_data_.reset(new IPChangeMonitorImplData(*this));
    }
}

void IPChangeMonitorImpl::stop_monitoring()
{
    if (impl_data_)
    {
        // Terminate the thread
        impl_data_.reset();
    }
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
