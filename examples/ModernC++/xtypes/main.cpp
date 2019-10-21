#include <dds/core/xtypes/xtypes.hpp>

#include <iostream>

using namespace dds::core::xtypes;

int main()
{
    StructType inner("InnerType");
    inner.add_member(Member("im1", primitive_type<uint32_t>()));
    inner.add_member(Member("im2", primitive_type<float>()).id(2));

    StructType outter("OutterType");
    outter.add_member("om1", primitive_type<double>());
    outter.add_member("om2", inner);
    outter.add_member("om3", StringType());
    outter.add_member("om4", SequenceType(primitive_type<uint32_t>(), 5));
    outter.add_member("om5", SequenceType(inner));
    outter.add_member("om6", ArrayType(primitive_type<uint32_t>(), 4));
    outter.add_member("om7", ArrayType(inner, 4));

    DynamicData data(outter);
    data["om1"].value(6.7);                     //PrimitiveType<double>
    data["om2"]["im1"].value(42u);              //PrimitiveType<uint32_t>
    data["om2"]["im2"].value(35.8f);            //PrimitiveType<float>
    data["om3"].value<std::string>("Hi!");      //StringType
    data["om3"].string("This is a string!");    //...
    data["om4"].push(12u);                      //SequenceType(PrimitiveType<uint32_t>)
    data["om4"].push(31u);                      //...
    data["om4"].push(50u);                      //...
    data["om4"][1].value(100u);                 //...
    data["om5"].push(data["om2"]);              //SequenceType(inner)
    data["om5"][0] = data["om2"];               //...
    data["om6"][1].value(123u);                 //ArrayType(PrimitiveType<uint32_t>)
    data["om7"][1] = data["om2"];               //ArrayType(inner)

    std::cout << data.to_string() << std::endl; //See to_string implementation as an example of data instrospection

    return 0;
}
