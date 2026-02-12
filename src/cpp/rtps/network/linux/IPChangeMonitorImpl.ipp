/* Copyright(C) 2025, Proyectos y Sistemas de Mantenimiento SL(eProsima)
 *
 * This program is commercial software licensed under the terms of the
 * eProsima Software License Agreement Rev 03 (the "License")
 *
 * You may obtain a copy of the License at
 * https://www.eprosima.com/licenses/LICENSE-REV03
 */
/**
 * @file IPChangeMonitorImpl.cpp
 *
 * Contains Linux implementation of IPChangeMonitor
 */

#include <atomic>

#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>

#include <fastdds/dds/log/Log.hpp>

#include <utils/thread.hpp>
#include <utils/threading.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

struct IPChangeMonitorImpl::IPChangeMonitorImplData
{
    IPChangeMonitorImplData(
            int fd,
            IPChangeMonitorImpl& parent)
        : socket_(fd)
        , parent_(parent)
        , watch_thread_(create_thread(std::bind(&IPChangeMonitorImplData::run, this), ThreadSettings(),
                "dds.ip_monitor"))
    {
    }

    ~IPChangeMonitorImplData()
    {
        run_thread_ = false;
        if (watch_thread_.joinable())
        {
            watch_thread_.join();
        }
    }

    static int open_socket()
    {
        int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
        if (fd < 0)
        {
            // Error creating socket
            return -1;
        }

        struct sockaddr_nl addr;
        memset(&addr, 0, sizeof(addr));
        addr.nl_family = AF_NETLINK;
        addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR;
        if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        {
            // Error binding socket
            ::close(fd);
            return -1;
        }

        return fd;
    }

private:

    std::atomic<bool> run_thread_{ true };
    int socket_{ -1 };
    IPChangeMonitorImpl& parent_;
    eprosima::thread watch_thread_;

    void run()
    {
        while (run_thread_)
        {
            // Use select to wait for data with a timeout
            fd_set read_fds;
            fd_set err_fds;
            FD_ZERO(&read_fds);
            FD_ZERO(&err_fds);
            FD_SET(socket_, &read_fds);
            FD_SET(socket_, &err_fds);
            struct timeval timeout {};
            timeout.tv_sec = 1; // 1 second timeout
            int select_ret = select(socket_ + 1, &read_fds, nullptr, &err_fds, &timeout);
            if (select_ret <= 0 || FD_ISSET(socket_, &err_fds) || !FD_ISSET(socket_, &read_fds))
            {
                // Either:
                // - Error in select, possibly interrupted by a signal
                // - Timeout occurred
                // - Error on the socket
                // - No data to read
                // Just continue and check run_thread condition
                continue;
            }

            // Read one buffer from the netlink socket
            char buffer[4096];
            struct nlmsghdr* nlh = (struct nlmsghdr*)buffer;
            ssize_t len = recv(socket_, nlh, sizeof(buffer), 0);
            if (len < 0)
            {
                // Error receiving data
                continue;
            }

            // Process all messages in the buffer
            bool notify_ip_change = false;
            for (; NLMSG_OK(nlh, (unsigned int)len); nlh = NLMSG_NEXT(nlh, len))
            {
                if (nlh->nlmsg_type == NLMSG_DONE)
                {
                    break;
                }

                switch (nlh->nlmsg_type)
                {
                    case RTM_NEWLINK:
                    case RTM_DELLINK:
                    case RTM_NEWADDR:
                    case RTM_DELADDR:
                        // Network interface change detected
                        notify_ip_change = true;
                        break;
                    default:
                        break;
                }
            }

            if (notify_ip_change && run_thread_)
            {
                // IP change detected, call the callback function
                parent_.ip_change_detected();
            }
        }

        ::close(socket_);
    }

};

IPChangeMonitorImpl::IPChangeMonitorImpl()
{
}

void IPChangeMonitorImpl::start_monitoring()
{
    int fd = IPChangeMonitorImplData::open_socket();
    if (fd < 0)
    {
        EPROSIMA_LOG_ERROR(RTPSDOMAIN, "Error opening IP monitoring socket, cannot monitor IP changes");
        return;
    }

    if (!impl_data_)
    {
        impl_data_.reset(new IPChangeMonitorImplData(fd, *this));
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
