#include <dds/core/xtypes/xtypes.hpp>

#include <iostream>

using namespace dds::core::xtypes;

int main()
{
    // XTYPES API
    StructType inner("InnerType");
    inner.add_member(Member("m1", primitive_type<uint32_t>()));
    inner.add_member(Member("m2", primitive_type<float>()));

    StructType outter("OutterType");
    outter.add_member(Member("m1", primitive_type<double>()));
    outter.add_member(Member("m2", inner));

    DynamicData data(outter);
    data.value("m1", 6.7);
    data.loan_value("m2").value("m1", 42);
    data.loan_value("m2").value("m2", 35.8f);

    // DYNAMIC TYPE INFO
    std::cout << "outter name: " << outter.name() << std::endl;
    std::cout << "outter kind: " << (outter.kind() == TypeKind::STRUCTURE_TYPE) << std::endl;
    std::cout << "  m1 name: " << outter.member("m1").name() << std::endl;
    std::cout << "  m1 type name: " << outter.member("m1").type().name() << std::endl;
    std::cout << "  m1 type kind: " << (outter.member("m1").type().kind() == TypeKind::FLOAT_64_TYPE) << std::endl;
    std::cout << "  m2 name: " << outter.member("m2").name() << std::endl;
    std::cout << "  m2 type name: " << outter.member("m2").type().name() << std::endl;
    std::cout << "  m2 type kind: " << (outter.member("m2").type().kind() == TypeKind::STRUCTURE_TYPE) << std::endl;
    std::cout << "  m2 m1 name: " << static_cast<const StructType&>(outter.member("m2").type()).member("m1").name() << std::endl;
    std::cout << "  m2 m2 name: " << static_cast<const StructType&>(outter.member("m2").type()).member("m2").name() << std::endl;

    //DYNAMIC DATA INFO
    std::cout << "outter values: " << std::endl;
    std::cout << "  m1: " << data.value<double>("m1") << std::endl;
    std::cout << "  m1: " << data.loan_value("m2").value<uint32_t>("m1") << std::endl;
    std::cout << "  m1: " << data.loan_value("m2").value<float>("m2") << std::endl;

    return 0;
}
