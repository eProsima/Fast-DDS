#include <nlohmann/json.hpp>

// #include <fastrtps/types/AnnotationDescriptor.h>
#include <fastdds/dds/xtypes/dynamic_types/AnnotationDescriptor.hpp>
// #include <fastrtps/types/TypeDescriptor.h>
#include <fastdds/dds/xtypes/dynamic_types/TypeDescriptor.hpp>
// #include <fastrtps/types/DynamicType.h>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
// #include <fastrtps/types/DynamicData.h>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
// #include <fastrtps/types/DynamicTypeMember.h>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeMember.hpp>
// #include <fastdds/dds/xtypes/type_representation/detail/dds_xtypes_typeobject.hpp>
#include <fastdds/dds/xtypes/dynamic_types/detail/dynamic_language_binding.hpp>

class DynamicDataHelper
{
public:

    static void print(
            const eprosima::fastdds::dds::DynamicData::_ref_type& data);

    static void print_json(
            const eprosima::fastdds::dds::DynamicData::_ref_type& data);

    static void print_json_stream(
            const eprosima::fastdds::dds::DynamicData::_ref_type& data,
            std::ostream& output);

    static std::ostream& print(
            std::ostream& output,
            const eprosima::fastdds::dds::DynamicData::_ref_type& data);

protected:

    static void print_member(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            const eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicTypeMember>::ref_type& type,
            const std::string& tabs = "");

    static void print_member_alias_aux(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            const eprosima::fastdds::dds::MemberId& id,
            const eprosima::fastdds::dds::TypeKind& kind,
            const std::string& tabs = "",
            eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicType>::ref_type enclosing_type=nullptr);

    static void print_member_json(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            const eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicTypeMember>::ref_type& type,
            nlohmann::json& j);

    static void print_member(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            std::ostream& output,
            const eprosima::fastdds::dds::traits<eprosima::fastdds::dds::DynamicTypeMember>::ref_type& type,
            const std::string& tabs = "");

    static void print_basic_element(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            eprosima::fastdds::dds::MemberId id,
            eprosima::fastdds::dds::TypeKind kind);

    static void print_basic_element_json(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            eprosima::fastdds::dds::MemberId id,
            eprosima::fastdds::dds::TypeKind kind,
            const std::string& member_name,
            nlohmann::json& j);

    static void print_basic_element_json(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            eprosima::fastdds::dds::MemberId id,
            eprosima::fastdds::dds::TypeKind kind,
            nlohmann::json& j);

    static void print_basic_element(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            eprosima::fastdds::dds::MemberId id,
            eprosima::fastdds::dds::TypeKind kind,
            std::ostream& output);

    static void print_collection(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            const std::string& tabs = "");

    static void print_collection_json(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            const std::string& member_name,
            nlohmann::json& j);

    static void print_collection_json(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            nlohmann::json& j);

    static void print_collection(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            std::ostream& output,
            const std::string& tabs = "");

    static void fill_array_positions(
            const std::vector<uint32_t>& bounds,
            std::vector<std::vector<uint32_t>>& positions);

    static void get_index_position(
            uint32_t index,
            const std::vector<uint32_t>& bounds,
            std::vector<uint32_t>& position);

    static void aux_index_position(
            uint32_t index,
            uint32_t inner_index,
            const std::vector<uint32_t>& bounds,
            std::vector<uint32_t>& position);

    static void print_basic_collection(
            eprosima::fastdds::dds::DynamicData::_ref_type data);

    static void print_basic_collection_json(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            const std::string& member_name,
            nlohmann::json& j);

    static void print_basic_collection_json(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            nlohmann::json& j);

    static void print_basic_collection(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            std::ostream& output);

    static void print_complex_collection(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            const std::string& tabs = "");

    static void print_complex_collection_json(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            const std::string& member_name,
            nlohmann::json& j);

    static void print_complex_collection_json(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            nlohmann::json& j);

    static void print_complex_collection(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            std::ostream& output,
            const std::string& tabs = "");

    static void print_complex_element(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            eprosima::fastdds::dds::MemberId id,
            const std::string& tabs = "");

    static void print_complex_element_json(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            eprosima::fastdds::dds::MemberId id,
            const std::string& member_name,
            nlohmann::json& j);

    static void print_complex_element_json(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            eprosima::fastdds::dds::MemberId id,
            nlohmann::json& j);

    static void print_complex_element(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            eprosima::fastdds::dds::MemberId id,
            std::ostream& output,
            const std::string& tabs = "");

    static void print_element(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            eprosima::fastdds::dds::MemberId id,
            eprosima::fastdds::dds::DynamicType::_ref_type type,
            const std::string& tabs = "");

    static void print_element_json(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            eprosima::fastdds::dds::MemberId id,
            eprosima::fastdds::dds::DynamicType::_ref_type type,
            const std::string& member_name,
            nlohmann::json& j);

    static void print_element(
            eprosima::fastdds::dds::DynamicData::_ref_type data,
            eprosima::fastdds::dds::MemberId id,
            eprosima::fastdds::dds::DynamicType::_ref_type type,
            std::ostream& output,
            const std::string& tabs = "");
};
