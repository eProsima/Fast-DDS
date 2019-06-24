#include <fastrtps/types/TypeNamesGenerator.h>
#include <fastrtps/types/TypeObjectFactory.h>

#include <sstream>

namespace eprosima {
namespace fastrtps {
namespace types {


std::string TypeNamesGenerator::get_string_type_name(
        uint32_t bound,
        bool wide,
        bool generate_identifier)
{
    std::stringstream type;
    type << ((wide) ? "wstring" : "string");
    type << ((bound < 256) ? "s_" : "l_") << bound;
    if (generate_identifier)
    {
        TypeObjectFactory::get_instance()->get_string_identifier(bound, wide);
    }
    return type.str();
}

std::string TypeNamesGenerator::get_sequence_type_name(
        const std::string& type_name,
        uint32_t bound,
        bool generate_identifier)
{
    std::stringstream auxType;
    auxType << ((bound < 256) ? "sequences_" : "sequencel_");
    auxType << type_name << "_" << bound;
    if (generate_identifier)
    {
        TypeObjectFactory::get_instance()->get_sequence_identifier(type_name, bound, true);
    }
    return auxType.str();
}

std::string TypeNamesGenerator::get_array_type_name(
        const std::string& type_name,
        const std::vector<uint32_t>& bound,
        bool generate_identifier)
{
    uint32_t unused;
    return get_array_type_name(type_name, bound, unused, generate_identifier);
}

std::string TypeNamesGenerator::get_array_type_name(
        const std::string& type_name,
        const std::vector<uint32_t>& bound,
        uint32_t& ret_size,
        bool generate_identifier)
{
    std::stringstream auxType;
    std::stringstream auxType2;
    auxType2 << type_name;
    uint32_t size = 0;
    for (uint32_t b : bound)
    {
        auxType2 << "_" << b;
        size += b;
    }
    if (size < 256)
    {
        auxType << "arrays_";
    }
    else
    {
        auxType << "arrayl_";
    }
    auxType << auxType2.str();
    ret_size = size;
    if (generate_identifier)
    {
        TypeObjectFactory::get_instance()->get_array_identifier(type_name, bound, true);
    }
    return auxType.str();
}

std::string TypeNamesGenerator::get_map_type_name(
        const std::string& key_type_name,
        const std::string& value_type_name,
        uint32_t bound,
        bool generate_identifier)
{
    std::stringstream auxType;
    auxType << ((bound < 256) ? "maps_" : "mapl_");
    auxType << key_type_name << "_" << value_type_name << "_" << bound;
    if (generate_identifier)
    {
        TypeObjectFactory::get_instance()->get_map_identifier(key_type_name, value_type_name, bound, true);
    }
    return auxType.str();
}


} // namespace types
} // namespace fastrtps
} // namespace eprosima
