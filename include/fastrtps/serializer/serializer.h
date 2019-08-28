#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/TypeObjectHashId.h>
namespace eprosima{

namespace fastcdr{
    class Cdr ;
}
namespace fastrtps{

namespace types{
    class DynamicData ;
}

namespace serializer{
	

enum class theType {DynamicData, MemberFlag, TypeFlag, TypeObjectHashId} ;

/*********************
 * THE POINTER UNION *
 *********************/
union Ptrs
{
public:
    Ptrs(){}

// MANY GETTERS
    types::DynamicData* dd()const noexcept{ return dd_ ; }
    types::MemberFlag* mf()const noexcept{ return mf_ ; }
    types::TypeFlag* tf()const noexcept{ return tf_ ; }
    types::TypeObjectHashId* tohi()const noexcept{ return tohi_ ; }
				
// MANY SETTERS
    void set(types::DynamicData *v){ dd_ = v ; }
    void set(types::MemberFlag *v){ mf_ = v ; }
    void set(types::TypeFlag *v){ tf_ = v ; }
    void set(types::TypeObjectHashId *v){ tohi_ = v ; }
    
private:
    types::DynamicData *dd_ ;
    types::MemberFlag *mf_ ;
    types::TypeFlag *tf_ ;
    types::TypeObjectHashId *tohi_ ;
};


/*********************
 *     SERIALIZER    *
 *********************/

class Serializer{
private:
    Serializer() = delete ;
public:

    Serializer(types::DynamicData *v):p_(), t_(theType::DynamicData){ p_.set(v); }
    Serializer(types::MemberFlag *v):p_(), t_(theType::MemberFlag){ p_.set(v); }
    Serializer(types::TypeFlag *v):p_(), t_(theType::TypeFlag){ p_.set(v); }
    Serializer(types::TypeObjectHashId *v):p_(), t_(theType::TypeObjectHashId){ p_.set(v); }

	
    void serialize(eprosima::fastcdr::Cdr& cdr) ;
    void deserialize(eprosima::fastcdr::Cdr& cdr) ;

private:
// FATTI PER ORA

    void serializeDynData(eprosima::fastrtps::types::DynamicData *dd, eprosima::fastcdr::Cdr& cdr) const ;
    void serialize_discriminator(eprosima::fastrtps::types::DynamicData *dd, eprosima::fastcdr::Cdr& cdr) const ;
    void serialize_empty_data(eprosima::fastrtps::types::DynamicType *dt, eprosima::fastcdr::Cdr& cdr) const ;
    bool deserializeDynData(eprosima::fastrtps::types::DynamicData *dd, eprosima::fastcdr::Cdr& cdr)const ;
    bool deserialize_discriminator(eprosima::fastcdr::Cdr& cdr);

    static size_t getCdrSerializedSize(
            const eprosima::fastrtps::types::DynamicData* data,
            size_t current_alignment = 0);

    static size_t getEmptyCdrSerializedSize(
            const eprosima::fastrtps::types::DynamicType* type,
            size_t current_alignment = 0);

    static size_t getKeyMaxCdrSerializedSize(
            const eprosima::fastrtps::types::DynamicType_ptr type,
            size_t current_alignment = 0);

    static size_t getMaxCdrSerializedSize(
            const eprosima::fastrtps::types::DynamicType_ptr type,
            size_t current_alignment = 0);

    void serializeKeyDynData(eprosima::fastcdr::Cdr& cdr) const;

				void serializeMemberFlag(eprosima::fastrtps::types::MemberFlag *mf, eprosima::fastcdr::Cdr &cdr) const ;
				void deserializeMemberFlag(eprosima::fastrtps::types::MemberFlag *mf, eprosima::fastcdr::Cdr &cdr);
				size_t getCdrSerializedSize(const eprosima::fastrtps::types::MemberFlag&, size_t current_alignment);

				void serializeTypeFlag(eprosima::fastrtps::types::TypeFlag *tf, eprosima::fastcdr::Cdr &cdr) const;
				void deserializeTypeFlag(eprosima::fastrtps::types::TypeFlag *tf, eprosima::fastcdr::Cdr &cdr);
				size_t getCdrSerializedSize(const eprosima::fastrtps::types::TypeFlag&, size_t current_alignment);

    static size_t getCdrSerializedSize(const eprosima::fastrtps::types::TypeObjectHashId& data, size_t current_alignment = 0);
    void serializeTypeObjectHashId(eprosima::fastrtps::types::TypeObjectHashId *tohi, eprosima::fastcdr::Cdr &cdr) const;
    void deserializeTypeObjectHashId(eprosima::fastrtps::types::TypeObjectHashId *tohi, eprosima::fastcdr::Cdr &cdr);
// ANCORA DA FARE

//    void serializeKeyTypeObjectHashId(eprosima::fastcdr::Cdr &cdr) const;
//    static size_t getKeyMaxCdrSerializedSize(eprosima::fastrtps::types::TypeObjectHashId *tohi, size_t current_alignment = 0);
//    static bool isKeyDefined();

    

private: //just for readibility: above methods are private as well
    Ptrs p_ ;
    theType t_ ;

};

//operator << to serialize a DD into cdr
eprosima::fastcdr::Cdr& operator <<(eprosima::fastcdr::Cdr& cdr, eprosima::fastrtps::types::DynamicData &dd)
{
    Serializer(&dd).serialize(cdr) ;
    return cdr ;
}
//operator >> to deserialize a DD into cdr
eprosima::fastcdr::Cdr& operator >>(eprosima::fastcdr::Cdr& cdr, eprosima::fastrtps::types::DynamicData &dd)
{
    Serializer(&dd).deserialize(cdr) ;
    return cdr ;
}


} //namespace serializer
} //namespace fastrtps
} //namespace eprosima
