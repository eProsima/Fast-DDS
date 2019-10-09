#include <dds/core/xtypes/xtypes.hpp>

#include <iostream>

using namespace dds::core::xtypes;

int main()
{
    // XTYPES API
    StructType inner("InnerType");
    inner.add_member(Member("im1", primitive_type<uint32_t>()));
    inner.add_member(Member("im2", primitive_type<float>()));

    StructType outter("OutterType");
    outter.add_member(Member("om1", primitive_type<double>()));
    outter.add_member(Member("om2", inner));
    outter.add_member(Member("om3", SequenceType(primitive_type<uint32_t>(), 5)));
    outter.add_member(Member("om4", SequenceType(inner)));
    outter.add_member(Member("om5", ArrayType(primitive_type<uint32_t>(), 4)));
    outter.add_member(Member("om6", ArrayType(inner, 4)));
    outter.add_member(Member("om7", StringType()));

    DynamicData data(outter);
    data["om1"].value(6.7);
    data["om2"]["im1"].value(42);
    data["om2"]["im2"].value(35.8f);
    data["om3"].push(12);
    data["om3"].push(31);
    data["om3"].push(50); // direct push to the internal std::vector
    data["om3"][1].value(100); //direct index access to the std::vector
    data["om4"].push(data["om2"]);
    data["om4"].push(data["om2"]);
    data["om4"][1] = data["om2"];
    data["om5"][1].value(123);
    data["om6"][1] = data["om2"];
    data["om7"].value<std::string>("Hi!"); //Check small string
    data["om7"].string("This is a string!"); //Check small string

    size_t deep = 0;
    data.for_each([&](ReadableDynamicDataRef data, size_t deep)
    {
        std::string tabs(deep * 4, ' ');
        switch(data.type().kind())
        {
            case TypeKind::UINT_32_TYPE:
                std::cout << tabs << "Type: " << data.type().name() << ", value: " << data.value<uint32_t>() << std::endl;
                break;
            case TypeKind::FLOAT_32_TYPE:
                std::cout << tabs << "Type: " << data.type().name() << ", value: " << data.value<float>() << std::endl;
                break;
            case TypeKind::FLOAT_64_TYPE:
                std::cout << tabs << "Type: " << data.type().name() << ", value: " << data.value<double>() << std::endl;
                break;
            case TypeKind::STRING_TYPE:
                std::cout << tabs << "Type: " << data.type().name() << ", value: " << data.value<std::string>() << std::endl;
                break;
            case TypeKind::ARRAY_TYPE:
                std::cout << tabs << "Type: " << data.type().name() << ", size: " << data.size() << std::endl;
                deep++;
                break;
            case TypeKind::SEQUENCE_TYPE:
                std::cout << tabs << "Type: " << data.type().name() << ", size: " << data.size() << std::endl;
                deep++;
                break;
            case TypeKind::STRUCTURE_TYPE:
                std::cout << tabs << "Type: Structure, name: " << data.type().name() << std::endl;
                deep++;
                break;
            default:
                std::cout << tabs << "Unexpected type: " << data.type().name() << std::endl;
        }
    });

    std::cout << "  outter is_subset_of outter: " << outter.is_subset_of(outter) << std::endl;
    std::cout << "  outter is_subset_of inner: " << outter.is_subset_of(inner) << std::endl;
    std::cout << "  data == data: " << (data == data) << std::endl;

    return 0;
}
