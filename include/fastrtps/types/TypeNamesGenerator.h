#ifndef _TYPE_NAMES_GENERATOR_
#define _TYPE_NAMES_GENERATOR_

#include <string>
#include <vector>

namespace eprosima {
namespace fastrtps {
namespace types {

class TypeNamesGenerator
{
public:
    static std::string getStringTypeName(uint32_t bound, bool wide, bool generate_identifier = true);
    static std::string getSequenceTypeName(const std::string &type_name, uint32_t bound,
        bool generate_identifier = true);
    static std::string getArrayTypeName(const std::string &type_name, const std::vector<uint32_t> &bound,
        bool generate_identifier = true);
    static std::string getArrayTypeName(const std::string &type_name, const std::vector<uint32_t> &bound,
        uint32_t &ret_size, bool generate_identifier = true);
    static std::string getMapTypeName(const std::string &key_type_name, const std::string &value_type_name,
        uint32_t bound, bool generate_identifier = true);
};

} // namespace types
} // namespace fastrtps
} // namespace eprosima

#endif //_TYPE_NAMES_GENERATOR_