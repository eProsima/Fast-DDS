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

/**** NO TYPE OBJECT ****/

/*
 * Both endpoints share the same type without DataRepresentationQos, so they must match.
 * This represents classical match.
*/
TEST_F(XTypes, NoTypeObjectSameType)
{
    BasicStructPubSubType type;
    TestPublisher pub;
    TestSubscriber sub;

    pub.init("NoTypeObjectSameType", 10, &type, nullptr, nullptr, nullptr, "Pub1", nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("NoTypeObjectSameType", 10, &type, nullptr, nullptr, nullptr, "Sub1", nullptr, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * Both endpoints share the same type without DataRepresentationQosm, but they force type validation,
 * so they must not match.
*/
TEST_F(XTypes, NoTypeObjectSameTypeForce)
{
    BasicStructPubSubType type;
    TestPublisher pub;
    TestSubscriber sub;

    TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    typeConQos.m_force_type_validation = false;

    pub.init("NoTypeObjectSameTypeForce", 10, &type, nullptr, nullptr, nullptr, "Pub1", nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("NoTypeObjectSameTypeForce", 10, &type, nullptr, nullptr, nullptr, "Sub1", nullptr, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * The endpoints have different types without DataRepresentationQos, so they must not match.
 * This represents classical mismatch.
*/
TEST_F(XTypes, NoTypeObjectDifferentType)
{
    BasicStructPubSubType type;
    BasicNamesStructPubSubType type2;
    TestPublisher pub;
    TestSubscriber sub;

    pub.init("NoTypeObjectDifferentType", 10, &type, nullptr, nullptr, nullptr, "Pub1", nullptr);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("NoTypeObjectDifferentType", 10, &type2, nullptr, nullptr, nullptr, "Sub1", nullptr, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/**** TYPE OBJECT V1 ****/

/*
 * Both endpoints share the same type using XCDR1 so they must match.
 * This represents classical match.
*/
TEST_F(XTypes, TypeObjectV1SameType)
{
    BasicStructPubSubType type;
    const TypeObject* type_obj = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id = GetBasicStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
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

    pub.init("TypeObjectV1SameType", 10, &type, type_obj, type_id, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1SameType", 10, &type, type_obj, type_id, nullptr, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * Both endpoints different types using XCDR1 so they must not match even with full coercion allowed.
*/
TEST_F(XTypes, TypeObjectV1DifferentType)
{
    BasicStructPubSubType type1;
    BasicBadStructPubSubType type2;
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicBadStructObject();
    const TypeIdentifier* type_id2 = GetBasicBadStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    typeConQos.m_ignore_sequence_bounds = true;
    typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = false;
    typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1DifferentType", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1DifferentType", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in members.
 * This test checks failure at the member names through TypeIdentifier's hashes. The endpoints must not match.
*/
TEST_F(XTypes, TypeObjectV1NamesManaged)
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

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
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

    pub.init("TypeObjectV1NamesManaged", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1NamesManaged", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in struct and members.
 * This test checks ignoring at the member names through TypeIdentifier's hashes. The endpoints must match.
*/
TEST_F(XTypes, TypeObjectV1NamesIgnored)
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

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
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

    pub.init("TypeObjectV1NamesIgnored", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1NamesIgnored", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in struct and members.
 * This test checks ignoring at the member names when disallowed type coercion has no effect.
 * The endpoints must not match.
*/
TEST_F(XTypes, TypeObjectV1NamesIgnoredDisallow)
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

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1NamesIgnoredDisallow", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1NamesIgnoredDisallow", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicWideStruct are similar structures, but with "Wide" adds a member.
 * This test checks type widening. The endpoints must match.
*/
TEST_F(XTypes, TypeObjectV1TypeWidening)
{
    BasicStructPubSubType type1;
    BasicWideStructPubSubType type2;
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicWideStructObject();
    const TypeIdentifier* type_id2 = GetBasicWideStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1TypeWidening", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1TypeWidening", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * BasicStruct and BadBasicWideStruct are similar structures, but with "Wide" adds a member, and in this case
 * modifies the type of other member. The endpoints must not match.
*/
TEST_F(XTypes, TypeObjectV1BadTypeWidening)
{
    BasicStructPubSubType type1;
    BadBasicWideStructPubSubType type2;
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBadBasicWideStructObject();
    const TypeIdentifier* type_id2 = GetBadBasicWideStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1BadTypeWidening", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1BadTypeWidening", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicWideStruct are similar structures, but with "Wide" adds a member.
 * This test checks prevent type widening when assigning narrow to wide. The endpoints must match.
*/
TEST_F(XTypes, TypeObjectV1TypeWideningPreventedNarrowToWide)
{
    BasicStructPubSubType type1;
    BasicWideStructPubSubType type2;
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicWideStructObject();
    const TypeIdentifier* type_id2 = GetBasicWideStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = true;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1TypeWideningPreventedNarrowToWide", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1TypeWideningPreventedNarrowToWide", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * BasicStruct and BasicWideStruct are similar structures, but with "Wide" adds a member.
 * This test checks prevent type widening when assining from wide to narrow. The endpoints must not match.
*/
TEST_F(XTypes, TypeObjectV1TypeWideningPreventedWideToNarrow)
{
    BasicStructPubSubType type1;
    BasicWideStructPubSubType type2;
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicWideStructObject();
    const TypeIdentifier* type_id2 = GetBasicWideStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = true;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1TypeWideningPreventedWideToNarrow", 10, &type2, type_obj2, type_id2, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1TypeWideningPreventedWideToNarrow", 10, &type1, type_obj1, type_id1, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicWideStruct are similar structures, but with "Wide" adds a member, but coercion is disallowed.
 * This test checks type widening. The endpoints must match.
*/
TEST_F(XTypes, TypeObjectV1TypeWideningDisallow)
{
    BasicStructPubSubType type1;
    BasicWideStructPubSubType type2;
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalBasicWideStructObject();
    const TypeIdentifier* type_id2 = GetBasicWideStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    //typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1TypeWideningDisallow", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1TypeWideningDisallow", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * SequenceStruct and SequenceBoundsStruct are similar structures, but with "Bounds" the size of the sequence is bigger.
 * This test checks sequence bounds. The endpoints must match.
*/
TEST_F(XTypes, TypeObjectV1SequenceBoundsIgnored)
{
    SequenceStructPubSubType type1;
    SequenceBoundsStructPubSubType type2;
    const TypeObject* type_obj1 = GetMinimalSequenceStructObject();
    const TypeIdentifier* type_id1 = GetSequenceStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalSequenceBoundsStructObject();
    const TypeIdentifier* type_id2 = GetSequenceBoundsStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    typeConQos.m_ignore_sequence_bounds = true;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1SequenceBoundsIgnored", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1SequenceBoundsIgnored", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * SequenceStruct and SequenceBoundsStruct are similar structures, but with "Bounds" the size of the sequence is bigger.
 * This test checks sequence bounds. The endpoints must not match.
*/
TEST_F(XTypes, TypeObjectV1SequenceBoundsManaged)
{
    SequenceStructPubSubType type1;
    SequenceBoundsStructPubSubType type2;
    const TypeObject* type_obj1 = GetMinimalSequenceStructObject();
    const TypeIdentifier* type_id1 = GetSequenceStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalSequenceBoundsStructObject();
    const TypeIdentifier* type_id2 = GetSequenceBoundsStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    typeConQos.m_ignore_sequence_bounds = false;
    //typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1SequenceBoundsManaged", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1SequenceBoundsManaged", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * StringStruct and LargeStringStruct are similar structures, but with "Large" the size of the string is bigger.
 * This test checks string bounds. The endpoints must match.
*/
TEST_F(XTypes, TypeObjectV1LargeStringIgnored)
{
    StringStructPubSubType type1;
    LargeStringStructPubSubType type2;
    const TypeObject* type_obj1 = GetMinimalStringStructObject();
    const TypeIdentifier* type_id1 = GetStringStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalLargeStringStructObject();
    const TypeIdentifier* type_id2 = GetLargeStringStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = true;
    typeConQos.m_ignore_string_bounds = true;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1LargeStringIgnored", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1LargeStringIgnored", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * StringStruct and LargeStringStruct are similar structures, but with "Large" the size of the string is bigger.
 * This test checks string bounds. The endpoints must not match.
*/
TEST_F(XTypes, TypeObjectV1LargeStringManaged)
{
    StringStructPubSubType type1;
    LargeStringStructPubSubType type2;
    const TypeObject* type_obj1 = GetMinimalStringStructObject();
    const TypeIdentifier* type_id1 = GetStringStructIdentifier(false);
    const TypeObject* type_obj2 = GetMinimalLargeStringStructObject();
    const TypeIdentifier* type_id2 = GetLargeStringStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    //typeConQos.m_ignore_sequence_bounds = false;
    typeConQos.m_ignore_string_bounds = false;
    //typeConQos.m_ignore_member_names = true;
    //typeConQos.m_prevent_type_widening = false;
    //typeConQos.m_force_type_validation = false;

    pub.init("TypeObjectV1LargeStringManaged", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1LargeStringManaged", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * Both endpoints share the same type using XCDR1 so they must match, in this case using Complete.
*/
TEST_F(XTypes, TypeObjectV1SameTypeComplete)
{
    BasicStructPubSubType type;
    const TypeObject* type_obj = GetCompleteBasicStructObject();
    const TypeIdentifier* type_id = GetBasicStructIdentifier(true);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
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

    pub.init("TypeObjectV1SameTypeComplete", 10, &type, type_obj, type_id, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1SameTypeComplete", 10, &type, type_obj, type_id, nullptr, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * Both endpoints use compatible but different types using XCDR1 but without coercion so they must not match,
 * in this case using Complete.
*/
TEST_F(XTypes, TypeObjectV1InvalidComplete)
{
    BasicStructPubSubType type1;
    BasicNamesStructPubSubType type2;
    const TypeObject* type_obj1 = GetCompleteBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(true);
    const TypeObject* type_obj2 = GetCompleteBasicNamesStructObject();
    const TypeIdentifier* type_id2 = GetBasicNamesStructIdentifier(true);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
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

    pub.init("TypeObjectV1InvalidComplete", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeObjectV1InvalidComplete", 10, &type2, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * Both endpoints the same type using XCDR1 but one using minimal and the other complete.
 * They shouldn't match.
*/
TEST_F(XTypes, MixingMinimalAndComplete)
{
    BasicStructPubSubType type1;
    const TypeObject* type_obj1 = GetMinimalBasicStructObject();
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeObject* type_obj2 = GetCompleteBasicStructObject();
    const TypeIdentifier* type_id2 = GetBasicStructIdentifier(true);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
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

    pub.init("MixingMinimalAndComplete", 10, &type1, type_obj1, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("MixingMinimalAndComplete", 10, &type1, type_obj2, type_id2, nullptr, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/**** TYPE IDENTIFIER ****/

/*
 * Both endpoints share the same type using XCDR1 so they must match.
 * This represents classical match.
*/
TEST_F(XTypes, TypeIdentifierSameType)
{
    BasicStructPubSubType type;
    const TypeIdentifier* type_id = GetBasicStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
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

    pub.init("TypeIdentifierSameType", 10, &type, nullptr, type_id, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeIdentifierSameType", 10, &type, nullptr, type_id, nullptr, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * Both endpoints different types using XCDR1 so they must not match even with full coercion allowed.
*/
TEST_F(XTypes, TypeIdentifierDifferentType)
{
    BasicStructPubSubType type1;
    BasicBadStructPubSubType type2;
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeIdentifier* type_id2 = GetBasicBadStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    typeConQos.m_ignore_sequence_bounds = true;
    typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = false;
    typeConQos.m_force_type_validation = false;

    pub.init("TypeIdentifierDifferentType", 10, &type1, nullptr, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeIdentifierDifferentType", 10, &type2, nullptr, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in members.
 * This test checks failure at the member names through TypeIdentifier's hashes. The endpoints must not match.
*/
TEST_F(XTypes, TypeIdentifierNamesManaged)
{
    BasicStructPubSubType type1;
    BasicNamesStructPubSubType type2;
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeIdentifier* type_id2 = GetBasicNamesStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
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

    pub.init("TypeIdentifierNamesManaged", 10, &type1, nullptr, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeIdentifierNamesManaged", 10, &type2, nullptr, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in struct and members.
 * This test checks ignoring at the member names through TypeIdentifier's hashes. The endpoints must match.
*/
TEST_F(XTypes, TypeIdentifierNamesIgnored)
{
    BasicStructPubSubType type1;
    BasicNamesStructPubSubType type2;
    const TypeIdentifier* type_id1 = GetBasicStructIdentifier(false);
    const TypeIdentifier* type_id2 = GetBasicNamesStructIdentifier(false);
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
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

    pub.init("TypeIdentifierNamesIgnored", 10, &type1, nullptr, type_id1, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeIdentifierNamesIgnored", 10, &type2, nullptr, type_id2, nullptr, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/**** TYPE INFORMATION ****/

/*
 * Both endpoints share the same type using XCDR1 so they must match.
 * This represents classical match.
*/
TEST_F(XTypes, TypeInformationSameType)
{
    BasicStructPubSubType type;
    const TypeInformation* type_info = TypeObjectFactory::get_instance()->get_type_information("BasicStruct");
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
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

    pub.init("TypeInformationSameType", 10, &type, nullptr, nullptr, type_info, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeInformationSameType", 10, &type, nullptr, nullptr, type_info, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * Both endpoints different types using XCDR1 so they must not match even with full coercion allowed.
*/
TEST_F(XTypes, TypeInformationDifferentType)
{
    BasicStructPubSubType type1;
    BasicBadStructPubSubType type2;
    const TypeInformation* type_info1 = TypeObjectFactory::get_instance()->get_type_information("BasicStruct");
    const TypeInformation* type_info2 = TypeObjectFactory::get_instance()->get_type_information("BasicBadStruct");
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    TypeConsistencyEnforcementQosPolicy typeConQos;
    typeConQos.m_kind = TypeConsistencyKind::ALLOW_TYPE_COERCION;
    //typeConQos.m_kind = TypeConsistencyKind::DISALLOW_TYPE_COERCION;
    typeConQos.m_ignore_sequence_bounds = true;
    typeConQos.m_ignore_string_bounds = true;
    typeConQos.m_ignore_member_names = true;
    typeConQos.m_prevent_type_widening = false;
    typeConQos.m_force_type_validation = false;

    pub.init("TypeInformationDifferentType", 10, &type1, nullptr, nullptr, type_info1, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeInformationDifferentType", 10, &type2, nullptr, nullptr, type_info2, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in members.
 * This test checks failure at the member names through TypeInformation's hashes. The endpoints must not match.
*/
TEST_F(XTypes, TypeInformationNamesManaged)
{
    BasicStructPubSubType type1;
    BasicNamesStructPubSubType type2;
    const TypeInformation* type_info1 = TypeObjectFactory::get_instance()->get_type_information("BasicStruct");
    const TypeInformation* type_info2 = TypeObjectFactory::get_instance()->get_type_information("BasicNamesStruct");
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
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

    pub.init("TypeInformationNamesManaged", 10, &type1, nullptr, nullptr, type_info1, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeInformationNamesManaged", 10, &type2, nullptr, nullptr, type_info2, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * BasicStruct and BasicNamesStruct are similar structures, but with different names in struct and members.
 * This test checks ignoring at the member names through TypeInformation's hashes. The endpoints must match.
*/
TEST_F(XTypes, TypeInformationNamesIgnored)
{
    BasicStructPubSubType type1;
    BasicNamesStructPubSubType type2;
    const TypeInformation* type_info1 = TypeObjectFactory::get_instance()->get_type_information("BasicStruct");
    const TypeInformation* type_info2 = TypeObjectFactory::get_instance()->get_type_information("BasicNamesStruct");
    TestPublisher pub;
    TestSubscriber sub;
    //TypeInformation* type_info = nullptr; // Not using it

    DataRepresentationQosPolicy dataRepQos;
    dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
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

    pub.init("TypeInformationNamesIgnored", 10, &type1, nullptr, nullptr, type_info1, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("TypeInformationNamesIgnored", 10, &type2, nullptr, nullptr, type_info2, "Sub1", &dataRepQos, &typeConQos);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/**** TODO - Mixing TypeObject, TypeInformation and TypeIdentifier ****/

/**** DataRepresentation Compatibility tests ****/

/*
 * Empty-Empty
*/
TEST_F(XTypes, DataRepQoSEE)
{
    BasicStructPubSubType type;
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos;
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoSEE", 10, &type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoSEE", 10, &type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * Empty-XCDR1
*/
TEST_F(XTypes, DataRepQoSE1)
{
    BasicStructPubSubType type;
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoSE1", 10, &type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos1);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoSE1", 10, &type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos2, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * Empty-XCDR2
*/
TEST_F(XTypes, DataRepQoSE2)
{
    BasicStructPubSubType type;
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoSE2", 10, &type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos1);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoSE2", 10, &type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos2, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * Empty-XML
*/
TEST_F(XTypes, DataRepQoSEX)
{
    BasicStructPubSubType type;
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoSEX", 10, &type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos1);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoSEX", 10, &type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos2, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * Empty-XCDR12
*/
TEST_F(XTypes, DataRepQoSE12)
{
    BasicStructPubSubType type;
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoSE12", 10, &type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos1);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoSE12", 10, &type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos2, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * XCDR1-XCDR2
*/
TEST_F(XTypes, DataRepQoS12)
{
    BasicStructPubSubType type;
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoS12", 10, &type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos1);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoS12", 10, &type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos2, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * XCDR2-XCDR1
*/
TEST_F(XTypes, DataRepQoS21)
{
    BasicStructPubSubType type;
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoS21", 10, &type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos2);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoS21", 10, &type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos1, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

/*
 * XCDR2-XCDR2
*/
TEST_F(XTypes, DataRepQoS22)
{
    BasicStructPubSubType type;
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos2;
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoS22", 10, &type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos2);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoS22", 10, &type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos2, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(true, 3);
    pub.waitDiscovery(true, 3);
}

/*
 * XML-Empty
*/
TEST_F(XTypes, DataRepQoSXE)
{
    BasicStructPubSubType type;
    TestPublisher pub;
    TestSubscriber sub;

    DataRepresentationQosPolicy dataRepQos1;
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos1.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    DataRepresentationQosPolicy dataRepQos2;
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR_DATA_REPRESENTATION);
    dataRepQos2.m_value.push_back(DataRepresentationId_t::XML_DATA_REPRESENTATION);
    //dataRepQos2.m_value.push_back(DataRepresentationId_t::XCDR2_DATA_REPRESENTATION);

    pub.init("DataRepQoSXE", 10, &type, nullptr, nullptr, nullptr, "Pub1", &dataRepQos2);
    ASSERT_TRUE(pub.isInitialized());

    sub.init("DataRepQoSXE", 10, &type, nullptr, nullptr, nullptr, "Sub1", &dataRepQos1, nullptr);
    ASSERT_TRUE(sub.isInitialized());

    // Wait for discovery.
    sub.waitDiscovery(false, 3);
    pub.waitDiscovery(false, 3);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
