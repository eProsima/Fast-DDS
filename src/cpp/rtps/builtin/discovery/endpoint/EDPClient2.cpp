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

/**
 * @file EDPClient2.cpp
 *
 */

#include <fastdds/dds/log/Log.hpp>

#include "./EDPClient2.hpp"

namespace eprosima {
namespace fastdds {
namespace rtps {

using namespace fastrtps::rtps;

bool EDPClient2::processLocalReaderProxyData(
        RTPSReader* local_reader,
        ReaderProxyData* rdata)
{
    (void)local_reader;
    (void)rdata;
    // TODO DISCOVERY SERVER VERSION 2
    return true;
}

bool EDPClient2::processLocalWriterProxyData(
        RTPSWriter* local_writer,
        WriterProxyData* wdata)
{
    (void)local_writer;
    (void)wdata;
    // TODO DISCOVERY SERVER VERSION 2
    return true;
}

bool EDPClient2::removeLocalWriter(
        RTPSWriter* W)
{
    (void)W;
    // TODO DISCOVERY SERVER VERSION 2
    return true;
}

bool EDPClient2::removeLocalReader(
        RTPSReader* R)
{
    (void)R;
    // TODO DISCOVERY SERVER VERSION 2
    return true;
}

} /* namespace rtps */
} /* namespace fastdds */
} /* namespace eprosima */
