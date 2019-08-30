#ifndef _TYPE_NAMES_GENERATOR_
#define _TYPE_NAMES_GENERATOR_

#include <fastrtps/fastrtps_dll.h>

#include <string>
#include <vector>

namespace eprosima {
namespace fastrtps {
namespace types {

class TypeNamesGenerator
{
public:
    RTPS_DllAPI static std::string get_string_type_name(
            uint32_t bound,
            bool wide,
            bool generate_identifier = true);

    RTPS_DllAPI static std::string get_sequence_type_name(
            const std::string& type_name,
            uint32_t bound,
            bool generate_identifier = true);

    RTPS_DllAPI static std::string get_array_type_name(
            const std::string& type_name,
            const std::vector<uint32_t>& bound,
            bool generate_identifier = true);

    RTPS_DllAPI static std::string get_array_type_name(
            const std::string& type_name,
            const std::vector<uint32_t>& bound,
            uint32_t& ret_size,
            bool generate_identifier = true);

    RTPS_DllAPI static std::string get_map_type_name(
            const std::string& key_type_name,
            const std::string& value_type_name,
            uint32_t bound,
            bool generate_identifier = true);
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif //_TYPE_NAMES_GENERATOR_
