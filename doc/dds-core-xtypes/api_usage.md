# API usage
## DynamicType definition
```c++
StructType inner("InnerType");
inner.add_member(Member("im1", primitive_type<uint32_t>()));
inner.add_member(Member("im2", primitive_type<float>()).optional());

StructType outter("OutterType");
outter.add_member("om1", primitive_type<double>());
outter.add_member("om2", inner);
outter.add_member("om3", StringType());
outter.add_member("om4", SequenceType(primitive_type<uint32_t>(), 5));
outter.add_member("om5", SequenceType(inner));
outter.add_member("om6", ArrayType(primitive_type<uint32_t>(), 4));
outter.add_member("om7", ArrayType(inner, 4));
```

## DynamicData instantiation
```c++
DynamicData data(outter);
data["om1"].value(6.7);                     //PrimitiveType<double>
data["om2"]["im1"].value(42);               //PrimitiveType<uint32_t>
data["om2"]["im2"].value(35.8f);            //PrimitiveType<float>
data["om3"].value<std::string>("Hi!");      //StringType
data["om3"].string("This is a string!");    //...
data["om4"].push(12);                       //SequenceType(PrimitiveType<uint32_t>)
data["om4"].push(31);                       //...
data["om4"].push(50);                       //...
data["om4"][1].value(100);                  //...
data["om5"].push(data["om2"]);              //SequenceType(inner)
data["om5"][0] = data["om2"];               //...
data["om6"][1].value(123);                  //ArrayType(PrimitiveType<uint32_t>)
data["om7"][1] = data["om2"];               //ArrayType(inner)
```

## Xtypes API
### Utilities
#### Operations
```c++
DynamicType t1;
//...
DynamicType t2;
//...

t1.is_subset_of(t2) //bool
```

```c++
DynamicType type;
//...

DynamicData d1(type);
//...
DynamicData d2(type);
//...

d1 == d2 //deep comparation
d1 != d2
d1 = d2
```

#### Visitor
Useful for buinding conversors
```c++
data.for_each([&](const DynamicData::ReadableNode& node) //Also the WritableNode version
{
    switch(node.data().type().kind())
    {
        case TypeKind::STRUCTURE_TYPE:
        case TypeKind::SEQUENCE_TYPE:
        case TypeKind::ARRAY_TYPE:
        case TypeKind::STRING_TYPE:
        case TypeKind::UINT_32_TYPE:
        case ...
        default:
    }

    node.data //For a view of the data
    node.type //Related type
    node.deep //nested deep
    node.parent //the parent node int the data tree
    node.access().index() //the parent access as index to this data
    node.access().member() //the parent access as member to this data
});
```
