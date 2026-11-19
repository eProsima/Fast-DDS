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

// Link-seam implementation of PDP: replaces src/cpp/rtps/builtin/discovery/participant/PDP.cpp. The members are set
// up as the real constructor does, but no proxy is preallocated. Non-virtual methods tests need are forwarded to the
// PDPMock companion of the instance, and the virtual ones do nothing.

#include <rtps/builtin/discovery/participant/PDP.h>

#include <mock/rtps/builtin/discovery/participant/PDPMock.hpp>
#include <rtps/builtin/discovery/participant/PDPEndpoints.hpp>

namespace eprosima {
namespace fastdds {
namespace rtps {

using Companion = mock::MockCompanion<PDP>;

PDP::PDP (
        BuiltinProtocols* built,
        const RTPSParticipantAllocationAttributes& allocation)
    : mp_builtin(built)
    , mp_RTPSParticipant(nullptr)
    , mp_EDP(nullptr)
    , participant_proxies_number_(0)
    , participant_proxies_(allocation.participants)
    , participant_proxies_pool_(allocation.participants)
    , reader_proxies_number_(0)
    , reader_proxies_pool_(allocation.total_readers())
    , writer_proxies_number_(0)
    , writer_proxies_pool_(allocation.total_writers())
    , m_hasChangedLocalPDP(true)
    , temp_reader_proxies_({
                allocation.locators.max_unicast_locators,
                allocation.locators.max_multicast_locators,
                allocation.data_limits,
                allocation.content_filter})
    , temp_writer_proxies_({
                allocation.locators.max_unicast_locators,
                allocation.locators.max_multicast_locators,
                allocation.data_limits})
    , mp_mutex(new std::recursive_mutex())
    , proxy_observer_(nullptr)
    , resend_participant_info_event_(nullptr)
{
    Companion::attach(this);
}

PDP::~PDP()
{
    Companion::detach(this);
    delete mp_mutex;
}

void PDP::initializeParticipantProxyData(
        ParticipantProxyData*)
{
}

void PDP::pre_enable_actions()
{
}

void PDP::announceParticipantState(
        bool,
        bool)
{
}

void PDP::stopParticipantAnnouncement()
{
}

void PDP::resetParticipantAnnouncement()
{
}

bool PDP::remove_remote_participant(
        const GUID_t&,
        ParticipantDiscoveryStatus)
{
    return false;
}

#if HAVE_SECURITY
bool PDP::pairing_remote_writer_with_local_reader_after_security(
        const GUID_t&,
        const WriterProxyData&)
{
    return false;
}

bool PDP::pairing_remote_reader_with_local_writer_after_security(
        const GUID_t&,
        const ReaderProxyData&)
{
    return false;
}

#endif // HAVE_SECURITY

bool PDP::get_all_local_proxies(
        std::vector<GUID_t>&)
{
    return false;
}

bool PDP::get_serialized_proxy(
        const GUID_t&,
        CDRMessage_t*)
{
    return false;
}

void PDP::local_participant_attributes_update_nts(
        const RTPSParticipantAttributes&)
{
}

void PDP::update_endpoint_locators_if_default_nts(
        const std::vector<BaseWriter*>&,
        const std::vector<BaseReader*>&,
        const RTPSParticipantAttributes&,
        const RTPSParticipantAttributes&)
{
}

ReaderProxyData* PDP::addReaderProxyData(
        const GUID_t& reader_guid,
        GUID_t& participant_guid,
        std::function<bool(ReaderProxyData*, bool, const ParticipantProxyData&)> initializer_func)
{
    return Companion::get(this).addReaderProxyData(reader_guid, participant_guid, initializer_func);
}

WriterProxyData* PDP::addWriterProxyData(
        const GUID_t& writer_guid,
        GUID_t& participant_guid,
        std::function<bool(WriterProxyData*, bool, const ParticipantProxyData&)> initializer_func)
{
    return Companion::get(this).addWriterProxyData(writer_guid, participant_guid, initializer_func);
}

void PDP::notify_incompatible_qos_matching(
        const GUID_t& local_guid,
        const GUID_t& remote_guid,
        const fastdds::dds::PolicyMask& incompatible_qos) const
{
    Companion::get(this).notify_incompatible_qos_matching(local_guid, remote_guid, incompatible_qos);
}

bool PDP::lookupReaderProxyData(
        const GUID_t&,
        ReaderProxyData&)
{
    return false;
}

bool PDP::lookupWriterProxyData(
        const GUID_t&,
        WriterProxyData&)
{
    return false;
}

CDRMessage_t PDP::get_participant_proxy_data_serialized(
        Endianness_t endian)
{
    return Companion::get(this).get_participant_proxy_data_serialized(endian);
}

} // namespace rtps
} // namespace fastdds
} // namespace eprosima
