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
    outter.add_member(Member("om5", ArrayType(primitive_type<uint32_t>(), 10)));
    outter.add_member(Member("om6", ArrayType(inner, 10)));

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


    // DYNAMIC TYPE INFO
    std::cout << "outter name: " << outter.name() << std::endl;
    std::cout << "outter kind: " << (outter.kind() == TypeKind::STRUCTURE_TYPE) << std::endl;
    std::cout << "  om1 name: " << outter.member("om1").name() << std::endl;
    std::cout << "  om1 type kind: " << (outter.member("om1").type().kind() == TypeKind::FLOAT_64_TYPE) << std::endl;
    std::cout << "  om2 name: " << outter.member("om2").name() << std::endl;
    std::cout << "  om2 type kind: " << (outter.member("om2").type().kind() == TypeKind::STRUCTURE_TYPE) << std::endl;
    std::cout << "  om2 im1 name: " << static_cast<const StructType&>(outter.member("om2").type()).member("im1").name() << std::endl;
    std::cout << "  om2 im2 name: " << static_cast<const StructType&>(outter.member("om2").type()).member("im2").name() << std::endl;
    std::cout << "  om3 name: " << outter.member("om3").name() << std::endl;
    std::cout << "  om3 type kind: " << (outter.member("om3").type().kind() == TypeKind::SEQUENCE_TYPE) << std::endl;

    //DYNAMIC DATA INFO
    std::cout << "outter values: " << std::endl;
    std::cout << "  om1: " << data["om1"].value<double>() << std::endl;
    std::cout << "  om2: " << data["om2"]["im1"].value<uint32_t>() << std::endl;
    std::cout << "  om2: " << data["om2"]["im2"].value<float>() << std::endl;
    std::cout << "  om3: " << data["om3"][0].value<uint32_t>() << std::endl;
    std::cout << "  om3: " << data["om3"][1].value<uint32_t>() << std::endl;
    std::cout << "  om3: " << data["om3"][2].value<uint32_t>() << std::endl;
    std::cout << "  om4: " << data["om4"][1]["im2"].value<float>() << std::endl;
    std::cout << "  om4: " << data["om5"][1].value<uint32_t>() << std::endl;
    std::cout << "  om4: " << data["om6"][1]["im1"].value<uint32_t>() << std::endl;

    return 0;
}
