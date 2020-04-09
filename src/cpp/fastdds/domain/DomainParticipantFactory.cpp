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

/**
 * @file DomainParticipantFactory.cpp
 *
 */

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/rtps/RTPSDomain.h>
#include <fastdds/rtps/participant/RTPSParticipant.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/domain/DomainParticipantImpl.hpp>

#include <fastdds/dds/log/Log.hpp>

#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/TypeObjectFactory.h>

using namespace eprosima::fastrtps::xmlparser;

using eprosima::fastrtps::ParticipantAttributes;
using eprosima::fastdds::dds::Log;

using eprosima::fastrtps::rtps::RTPSDomain;
using eprosima::fastrtps::rtps::RTPSParticipant;

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipantFactoryReleaser
{
public:

    ~DomainParticipantFactoryReleaser()
    {
        DomainParticipantFactory::delete_instance();
    }

};

static bool g_instance_initialized = false;
static std::mutex g_mtx;
static DomainParticipantFactoryReleaser s_releaser;
static DomainParticipantFactory* g_instance = nullptr;

DomainParticipantFactory::DomainParticipantFactory()
    : default_xml_profiles_loaded(false)
    , default_participant_qos_(PARTICIPANT_QOS_DEFAULT)
{
}

DomainParticipantFactory::~DomainParticipantFactory()
{
    {
        std::lock_guard<std::mutex> guard(mtx_participants_);
        for (auto it : participants_)
        {
            for (auto pit : it.second)
            {
                pit->disable();
            }
        }
        // Delete participants
        for (auto it : participants_)
        {
            for (auto pit = it.second.begin(); pit != it.second.end(); ++pit)
            {
                delete *pit;
            }
        }
        participants_.clear();
    }

    // Deletes DynamicTypes and TypeObject factories
    fastrtps::types::DynamicTypeBuilderFactory::delete_instance();
    fastrtps::types::DynamicDataFactory::delete_instance();
    fastrtps::types::TypeObjectFactory::delete_instance();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    eprosima::fastdds::dds::Log::KillThread();
}

DomainParticipantFactory* DomainParticipantFactory::get_instance()
{
    if (!g_instance_initialized)
    {
        std::lock_guard<std::mutex> lock(g_mtx);
        if (g_instance == nullptr)
        {
            g_instance = new DomainParticipantFactory();
            g_instance_initialized = true;
        }
    }
    return g_instance;
}

bool DomainParticipantFactory::delete_instance()
{
    std::lock_guard<std::mutex> lock(g_mtx);
    if (g_instance_initialized && g_instance != nullptr)
    {
        delete g_instance;
        g_instance = nullptr;
        g_instance_initialized = false;
        return true;
    }
    return false;
}

ReturnCode_t DomainParticipantFactory::delete_participant(
        DomainParticipant* part)
{
    using PartVectorIt = std::vector<DomainParticipantImpl*>::iterator;
    using VectorIt = std::map<DomainId_t, std::vector<DomainParticipantImpl*> >::iterator;

    if (part != nullptr)
    {
        if (part->has_active_entities())
        {
            return ReturnCode_t::RETCODE_PRECONDITION_NOT_MET;
        }
        std::lock_guard<std::mutex> guard(mtx_participants_);

        VectorIt vit = participants_.find(part->get_domain_id());

        if (vit != participants_.end())
        {
            for (PartVectorIt pit = vit->second.begin(); pit != vit->second.end();)
            {
                if ((*pit)->get_participant() == part
                        || (*pit)->get_participant()->guid() == part->guid())
                {
                    delete (*pit);
                    PartVectorIt next_it = vit->second.erase(pit);
                    pit = next_it;
                    break;
                }
                else
                {
                    ++pit;
                }
            }
            if (vit->second.empty())
            {
                participants_.erase(vit);
            }
            return ReturnCode_t::RETCODE_OK;
        }
    }
    return ReturnCode_t::RETCODE_ERROR;
}

DomainParticipant* DomainParticipantFactory::create_participant(
        DomainId_t did,
        const DomainParticipantQos& qos,
        DomainParticipantListener* listen,
        const StatusMask& mask)
{
    const DomainParticipantQos& pqos = (&qos == &PARTICIPANT_QOS_DEFAULT) ? default_participant_qos_ : qos;

    DomainParticipant* dom_part = new DomainParticipant(mask);
    DomainParticipantImpl* dom_part_impl = new DomainParticipantImpl(dom_part, pqos, listen);

    fastrtps::rtps::RTPSParticipantAttributes rtps_attr = get_attributes(pqos);
    RTPSParticipant* part = RTPSDomain::createParticipant(did, rtps_attr, &dom_part_impl->rtps_listener_);

    if (part == nullptr)
    {
        logError(DOMAIN_PARTICIPANT_FACTORY, "Problem creating RTPSParticipant");
        delete dom_part_impl;
        return nullptr;
    }

    dom_part_impl->rtps_participant_ = part;

    {
        std::lock_guard<std::mutex> guard(mtx_participants_);
        using VectorIt = std::map<DomainId_t, std::vector<DomainParticipantImpl*> >::iterator;
        VectorIt vector_it = participants_.find(did);

        if (vector_it == participants_.end())
        {
            // Insert the vector
            std::vector<DomainParticipantImpl*> new_vector;
            auto pair_it = participants_.insert(std::make_pair(did, std::move(new_vector)));
            vector_it = pair_it.first;
        }

        vector_it->second.push_back(dom_part_impl);
    }

    if (factory_qos_.entity_factory().autoenable_created_entities)
    {
        dom_part->enable();
    }

    part->set_check_type_function(
        [dom_part](const std::string& type_name) -> bool
                {
                    return dom_part->find_type(type_name).get() != nullptr;
                });

    return dom_part;
}

