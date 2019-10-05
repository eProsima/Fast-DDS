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

    /*
    StructType outter("OutterType");
    outter.add_member(Member("om1", primitive_type<double>()));
    outter.add_member(Member("om2",
        StructType("InnerType")
        .add_member(Member("im1", primitive_type<uint32_t>()))
        .add_member(Member("im2", primitive_type<float>()))
        ));
    */

    DynamicData data(outter);
    data.value("om1", 6.7);
    data.loan_value("om2").value("im1", 42);
    data.loan_value("om2").value("im2", 35.8f);

    // DYNAMIC TYPE INFO
    std::cout << "outter name: " << outter.name() << std::endl;
    std::cout << "outter kind: " << (outter.kind() == TypeKind::STRUCTURE_TYPE) << std::endl;
    std::cout << "  om1 name: " << outter.member("om1").name() << std::endl;
    std::cout << "  om1 type name: " << outter.member("om1").type().name() << std::endl;
    std::cout << "  om1 type kind: " << (outter.member("om1").type().kind() == TypeKind::FLOAT_64_TYPE) << std::endl;
    std::cout << "  om2 name: " << outter.member("om2").name() << std::endl;
    std::cout << "  om2 type name: " << outter.member("om2").type().name() << std::endl;
    std::cout << "  om2 type kind: " << (outter.member("om2").type().kind() == TypeKind::STRUCTURE_TYPE) << std::endl;
    std::cout << "  om2 im1 name: " << static_cast<const StructType&>(outter.member("om2").type()).member("im1").name() << std::endl;
    std::cout << "  om2 im2 name: " << static_cast<const StructType&>(outter.member("om2").type()).member("im2").name() << std::endl;

    //DYNAMIC DATA INFO
    std::cout << "outter values: " << std::endl;
    std::cout << "  om1: " << data.value<double>("om1") << std::endl;
    std::cout << "  om2: " << data.loan_value("om2").value<uint32_t>("im1") << std::endl;
    std::cout << "  om2: " << data.loan_value("om2").value<float>("im2") << std::endl;

    return 0;
}
