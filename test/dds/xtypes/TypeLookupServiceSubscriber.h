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
 * @file TypeLookupServiceSubscriber.h
 *
 */

#ifndef _TEST_DDS_XTYPES_TYPELOOKUPSERVICETEST_SUBSCRIBER_H_
#define _TEST_DDS_XTYPES_TYPELOOKUPSERVICETEST_SUBSCRIBER_H_

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
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/SubscriberListener.hpp>
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

struct SubKnownType
{
    std::shared_ptr<void> type_;
    DynamicType::_ref_type dyn_type_;
    TypeSupport type_sup_;
};

// Define a macro to simplify type registration
#define SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type) \
    type_creator_functions_[#Type] = std::bind(&TypeLookupServiceSubscriber::create_known_type_impl<Type, \
                    Type ## PubSubType>, \
                    this, \
                    std::placeholders::_1); \
    type_processor_functions_[#Type] = std::bind(&TypeLookupServiceSubscriber::process_type_impl<Type>, \
                    this, \
                    std::placeholders::_1)

class TypeLookupServiceSubscriber
    : public DomainParticipantListener
{
public:

    TypeLookupServiceSubscriber()
    {
    }

    ~TypeLookupServiceSubscriber();

    bool init(
            uint32_t domain_id,
            std::vector<std::string> known_types,
            uint32_t builtin_flow_controller_bytes);

    bool wait_discovery(
            uint32_t expected_matches,
            uint32_t timeout);

    bool wait_participant_discovery(
            uint32_t expected_matches,
            uint32_t timeout);

    bool run(
            uint32_t samples,
            uint32_t timeout);

    void on_subscription_matched(
            DataReader* /*reader*/,
            const SubscriptionMatchedStatus& info) override;

    void on_data_available(
            DataReader* reader) override;

    void on_data_writer_discovery(
            eprosima::fastdds::dds::DomainParticipant* /*participant*/,
            eprosima::fastdds::rtps::WriterDiscoveryStatus reason,
            const eprosima::fastdds::dds::PublicationBuiltinTopicData& info,
            bool& should_be_ignored) override;

    void on_participant_discovery(
            DomainParticipant* participant,
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus status,
            const ParticipantBuiltinTopicData& info,
            bool& should_be_ignored) override;

private:

    bool setup_subscriber(
            SubKnownType& new_type);

    bool create_known_type(
            const std::string& type);

    template <typename Type, typename TypePubSubType>
    bool create_known_type_impl(
            const std::string& type);

    template <typename Type>
    bool process_type_impl(
            DataReader* reader);

    bool process_dyn_type_impl(
            DataReader* reader);

    bool create_discovered_type(
            const eprosima::fastdds::dds::PublicationBuiltinTopicData& info);

    bool check_registered_type(
            const xtypes::TypeInformationParameter& type_info);

    DomainParticipant* participant_ = nullptr;

    std::mutex mutex_;
    std::condition_variable cv_;
    int32_t matched_ {0};
    int32_t participant_matched_ {0};
    uint32_t expected_matches_ {0};
    std::map<eprosima::fastdds::rtps::GUID_t, uint32_t> received_samples_;

    std::mutex known_types_mutex_;
    std::map<std::string, SubKnownType> known_types_;
    std::unordered_set<std::string> types_without_typeobject_;
    std::map<std::string, std::function<bool(const std::string&)>> type_creator_functions_;
    std::map<std::string, std::function<bool(DataReader*)>> type_processor_functions_;
    std::vector<std::thread> create_types_threads;
    uint32_t domain_id_ {0};

    /**
     * This method is updated automatically using the update_headers_and_create_cases.py script
     */
    void create_type_creator_functions()
    {
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type1);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type2);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type3);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type10);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type100);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type11);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type12);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type13);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type14);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type15);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type16);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type17);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type18);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type19);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type20);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type21);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type22);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type23);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type24);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type25);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type26);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type27);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type28);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type29);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type30);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type31);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type32);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type33);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type34);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type35);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type36);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type37);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type38);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type39);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type4);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type40);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type41);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type42);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type43);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type44);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type45);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type46);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type47);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type48);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type49);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type5);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type50);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type51);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type52);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type53);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type54);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type55);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type56);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type57);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type58);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type59);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type6);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type60);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type61);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type62);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type63);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type64);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type65);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type66);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type67);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type68);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type69);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type7);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type70);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type71);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type72);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type73);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type74);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type75);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type76);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type77);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type78);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type79);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type8);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type80);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type81);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type82);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type83);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type84);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type85);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type86);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type87);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type88);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type89);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type9);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type90);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type91);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type92);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type93);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type94);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type95);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type96);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type97);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type98);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Type99);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(TypeBig);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(TypeDep);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(TypeNoTypeObject);
        types_without_typeobject_.insert("TypeNoTypeObject");
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasAlias);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasArray);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasBitmask);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasBitset);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasBool);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasChar16);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasChar8);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasEnum);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasFloat128);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasFloat32);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasFloat64);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasInt16);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasInt32);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasInt64);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasMap);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasMultiArray);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasSequence);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasString16);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasString8);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasUInt32);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasUInt64);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasUint16);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AliasUnion);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(BasicAnnotationsStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(EmptyAnnotatedStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableBooleanStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableCharStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableDoubleStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableEmptyInheritanceStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableEmptyStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableExtensibilityInheritance);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableFloatStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableInheritanceEmptyStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableInheritanceStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableLongDoubleStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableLongLongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableLongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableOctetStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableShortStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableULongLongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableULongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableUShortStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableUnionStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(AppendableWCharStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayAlias);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayBitMask);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayBitset);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayBoundedString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayBoundedWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayEnum);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayFloat);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayLongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMap);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionAlias);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionBitMask);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionBitset);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionBoundedString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionBoundedWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionEnum);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionFloat);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsAlias);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsBitMask);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsBitSet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsBoundedString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsBoundedWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsEnum);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsFloat);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsLongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsMap);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsSequence);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsStructure);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsULong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsULongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsUShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsUnion);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsWChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLiteralsWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionMap);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionSequence);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionStructure);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionULong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionULongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionUShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionUnion);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionWChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayMultiDimensionWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySequence);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayShortArray);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsAlias);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsBitMask);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsBitset);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsBoundedString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsBoundedWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsEnum);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsFloat);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsLongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsMap);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsSequence);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsShortArray);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsStructure);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsUnion);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsUnsignedLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsUnsignedLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsUnsignedShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsWChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArraySingleDimensionLiteralsWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayStructure);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayUInt8);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayULong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayULongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayUShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayUnion);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayWChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ArrayWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(BoundedBigArrays);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(BoundedSmallArrays);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(BitsetStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ConstsLiteralsStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(const_module1::ModuleConstsLiteralsStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(const_module2::Module2ConstsLiteralsStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ForwardDeclarationsRecursiveStruct);
        types_without_typeobject_.insert("ForwardDeclarationsRecursiveStruct");
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ForwardStruct);
        types_without_typeobject_.insert("ForwardStruct");
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ModuledCommonNameStructure);
        types_without_typeobject_.insert("ModuledCommonNameStructure");
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ModuledForwardDeclarationsRecursiveStruct);
        types_without_typeobject_.insert("ModuledForwardDeclarationsRecursiveStruct");
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(declarations_module::ModuledForwardStruct);
        types_without_typeobject_.insert("declarations_module__ModuledForwardStruct");
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(BitMaskStructure);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(BoundedBitMaskStructure);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(EnumStructure);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(EnumWithValuesStructure);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalBooleanStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalCharStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalDoubleStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalEmptyInheritanceStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalEmptyStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalExtensibilityInheritance);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalFloatStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalInheritanceStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalLongDoubleStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalLongLongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalLongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalOctetStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalShortStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalULongLongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalULongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalUShortStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalUnionStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FinalWCharStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(InheritanceEmptyStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(InnerEmptyStructureHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(InnerStructureHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(BitsetsChildInheritanceStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(InnerEmptyStructureHelperChild);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(InnerStructureHelperChild);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(InnerStructureHelperChildChild);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(InnerStructureHelperEmptyChild);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(InnerStructureHelperEmptyChildChild);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructAliasInheritanceStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructuresInheritanceStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(InheritanceKeyedEmptyStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedAppendable);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedBooleanStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedCharStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedDoubleStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedEmptyInheritanceStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedEmptyStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedFinal);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedFloatStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedInheritanceStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedLongDoubleStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedLongLongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedLongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedMutable);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedOctetStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedShortStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedULongLongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedULongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedUShortStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(KeyedWCharStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(BoundedLargeMap);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(BoundedSmallMap);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperFloat);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerAliasArrayHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerAliasBoundedStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerAliasBoundedWStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerAliasHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerAliasMapHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerAliasSequenceHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerBitMaskHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerBitsetHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerEnumHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerStructureHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperInnerUnionHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperLongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperULong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperULongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperUShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperWChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapInnerAliasBoundedStringHelperWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapKeyULongLongValueDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapKeyULongValueLongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapKeyULongValueLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongFloat);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongInnerAliasArrayHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongInnerAliasBoundedStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongInnerAliasBoundedWStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongInnerAliasHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongInnerAliasMapHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongInnerAliasSequenceHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongInnerBitMaskHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongInnerBitsetHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongInnerEnumHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongInnerStructureHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongInnerUnionHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongKeyLongDoubleValue);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongKeyLongLongValue);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongFloat);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongInnerAliasArrayHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongInnerAliasBoundedStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongInnerAliasBoundedWStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongInnerAliasHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongInnerAliasMapHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongInnerAliasSequenceHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongInnerBitMaskHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongInnerBitsetHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongInnerEnumHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongInnerStructureHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongInnerUnionHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongKeyDoubleValue);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongKeyLongValue);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongLongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongULong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongULongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongUShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongWChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongLongWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongULong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongULongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongUShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongWChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapLongWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortFloat);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortInnerAliasArrayHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortInnerAliasBoundedStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortInnerAliasBoundedWStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortInnerAliasHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortInnerAliasMapHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortInnerAliasSequenceHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortInnerBitMaskHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortInnerBitsetHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortInnerEnumHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortInnerStructureHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortInnerUnionHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortLongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortULong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortULongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortUShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortWChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapShortWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringFloat);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringInnerAliasArrayHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringInnerAliasBoundedStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringInnerAliasBoundedWStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringInnerAliasHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringInnerAliasMapHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringInnerAliasSequenceHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringInnerBitMaskHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringInnerBitsetHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringInnerEnumHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringInnerStructureHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringInnerUnionHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringLongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringULong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringULongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringUShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringWChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapStringWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongFloat);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongInnerAliasArrayHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongInnerAliasBoundedStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongInnerAliasBoundedWStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongInnerAliasHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongInnerAliasMapHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongInnerAliasSequenceHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongInnerBitMaskHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongInnerBitsetHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongInnerEnumHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongInnerStructureHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongInnerUnionHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongFloat);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongInnerAliasArrayHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongInnerAliasBoundedStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongInnerAliasBoundedWStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongInnerAliasHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongInnerAliasMapHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongInnerAliasSequenceHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongInnerBitMaskHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongInnerBitsetHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongInnerEnumHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongInnerStructureHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongInnerUnionHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongLongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongULong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongULongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongUShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongWChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongLongWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongULong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongULongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongUShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongWChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapULongWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortFloat);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortInnerAliasArrayHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortInnerAliasBoundedStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortInnerAliasBoundedWStringHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortInnerAliasHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortInnerAliasMapHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortInnerAliasSequenceHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortInnerBitMaskHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortInnerBitsetHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortInnerEnumHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortInnerStructureHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortInnerUnionHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortLongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortULong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortULongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortUShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortWChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MapUShortWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableBooleanStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableCharStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableDoubleStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableEmptyInheritanceStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableEmptyStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableExtensibilityInheritance);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableFloatStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableInheritanceEmptyStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableInheritanceStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableLongDoubleStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableLongLongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableLongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableOctetStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableShortStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableULongLongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableULongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableUShortStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableUnionStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(MutableWCharStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(InnerStructOptional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(array_short_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(array_short_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(array_short_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(array_short_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(boolean_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(boolean_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(boolean_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(boolean_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(char_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(char_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(char_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(char_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(double_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(double_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(double_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(double_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(float_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(float_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(float_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(float_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(long_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(long_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(long_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(long_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(longdouble_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(longdouble_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(longdouble_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(longdouble_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(longlong_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(longlong_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(longlong_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(longlong_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(map_short_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(map_short_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(map_short_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(map_short_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(octet_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(octet_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(octet_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(octet_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(opt_struct_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(opt_struct_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(opt_struct_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(opt_struct_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(sequence_short_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(sequence_short_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(sequence_short_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(sequence_short_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(short_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(short_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(short_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(short_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(string_bounded_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(string_bounded_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(string_bounded_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(string_bounded_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(string_unbounded_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(string_unbounded_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(string_unbounded_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(string_unbounded_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(struct_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(struct_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(struct_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(struct_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ulong_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ulong_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ulong_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ulong_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ulonglong_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ulonglong_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ulonglong_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ulonglong_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ushort_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ushort_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ushort_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ushort_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(wchar_align_1_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(wchar_align_2_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(wchar_align_4_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(wchar_optional);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(BooleanStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(CharStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(DoubleStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(FloatStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Int16Struct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Int32Struct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Int64Struct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Int8Struct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(LongDoubleStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(LongLongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(LongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(OctetStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ShortStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ULongLongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(ULongStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UShortStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Uint16Struct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Uint32Struct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Uint64Struct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Uint8Struct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(WCharStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(BoundedBigSequences);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(BoundedSmallSequences);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceAlias);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceBitMask);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceBitset);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceEnum);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceFloat);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceLongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceMap);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceSequence);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceShortArray);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceStringBounded);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceStructure);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceULong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceULongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceUShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceUnion);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceWChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SequenceWStringBounded);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(NoCommon_Module::My_Structure);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(LargeStringStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(LargeWStringStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SmallStringStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(SmallWStringStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StringStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(WStringStruct);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructAlias);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructBitMask);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructBitset);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructBoundedString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructBoundedWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructChar16);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructChar8);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructEmpty);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructEnum);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructFloat);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructLongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructMap);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructSequence);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructShortArray);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructStructure);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructUnion);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructUnsignedLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructUnsignedLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructUnsignedShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(StructWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(Structures);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(bar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(root);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(root1);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(root2);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(testing_1::foo);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(testing_2::foo);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionArray);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionBoundedString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionBoundedWString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorAlias);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorBoolean);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorEnum);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorEnumLabel);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorULong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorULongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorUShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionDiscriminatorWChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionFixedStringAlias);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionFloat);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionInnerAliasHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionInnerBitMaskHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionInnerBitsetHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionInnerEnumHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionInnerStructureHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionInnerUnionHelper);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionLongDouble);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionLongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionMap);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionOctet);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionSequence);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionSeveralFields);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionSeveralFieldsWithDefault);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionShortExtraMember);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionString);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionULong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionULongLong);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionUShort);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionWChar);
        SUBSCRIBER_TYPE_CREATOR_FUNCTION(UnionWString);
    }

};

} // dds
} // fastdds
} // eprosima


#endif /* _TEST_DDS_XTYPES_TYPELOOKUPSERVICETEST_SUBSCRIBER_H_ */