DomainParticipant* DomainParticipantFactory::lookup_participant(
        DomainId_t domain_id) const
{
    std::lock_guard<std::mutex> guard(mtx_participants_);

    auto it = participants_.find(domain_id);
    if (it != participants_.end() && it->second.size() > 0)
    {
        return it->second.front()->get_participant();
    }

    return nullptr;
}

std::vector<DomainParticipant*> DomainParticipantFactory::lookup_participants(
        DomainId_t domain_id) const
{
    std::lock_guard<std::mutex> guard(mtx_participants_);

    std::vector<DomainParticipant*> result;
    auto it = participants_.find(domain_id);
    if (it != participants_.end())
    {
        const std::vector<DomainParticipantImpl*>& v = it->second;
        for (auto pit = v.begin(); pit != v.end(); ++pit)
        {
            result.push_back((*pit)->get_participant());
        }
    }

    return result;
}

ReturnCode_t DomainParticipantFactory::get_default_participant_qos(
        DomainParticipantQos& qos) const
{
    qos = default_participant_qos_;
    return ReturnCode_t::RETCODE_OK;
}

const DomainParticipantQos& DomainParticipantFactory::get_default_participant_qos()
{
    return default_participant_qos_;
}

ReturnCode_t DomainParticipantFactory::set_default_participant_qos(
        const DomainParticipantQos& qos)
{
    if (&qos == &PARTICIPANT_QOS_DEFAULT)
    {
        DomainParticipantImpl::set_qos(default_participant_qos_, PARTICIPANT_QOS_DEFAULT, true);
        return ReturnCode_t::RETCODE_OK;
    }

    ReturnCode_t ret_val = DomainParticipantImpl::check_qos(qos);
    if (!ret_val)
    {
        return ret_val;
    }
    DomainParticipantImpl::set_qos(default_participant_qos_, qos, true);
    return ReturnCode_t::RETCODE_OK;
}

bool DomainParticipantFactory::load_XML_profiles_file(
        const std::string& xml_profile_file)
{
    if (false == default_xml_profiles_loaded)
    {
        XMLProfileManager::loadDefaultXMLFile();
        default_xml_profiles_loaded = true;
    }

    if (XMLP_ret::XML_ERROR == XMLProfileManager::loadXMLFile(xml_profile_file))
    {
        logError(DOMAIN, "Problem loading XML file '" << xml_profile_file << "'");
        return false;
    }
    return true;
}

fastrtps::rtps::RTPSParticipantAttributes DomainParticipantFactory::get_attributes(
        const DomainParticipantQos& qos)
{
    fastrtps::rtps::RTPSParticipantAttributes rtps_attr;
    rtps_attr.allocation = qos.allocation();
    rtps_attr.properties = qos.properties();
    rtps_attr.setName(qos.name());
    rtps_attr.prefix = qos.wire_protocol().prefix;
    rtps_attr.participantID = qos.wire_protocol().participant_id;
    rtps_attr.builtin = qos.wire_protocol().builtin;
    rtps_attr.port = qos.wire_protocol().port;
    rtps_attr.throughputController = qos.wire_protocol().throughput_controller;
    rtps_attr.defaultUnicastLocatorList = qos.wire_protocol().default_unicast_locator_list;
    rtps_attr.defaultMulticastLocatorList = qos.wire_protocol().default_multicast_locator_list;
    rtps_attr.userTransports = qos.transport().user_transports;
    rtps_attr.useBuiltinTransports = qos.transport().use_builtin_transports;
    rtps_attr.sendSocketBufferSize = qos.transport().send_socket_buffer_size;
    rtps_attr.listenSocketBufferSize = qos.transport().listen_socket_buffer_size;
    return rtps_attr;
}

ReturnCode_t DomainParticipantFactory::get_qos(
        DomainParticipantFactoryQos& qos) const
{
    qos = factory_qos_;
    return ReturnCode_t::RETCODE_OK;
}

ReturnCode_t DomainParticipantFactory::set_qos(
        const DomainParticipantFactoryQos& qos)
{
    ReturnCode_t ret_val = check_qos(qos);
    if (!ret_val)
    {
        return ret_val;
    }
    if (!can_qos_be_updated(factory_qos_, qos))
    {
        return ReturnCode_t::RETCODE_IMMUTABLE_POLICY;
    }
    set_qos(factory_qos_, qos, false);
    return ReturnCode_t::RETCODE_OK;
}

void DomainParticipantFactory::set_qos(
        DomainParticipantFactoryQos& to,
        const DomainParticipantFactoryQos& from,
        bool first_time)
{
    (void) first_time;
    //As all the Qos can always be updated and none of them need to be sent
    to = from;
}

ReturnCode_t DomainParticipantFactory::check_qos(
        const DomainParticipantFactoryQos& qos)
{
    (void) qos;
    //There is no restriction by the moment with the contained Qos
    return ReturnCode_t::RETCODE_OK;
}

bool DomainParticipantFactory::can_qos_be_updated(
        const DomainParticipantFactoryQos& to,
        const DomainParticipantFactoryQos& from)
{
    (void) to;
    (void) from;
    //All the DomainParticipantFactoryQos can be updated
    return true;
}

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
