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

/*
 * Both endpoints share the same type, so they must match.
*/
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

    pub.init("ValidMinimal", 10, &type, type_obj, type_id, nullptr, "Pub1", nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("ValidMinimal", 10, &type, type_obj, type_id, nullptr, "Sub1", nullptr, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in struct and members.
 * In this test we will trick the BasicNamesStruct name to test failure at the member names through
 * TypeIdentifier's hashes. The endpoints must not match.
*/
TEST_F(XTypes, MemberNamesManaged)
{
    BasicStructPubSubType type1;
    BasicNamesStructPubSubType type2;
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicNamesStructObject();
    const TypeIdentifier* type_id2 = GetBasicNamesStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    type2.setName(type1.getName()); // Avoid failure by struct name.

    //DataRepresentationQosPolicy dataRepQos;
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = false;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("ValidMinimal", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("ValidMinimal", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", nullptr, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in struct and members.
 * In this test we will trick the BasicNamesStruct name to test ignoring names by using ignore_member_names.
 * The endpoints must match.
*/
TEST_F(XTypes, MemberNamesIgnored)
{
    BasicStructPubSubType type1;
    BasicNamesStructPubSubType type2;
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicNamesStructObject();
    const TypeIdentifier* type_id2 = GetBasicNamesStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    type2.setName(type1.getName()); // Avoid failure by struct name.

    //DataRepresentationQosPolicy dataRepQos;
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("ValidMinimal", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("ValidMinimal", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", nullptr, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * Both endpoints share the same type, so they must match.
*/
TEST_F(XTypes, ValidComplete)
{
    BasicStructPubSubType type;
    const TypeObject* type_obj = GetCompleteBasicStructObject();
    const TypeIdentifier* type_id = GetBasicStructIdentifier(true);
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

    pub.init("ValidComplete", 10, &type, type_obj, type_id, nullptr, "Pub1", nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("ValidComplete", 10, &type, type_obj, type_id, nullptr, "Sub1", nullptr, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

TEST_F(XTypes, InvalidComplete)
{
    BasicStructPubSubType type1;
    BasicNamesStructPubSubType type2;
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(true);
    const TypeObject* type_obj2 = GetMinimalBasicNamesStructObject();
    const TypeIdentifier* type_id2 = GetBasicNamesStructIdentifier(true);
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

    pub.init("ValidComplete", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("ValidComplete", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", nullptr, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

TEST_F(XTypes, InvalidMinimalComplete)
{
    BasicStructPubSubType type1;
    BasicNamesStructPubSubType type2;
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(true);
    const TypeObject* type_obj2 = GetMinimalBasicNamesStructObject();
    const TypeIdentifier* type_id2 = GetBasicNamesStructIdentifier(true);
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

    pub.init("ValidComplete", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("ValidComplete", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", nullptr, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}


/*
 * Empty representation test:
 * Empty-Empty == Type
 * Empty-Empty != Type
 * Empty-Empty ~= Type
 *
 * Empty-XCDR1 == Type
 * Empty-XCDR1 != Type
 * Empty-XCDR1 ~= Type
 *
 * Empty-XCDR2 == Type
 * Empty-XCDR2 != Type
 * Empty-XCDR2 ~= Type
 *
 * Empty-XML == Type
 * Empty-XML != Type
 * Empty-XML ~= Type
 *
 * XCDR1-Empty == Type
 * XCDR1-Empty != Type
 * XCDR1-Empty ~= Type
 *
 * XCDR2-Empty == Type
 * XCDR2-Empty != Type
 * XCDR2-Empty ~= Type
 *
 * XML-Empty == Type
 * XML-Empty != Type
 * XML-Empty ~= Type
 */

/*
 * XML representation test:
 * XML-XML == Type
 * XML-XML != Type
 * XML-XML ~= Type
 *
 * XML-XCDR1 == Type
 * XML-XCDR1 != Type
 * XML-XCDR1 ~= Type
 *
 * XML-XCDR2 == Type
 * XML-XCDR2 != Type
 * XML-XCDR2 ~= Type
 *
 * XCDR1-XML == Type
 * XCDR1-XML != Type
 * XCDR1-XML ~= Type
 *
 * XCDR2-XML == Type
 * XCDR2-XML != Type
 * XCDR2-XML ~= Type
 */

/*
 * Pub XCDR2 + Sub XCDR1 incompatibility.
 * XCDR2-XCDR1 == Type
 * XCDR2-XCDR1 != Type
 *
 * Pub XCDR1 + Sub XCDR1 compatibility.
 * XCDR1-XCDR1 == Type
 * XCDR1-XCDR1 != Type
 *
 * Pub XCDR1 + Sub XCDR2 compatibility.
 * XCDR1-XCDR2 == Type
 * XCDR1-XCDR2 != Type
 *
 * Pub XCDR2 + Sub XCDR2 compatibility.
 * XCDR2-XCDR2 == Type
 * XCDR2-XCDR2 != Type
 */

/*
 * XCDR2-XCDR1 ~= Type
 * XCDR1-XCDR1 ~= Type
 * XCDR1-XCDR2 ~= Type
 * XCDR2-XCDR2 ~= Type
 *
 * Coercion Allow vs Disallow
 * Widening Allowed vs prevented
 * SequenceBounds Managed vs Ignored
 * StringBounds Managed vs Ignored
 * MemberNames Managed vs Ignored
 */

// TEST_F(XTypes, TypeWideningPermitted)
// TEST_F(XTypes, TypeWideningBlocked)
// TEST_F(XTypes, SequenceBoundsIgnored)
// TEST_F(XTypes, SequenceBoundsManaged)
// TEST_F(XTypes, StringBoundsIgnored)
// TEST_F(XTypes, StringBoundsManaged)
// TEST_F(XTypes, )
// TEST_F(XTypes, )

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
