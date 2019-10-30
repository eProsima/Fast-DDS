// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#pragma once

#include <stdint.h>
#include <string>
#include <atomic>
#include <memory>

/**
 * Lower layer interface of the shared memory transport 
 */
class SharedMemManager
{
public:

    SharedMemManager()
    {
    }

    class Buffer
    {
    public:
        virtual void* data() = 0;
        virtual uint32_t size() = 0;
    };

    class Segment
    {
    public:

        Segment()
        {
        }

        std::shared_ptr<Buffer> alloc_buffer(
                uint32_t size)
        {
            (void)size;

            return nullptr;
        }
    }; // Segment

    class Port;
    class Reader
    {
    public:

        std::shared_ptr<Buffer> pop()
        {
            return nullptr;
        }
    }; // Reader

    class Port
    {
    public:

        void push(
                std::shared_ptr<Buffer> buffer)
        {
            (void)buffer;
        }

        std::shared_ptr<Reader> create_reader()
        {
            return nullptr;
        }
    }; // Port

    std::shared_ptr<Segment> create_segment(
            uint32_t size)
    {
        (void)size;

        return nullptr;
    }

    std::shared_ptr<Port> open_port(
            uint32_t port_id, 
            uint32_t max_descriptors)
    {
        (void)port_id;
        (void)max_descriptors;

        return nullptr;
    }
};