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

// Link-seam implementation of EDP: replaces src/cpp/rtps/builtin/discovery/endpoint/EDP.cpp. No proxy is unpaired and
// no remote endpoint is paired.

#include <rtps/builtin/discovery/endpoint/EDP.h>

namespace eprosima {
namespace fastdds {
namespace rtps {

EDP::~EDP()
{
}

bool EDP::unpairWriterProxy(
        const GUID_t&,
        const GUID_t&,
        bool)
{
    return false;
}

bool EDP::unpairReaderProxy(
        const GUID_t&,
        const GUID_t&)
{
    return false;
}

#if HAVE_SECURITY
bool EDP::pairing_reader_proxy_with_local_writer(
        const GUID_t&,
        const GUID_t&,
        ReaderProxyData&)
{
    return false;
}

bool EDP::pairing_writer_proxy_with_local_reader(
        const GUID_t&,
        const GUID_t&,
        WriterProxyData&)
{
    return false;
}

bool EDP::pairing_remote_reader_with_local_writer_after_security(
        const GUID_t&,
        const ReaderProxyData&)
{
    return false;
}

bool EDP::pairing_remote_writer_with_local_reader_after_security(
        const GUID_t&,
        const WriterProxyData&)
{
    return false;
}

#endif // if HAVE_SECURITY

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
