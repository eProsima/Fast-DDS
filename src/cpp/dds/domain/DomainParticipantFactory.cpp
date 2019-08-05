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

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <dds/domain/DomainParticipantImpl.hpp>

#include <fastrtps/utils/eClock.h>
#include <fastrtps/log/Log.h>

#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/TypeObjectFactory.h>

using namespace eprosima::fastrtps::xmlparser;

using eprosima::fastrtps::ParticipantAttributes;
using eprosima::fastrtps::Log;

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
    XMLProfileManager::DeleteInstance();

    fastrtps::eClock::my_sleep(100);
    fastrtps::Log::KillThread();
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

bool DomainParticipantFactory::delete_participant(
        DomainParticipant* part)
{
    if (part != nullptr)
    {
        std::lock_guard<std::mutex> guard(mtx_participants_);

        auto it = participants_.find(part->get_domain_id());

        if (it != participants_.end())
        {
            auto pit = std::find(it->second.begin(), it->second.end(), part->impl_);
            if (pit != it->second.end())
            {
                it->second.erase(pit);
                delete *pit;
                return true;
            }
        }
    }
    return false;
}

DomainParticipant* DomainParticipantFactory::create_participant(
        const std::string& participant_profile,
        DomainParticipantListener* listen)
{
    if (false == default_xml_profiles_loaded)
    {
        XMLProfileManager::loadDefaultXMLFile();
        default_xml_profiles_loaded = true;
    }

    ParticipantAttributes participant_att;
    if (XMLP_ret::XML_ERROR == XMLProfileManager::fillParticipantAttributes(participant_profile, participant_att))
    {
        logError(DOMAIN_PARTICIPANT_FACTORY, "Problem loading profile '" << participant_profile << "'");
        return nullptr;
    }

    return create_participant(participant_att, listen);
}

DomainParticipant* DomainParticipantFactory::create_participant(
        const ParticipantAttributes& att,
        DomainParticipantListener* listen)
{
    uint8_t domain_id = static_cast<uint8_t>(att.rtps.builtin.domainId);
    DomainParticipant* dom_part = new DomainParticipant();
    DomainParticipantImpl* dom_part_impl = new DomainParticipantImpl(att, dom_part, listen);
    RTPSParticipant* part = RTPSDomain::createParticipant(att.rtps, &dom_part_impl->rtps_listener_);

    if (part == nullptr)
    {
        logError(DOMAIN_PARTICIPANT_FACTORY, "Problem creating RTPSParticipant");
        delete dom_part_impl;
        return nullptr;
    }

    dom_part_impl->rtps_participant_ = part;

    {
        std::lock_guard<std::mutex> guard(mtx_participants_);
        participants_[domain_id].push_back(dom_part_impl);
    }
    return dom_part;
}

DomainParticipant* DomainParticipantFactory::lookup_participant(
        uint8_t domain_id) const
{
    std::lock_guard<std::mutex> guard(mtx_participants_);

    auto it = participants_.find(domain_id);
    if (it != participants_.end() && it->second.size() > 0)
    {
        return it->second.front()->get_participant();
    }

    return nullptr;
}

bool DomainParticipantFactory::get_default_participant_qos(
        ParticipantAttributes& participant_attributes) const
{
    if (false == default_xml_profiles_loaded)
    {
        XMLProfileManager::loadDefaultXMLFile();
        default_xml_profiles_loaded = true;
    }

    XMLProfileManager::getDefaultParticipantAttributes(participant_attributes);
    return true;
}

/* TODO
bool DomainParticipantFactory::set_default_participant_qos(
        const fastrtps::ParticipantAttributes &participant_qos)
{
    // TODO XMLProfileManager::setDefault...
    (void)participant_qos;
    logError(DOMAIN_PARTICIPANT_FACTORY, "Not implemented.");
    return false;
}
*/

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

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */
