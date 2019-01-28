// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "idl/TypesPubSubTypes.h"
#include "idl/TypesTypeObject.h"

#include <thread>

#include "TestPublisher.h"
#include "TestSubscriber.h"

#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>
#include <fastrtps/types/TypeObjectFactory.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastrtps/log/Log.h>

#include <thread>
#include <memory>
#include <cstdlib>
#include <string>
#include <gtest/gtest.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class XTypes : public ::testing::Test
{
    public:

        XTypes()
        {
            //Log::SetVerbosity(Log::Info);
            //Log::SetCategoryFilter(std::regex("(SECURITY)"));
        }

        ~XTypes()
        {
            //Log::Reset();
            Log::KillThread();
            eprosima::fastrtps::Domain::stopAll();
        }
};

TEST_F(XTypes, ValidMinimal)
{
    BasicStructPubSubType type;
    const TypeObject* type_obj = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id = GetBasicStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    //DataRepresentationQosPolicy dataRepQos;
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

	//TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("ValidMinimal", 10, &type, type_obj, type_id, nullptr, "Pub1", nullptr, nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("ValidMinimal", 10, &type, type_obj, type_id, nullptr, "Sub1", nullptr, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

TEST_F(XTypes, InvalidMinimal)
{
    BasicStructPubSubType type1;
    BasicStructPubSubType type2; // Instead of BasicNamesStructPubSubType
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicNamesStructObject();
    const TypeIdentifier* type_id2 = GetBasicNamesStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    //DataRepresentationQosPolicy dataRepQos;
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

	//TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("ValidMinimal", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", nullptr, nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("ValidMinimal", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", nullptr, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 10);
    pub.waitDiscovery(false, 10);
}

// TEST_F(XTypes, ValidComplete)
// TEST_F(XTypes, InvalidComplete)
// TEST_F(XTypes, InvalidMinimalComplete)
// TEST_F(XTypes, TypeWideningPermitted)
// TEST_F(XTypes, TypeWideningBlocked)
// TEST_F(XTypes, SequenceBoundsIgnored)
// TEST_F(XTypes, SequenceBoundsManaged)
// TEST_F(XTypes, StringBoundsIgnored)
// TEST_F(XTypes, StringBoundsManaged)
// TEST_F(XTypes, MemberNamesIgnored)
// TEST_F(XTypes, MemberNamesManaged)

// TEST_F(XTypes, EmptyRepresentationLists)
// TEST_F(XTypes, EmptyRepresentationListsAndXCDR)
// TEST_F(XTypes, XCDRAndXCDR2)
// TEST_F(XTypes, XCDR2AndXCDR)
// TEST_F(XTypes, XCDR2AndXCDR2)
// TEST_F(XTypes, XMLAndXCDR2)
// TEST_F(XTypes, XMLAndXCDR)
// TEST_F(XTypes, XCDRAndXML)
// TEST_F(XTypes, XCDR2AndXML)

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
