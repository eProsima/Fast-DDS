// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file TypeLookupServicePublisher.h
 *
 */

#ifndef _TEST_DDS_XTYPES_TYPELOOKUPSERVICETEST_PUBLISHER_H_
#define _TEST_DDS_XTYPES_TYPELOOKUPSERVICETEST_PUBLISHER_H_

#include <asio.hpp>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_set>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/publisher/PublisherListener.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.hpp>

#include "TypeLookupServiceTestsTypes.h"

namespace eprosima {
namespace fastdds {
namespace dds {

struct PubKnownType
{
    std::shared_ptr<void> type_;
    DynamicType::_ref_type dyn_type_;
    TypeSupport type_sup_;

    DataWriter* writer_ = nullptr;
};

// Define a macro to simplify type registration
    #define PUBLISHER_TYPE_CREATOR_FUNCTION(Type) \
    type_creator_functions_[#Type] = std::bind(&TypeLookupServicePublisher::create_known_type_impl<Type, \
                    Type ## PubSubType>, \
                    this, \
                    std::placeholders::_1)


class TypeLookupServicePublisher
    : public DomainParticipantListener
{
public:

    TypeLookupServicePublisher()
    {
    }

    ~TypeLookupServicePublisher();

    bool init(
            uint32_t domain_id,
            std::vector<std::string> known_types,
            uint32_t builtin_flow_controller_bytes);

    bool wait_discovery(
            uint32_t expected_matches,
            uint32_t timeout);

    bool run(
            uint32_t samples,
            uint32_t timeout);

    void on_publication_matched(
            DataWriter* /*writer*/,
            const PublicationMatchedStatus& info) override;

    void on_data_reader_discovery(
            eprosima::fastdds::dds::DomainParticipant* /*participant*/,
            eprosima::fastdds::rtps::ReaderDiscoveryStatus reason,
            const eprosima::fastdds::dds::SubscriptionBuiltinTopicData& info,
            bool& should_be_ignored) override;

private:

    bool setup_publisher(
            PubKnownType& new_type);

    bool create_known_type(
            const std::string& type);

    template <typename Type, typename TypePubSubType>
    bool create_known_type_impl(
            const std::string& type);

    bool create_discovered_type(
            const eprosima::fastdds::dds::SubscriptionBuiltinTopicData& info);

    bool check_registered_type(
            const xtypes::TypeInformationParameter& type_info);

    DomainParticipant* participant_ = nullptr;

    std::mutex mutex_;
    std::condition_variable cv_;
    int32_t matched_ {0};
    uint32_t expected_matches_ {0};
    std::map<eprosima::fastdds::rtps::GUID_t, uint32_t> sent_samples_;

    std::mutex known_types_mutex_;
    std::map<std::string, PubKnownType> known_types_;
    std::unordered_set<std::string> types_without_typeobject_;
    std::map<std::string, std::function<bool(const std::string&)>> type_creator_functions_;
    std::vector<std::thread> create_types_threads;
    uint32_t domain_id_ {0};

    /**
     * This method is updated automatically using the update_headers_and_create_cases.py script
     */
    void create_type_creator_functions()
    {
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type1);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type2);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type3);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type10);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type100);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type11);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type12);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type13);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type14);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type15);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type16);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type17);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type18);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type19);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type20);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type21);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type22);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type23);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type24);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type25);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type26);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type27);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type28);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type29);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type30);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type31);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type32);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type33);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type34);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type35);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type36);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type37);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type38);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type39);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type4);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type40);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type41);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type42);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type43);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type44);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type45);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type46);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type47);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type48);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type49);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type5);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type50);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type51);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type52);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type53);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type54);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type55);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type56);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type57);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type58);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type59);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type6);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type60);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type61);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type62);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type63);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type64);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type65);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type66);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type67);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type68);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type69);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type7);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type70);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type71);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type72);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type73);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type74);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type75);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type76);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type77);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type78);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type79);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type8);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type80);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type81);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type82);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type83);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type84);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type85);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type86);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type87);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type88);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type89);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type9);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type90);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type91);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type92);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type93);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type94);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type95);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type96);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type97);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type98);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Type99);
        PUBLISHER_TYPE_CREATOR_FUNCTION(TypeBig);
        PUBLISHER_TYPE_CREATOR_FUNCTION(TypeDep);
        PUBLISHER_TYPE_CREATOR_FUNCTION(TypeNoTypeObject);
        types_without_typeobject_.insert("TypeNoTypeObject");
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasAlias);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasArray);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasBitmask);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasBitset);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasBool);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasChar16);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasChar8);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasEnum);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasFloat128);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasFloat32);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasFloat64);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasInt16);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasInt32);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasInt64);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasMap);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasMultiArray);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasSequence);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasString16);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasString8);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasUInt32);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasUInt64);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasUint16);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AliasUnion);
        PUBLISHER_TYPE_CREATOR_FUNCTION(BasicAnnotationsStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(EmptyAnnotatedStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableBooleanStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableCharStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableDoubleStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableEmptyInheritanceStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableEmptyStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableExtensibilityInheritance);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableFloatStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableInheritanceEmptyStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableInheritanceStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableLongDoubleStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableLongLongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableLongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableOctetStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableShortStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableULongLongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableULongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableUShortStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableUnionStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(AppendableWCharStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayAlias);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayBitMask);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayBitset);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayBoundedString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayBoundedWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayEnum);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayFloat);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayLongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMap);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionAlias);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionBitMask);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionBitset);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionBoundedString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionBoundedWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionEnum);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionFloat);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsAlias);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsBitMask);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsBitSet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsBoundedString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsBoundedWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsEnum);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsFloat);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsLongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsMap);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsSequence);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsStructure);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsULong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsULongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsUShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsUnion);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsWChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionMap);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionSequence);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionStructure);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionULong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionULongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionUShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionUnion);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionWChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySequence);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayShortArray);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsAlias);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsBitMask);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsBitset);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsBoundedString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsBoundedWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsEnum);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsFloat);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsLongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsMap);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsSequence);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsShortArray);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsStructure);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsUnion);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsUnsignedLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsUnsignedLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsUnsignedShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsWChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayStructure);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayUInt8);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayULong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayULongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayUShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayUnion);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayWChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ArrayWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(BoundedBigArrays);
        PUBLISHER_TYPE_CREATOR_FUNCTION(BoundedSmallArrays);
        PUBLISHER_TYPE_CREATOR_FUNCTION(BitsetStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ConstsLiteralsStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(const_module1::ModuleConstsLiteralsStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(const_module2::Module2ConstsLiteralsStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ForwardDeclarationsRecursiveStruct);
        types_without_typeobject_.insert("ForwardDeclarationsRecursiveStruct");
        PUBLISHER_TYPE_CREATOR_FUNCTION(ForwardStruct);
        types_without_typeobject_.insert("ForwardStruct");
        PUBLISHER_TYPE_CREATOR_FUNCTION(ModuledCommonNameStructure);
        types_without_typeobject_.insert("ModuledCommonNameStructure");
        PUBLISHER_TYPE_CREATOR_FUNCTION(ModuledForwardDeclarationsRecursiveStruct);
        types_without_typeobject_.insert("ModuledForwardDeclarationsRecursiveStruct");
        PUBLISHER_TYPE_CREATOR_FUNCTION(declarations_module::ModuledForwardStruct);
        types_without_typeobject_.insert("declarations_module__ModuledForwardStruct");
        PUBLISHER_TYPE_CREATOR_FUNCTION(BitMaskStructure);
        PUBLISHER_TYPE_CREATOR_FUNCTION(BoundedBitMaskStructure);
        PUBLISHER_TYPE_CREATOR_FUNCTION(EnumStructure);
        PUBLISHER_TYPE_CREATOR_FUNCTION(EnumWithValuesStructure);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalBooleanStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalCharStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalDoubleStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalEmptyInheritanceStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalEmptyStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalExtensibilityInheritance);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalFloatStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalInheritanceStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalLongDoubleStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalLongLongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalLongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalOctetStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalShortStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalULongLongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalULongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalUShortStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalUnionStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FinalWCharStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(InheritanceEmptyStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(InnerEmptyStructureHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(InnerStructureHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(BitsetsChildInheritanceStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(InnerEmptyStructureHelperChild);
        PUBLISHER_TYPE_CREATOR_FUNCTION(InnerStructureHelperChild);
        PUBLISHER_TYPE_CREATOR_FUNCTION(InnerStructureHelperChildChild);
        PUBLISHER_TYPE_CREATOR_FUNCTION(InnerStructureHelperEmptyChild);
        PUBLISHER_TYPE_CREATOR_FUNCTION(InnerStructureHelperEmptyChildChild);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructAliasInheritanceStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructuresInheritanceStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(InheritanceKeyedEmptyStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedAppendable);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedBooleanStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedCharStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedDoubleStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedEmptyInheritanceStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedEmptyStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedFinal);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedFloatStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedInheritanceStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedLongDoubleStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedLongLongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedLongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedMutable);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedOctetStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedShortStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedULongLongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedULongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedUShortStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(KeyedWCharStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(BoundedLargeMap);
        PUBLISHER_TYPE_CREATOR_FUNCTION(BoundedSmallMap);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperFloat);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerAliasArrayHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerAliasBoundedStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerAliasBoundedWStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerAliasHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerAliasMapHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerAliasSequenceHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerBitMaskHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerBitsetHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerEnumHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerStructureHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerUnionHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperLongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperULong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperULongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperUShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperWChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapKeyULongLongValueDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapKeyULongValueLongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapKeyULongValueLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongFloat);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongInnerAliasArrayHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongInnerAliasBoundedStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongInnerAliasBoundedWStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongInnerAliasHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongInnerAliasMapHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongInnerAliasSequenceHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongInnerBitMaskHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongInnerBitsetHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongInnerEnumHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongInnerStructureHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongInnerUnionHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongKeyLongDoubleValue);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongKeyLongLongValue);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongFloat);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongInnerAliasArrayHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongInnerAliasBoundedStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongInnerAliasBoundedWStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongInnerAliasHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongInnerAliasMapHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongInnerAliasSequenceHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongInnerBitMaskHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongInnerBitsetHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongInnerEnumHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongInnerStructureHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongInnerUnionHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongKeyDoubleValue);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongKeyLongValue);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongLongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongULong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongULongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongUShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongWChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongLongWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongULong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongULongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongUShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongWChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapLongWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortFloat);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortInnerAliasArrayHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortInnerAliasBoundedStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortInnerAliasBoundedWStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortInnerAliasHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortInnerAliasMapHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortInnerAliasSequenceHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortInnerBitMaskHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortInnerBitsetHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortInnerEnumHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortInnerStructureHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortInnerUnionHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortLongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortULong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortULongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortUShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortWChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapShortWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringFloat);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringInnerAliasArrayHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringInnerAliasBoundedStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringInnerAliasBoundedWStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringInnerAliasHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringInnerAliasMapHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringInnerAliasSequenceHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringInnerBitMaskHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringInnerBitsetHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringInnerEnumHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringInnerStructureHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringInnerUnionHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringLongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringULong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringULongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringUShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringWChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapStringWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongFloat);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongInnerAliasArrayHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongInnerAliasBoundedStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongInnerAliasBoundedWStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongInnerAliasHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongInnerAliasMapHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongInnerAliasSequenceHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongInnerBitMaskHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongInnerBitsetHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongInnerEnumHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongInnerStructureHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongInnerUnionHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongFloat);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongInnerAliasArrayHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongInnerAliasBoundedStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongInnerAliasBoundedWStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongInnerAliasHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongInnerAliasMapHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongInnerAliasSequenceHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongInnerBitMaskHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongInnerBitsetHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongInnerEnumHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongInnerStructureHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongInnerUnionHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongLongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongULong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongULongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongUShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongWChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongLongWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongULong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongULongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongUShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongWChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapULongWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortFloat);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortInnerAliasArrayHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortInnerAliasBoundedStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortInnerAliasBoundedWStringHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortInnerAliasHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortInnerAliasMapHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortInnerAliasSequenceHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortInnerBitMaskHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortInnerBitsetHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortInnerEnumHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortInnerStructureHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortInnerUnionHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortLongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortULong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortULongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortUShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortWChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MapUShortWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableBooleanStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableCharStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableDoubleStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableEmptyInheritanceStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableEmptyStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableExtensibilityInheritance);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableFloatStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableInheritanceEmptyStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableInheritanceStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableLongDoubleStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableLongLongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableLongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableOctetStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableShortStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableULongLongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableULongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableUShortStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableUnionStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(MutableWCharStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(InnerStructOptional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(array_short_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(array_short_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(array_short_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(array_short_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(boolean_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(boolean_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(boolean_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(boolean_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(char_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(char_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(char_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(char_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(double_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(double_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(double_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(double_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(float_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(float_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(float_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(float_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(long_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(long_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(long_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(long_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(longdouble_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(longdouble_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(longdouble_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(longdouble_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(longlong_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(longlong_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(longlong_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(longlong_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(map_short_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(map_short_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(map_short_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(map_short_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(octet_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(octet_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(octet_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(octet_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(opt_struct_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(opt_struct_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(opt_struct_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(opt_struct_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(sequence_short_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(sequence_short_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(sequence_short_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(sequence_short_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(short_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(short_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(short_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(short_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(string_bounded_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(string_bounded_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(string_bounded_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(string_bounded_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(string_unbounded_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(string_unbounded_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(string_unbounded_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(string_unbounded_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(struct_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(struct_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(struct_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(struct_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ulong_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ulong_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ulong_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ulong_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ulonglong_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ulonglong_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ulonglong_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ulonglong_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ushort_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ushort_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ushort_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ushort_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(wchar_align_1_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(wchar_align_2_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(wchar_align_4_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(wchar_optional);
        PUBLISHER_TYPE_CREATOR_FUNCTION(BooleanStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(CharStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(DoubleStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(FloatStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Int16Struct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Int32Struct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Int64Struct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Int8Struct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(LongDoubleStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(LongLongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(LongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(OctetStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ShortStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ULongLongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(ULongStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UShortStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Uint16Struct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Uint32Struct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Uint64Struct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Uint8Struct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(WCharStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(BoundedBigSequences);
        PUBLISHER_TYPE_CREATOR_FUNCTION(BoundedSmallSequences);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceAlias);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceBitMask);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceBitset);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceEnum);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceFloat);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceLongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceMap);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceSequence);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceShortArray);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceStringBounded);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceStructure);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceULong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceULongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceUShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceUnion);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceWChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SequenceWStringBounded);
        PUBLISHER_TYPE_CREATOR_FUNCTION(NoCommon_Module::My_Structure);
        PUBLISHER_TYPE_CREATOR_FUNCTION(LargeStringStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(LargeWStringStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SmallStringStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(SmallWStringStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StringStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(WStringStruct);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructAlias);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructBitMask);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructBitset);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructBoundedString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructBoundedWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructChar16);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructChar8);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructEmpty);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructEnum);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructFloat);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructLongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructMap);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructSequence);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructShortArray);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructStructure);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructUnion);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructUnsignedLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructUnsignedLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructUnsignedShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(StructWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(Structures);
        PUBLISHER_TYPE_CREATOR_FUNCTION(bar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(root);
        PUBLISHER_TYPE_CREATOR_FUNCTION(root1);
        PUBLISHER_TYPE_CREATOR_FUNCTION(root2);
        PUBLISHER_TYPE_CREATOR_FUNCTION(testing_1::foo);
        PUBLISHER_TYPE_CREATOR_FUNCTION(testing_2::foo);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionArray);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionBoundedString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionBoundedWString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorAlias);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorBoolean);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorEnum);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorEnumLabel);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorULong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorULongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorUShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorWChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionFixedStringAlias);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionFloat);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionInnerAliasHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionInnerBitMaskHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionInnerBitsetHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionInnerEnumHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionInnerStructureHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionInnerUnionHelper);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionLongDouble);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionLongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionMap);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionOctet);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionSequence);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionSeveralFields);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionSeveralFieldsWithDefault);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionShortExtraMember);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionString);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionULong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionULongLong);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionUShort);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionWChar);
        PUBLISHER_TYPE_CREATOR_FUNCTION(UnionWString);
    }

};

} // dds
} // fastdds
} // eprosima

#endif /* _TEST_DDS_XTYPES_TYPELOOKUPSERVICETEST_PUBLISHER_H_ */
