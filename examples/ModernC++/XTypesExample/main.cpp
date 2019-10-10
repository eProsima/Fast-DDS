#include <dds/core/xtypes/xtypes.hpp>

#include <iostream>

using namespace dds::core::xtypes;

void print_data(const ReadableDynamicDataRef& data)
{
    data.for_each([&](const DynamicData::ReadableNode& o)
    {
        std::cout << std::string(o.deep() * 4, ' ');
        if(o.has_parent())
        {
            std::cout << "["
                << (o.parent().type().is_aggregation_type()
                    ? o.access().struct_member().name()
                    : std::to_string(o.access().index()))
                << "] ";
        }
        switch(o.data().type().kind())
        {
            case TypeKind::UINT_32_TYPE:
                std::cout << "<" << o.data().type().name() << ">    " << o.data().value<uint32_t>();
                break;
            case TypeKind::FLOAT_32_TYPE:
                std::cout << "<" << o.data().type().name() << ">       " << o.data().value<float>();
                break;
            case TypeKind::FLOAT_64_TYPE:
                std::cout << "<" << o.data().type().name() << ">     " << o.data().value<double>();
                break;
            case TypeKind::STRING_TYPE:
                std::cout << "<" << o.data().type().name() << ">      " << o.data().value<std::string>();
                break;
            case TypeKind::ARRAY_TYPE:
                std::cout << "<" << o.data().type().name() << ">";
                break;
            case TypeKind::SEQUENCE_TYPE:
                std::cout << "<" << o.data().type().name() << "[" << o.data().size() << "]>";
                break;
            case TypeKind::STRUCTURE_TYPE:
                std::cout << "Structure: <" << o.data().type().name() << ">";
                break;
            default:
                std::cout << "Unexpected type: " << o.data().type().name();
        }
        std::cout << std::endl;
    });
}

int main()
{
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
    data["om3"].push(50);
    data["om3"][1].value(100);
    data["om4"].push(data["om2"]);
    data["om4"].push(data["om2"]);
    data["om4"][1] = data["om2"];
    data["om5"][1].value(123);
    data["om6"][1] = data["om2"];
    data["om7"].value<std::string>("Hi!");
    data["om7"].string("This is a string!");

    print_data(data);

    return 0;
}
