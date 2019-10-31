#include <dds/core/xtypes/xtypes.hpp>

#include <iostream>

using namespace dds::core::xtypes;

int main()
{
    StructType inner("InnerType");
    inner.add_member(Member("im1", primitive_type<uint32_t>()));
    inner.add_member(Member("im2", primitive_type<float>()).id(2));

    StructType outer("OuterType");
    outer.add_member("om1", primitive_type<double>());
    outer.add_member("om2", inner);
    outer.add_member("om3", StringType());
    outer.add_member("om4", WStringType());
    outer.add_member("om5", SequenceType(primitive_type<uint32_t>(), 5));
    outer.add_member("om6", SequenceType(inner));
    outer.add_member("om7", ArrayType(primitive_type<uint32_t>(), 4));
    outer.add_member("om8", ArrayType(inner, 4));

    DynamicData data(outer);
    data["om1"].value(6.7);                                //PrimitiveType<double>
    data["om2"]["im1"].value(42u);                         //PrimitiveType<uint32_t>
    data["om2"]["im2"].value(35.8f);                       //PrimitiveType<float>
    data["om3"].value<std::string>("Hi!");                 //StringType
    data["om3"].string("This is a string");                //...
    data["om4"].wstring(L"Also support unicode! \u263A");  //WStringType
    data["om5"].push(12u);                                 //SequenceType(PrimitiveType<uint32_t>)
    data["om5"].push(31u);                                 //...
    data["om5"].push(50u);                                 //...
    data["om5"][1].value(100u);                            //...
    data["om6"].push(data["om2"]);                         //SequenceType(inner)
    data["om6"][0] = data["om2"];                          //...
    data["om7"][1].value(123u);                            //ArrayType(PrimitiveType<uint32_t>)
    data["om8"][1] = data["om2"];                          //ArrayType(inner)

    data["om1"] = 8.88;                                    //PrimitiveType<double>
    data["om3"] = "This is a string, assignable too.";     //StringType
    data["om4"] = L"WStrings attack again! \u263A";        //WStringType

    std::cout << data.to_string() << std::endl; //See to_string implementation as an example of data instrospection

    return 0;
}
