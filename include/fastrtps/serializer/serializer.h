#include <fastrtps/types/DynamicTypePtr.h>
#include <fastrtps/types/TypeObjectHashId.h>
#include <fastrtps/types/TypeObject.h>
namespace eprosima{

namespace fastcdr{
    class Cdr ;
}
namespace fastrtps{

namespace types{
    class DynamicData ;
}

namespace serializer{

/*********************
 *     SERIALIZER    *
 *********************/
	
template<class Serializable>
class Serializer{
private:
    Serializer() = delete ;
public:
    Serializer<Serializable>(Serializable *p):p_(p){}
    
    void serialize(eprosima::fastcdr::Cdr& cdr)
				{
        serialize(p_, cdr) ;
    }
    void deserialize(eprosima::fastcdr::Cdr& cdr)
				{
        deserialize(p_, cdr) ;
    }


private:
// FATTI PER ORA

/*DynamicData Methods*/
    void serialize(eprosima::fastrtps::types::DynamicData *dd, eprosima::fastcdr::Cdr& cdr) const ;
    void serialize_discriminator(eprosima::fastrtps::types::DynamicData *dd, eprosima::fastcdr::Cdr& cdr) const ;
    void serialize_empty_data(eprosima::fastrtps::types::DynamicType *dt, eprosima::fastcdr::Cdr& cdr) const ;
    bool deserialize(eprosima::fastrtps::types::DynamicData *dd, eprosima::fastcdr::Cdr& cdr) const ;
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

    void serializeKey(eprosima::fastrtps::types::DynamicData *dd, eprosima::fastcdr::Cdr& cdr) const;

/*MemberFlag Methods*/
				void serialize(eprosima::fastrtps::types::MemberFlag *mf, eprosima::fastcdr::Cdr &cdr) const ;
				void deserialize(eprosima::fastrtps::types::MemberFlag *mf, eprosima::fastcdr::Cdr &cdr);
				size_t getCdrSerializedSize(const eprosima::fastrtps::types::MemberFlag&, size_t current_alignment);
/*TypeFlag Methods*/
				void serialize(eprosima::fastrtps::types::TypeFlag *tf, eprosima::fastcdr::Cdr &cdr) const;
				void deserialize(eprosima::fastrtps::types::TypeFlag *tf, eprosima::fastcdr::Cdr &cdr);
				size_t getCdrSerializedSize(const eprosima::fastrtps::types::TypeFlag&, size_t current_alignment);
/*TypeObjectHashId Methods*/
    static size_t getCdrSerializedSize(const eprosima::fastrtps::types::TypeObjectHashId& data, size_t current_alignment = 0);
    void serialize(eprosima::fastrtps::types::TypeObjectHashId *tohi, eprosima::fastcdr::Cdr &cdr) const;
    void deserialize(eprosima::fastrtps::types::TypeObjectHashId *tohi, eprosima::fastcdr::Cdr &cdr);
/*CommonStructMember Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CommonStructMember& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CommonStructMember *csm, eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CommonStructMember *csm, eprosima::fastcdr::Cdr &cdr);
/*CompleteMemberDetail Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CompleteMemberDetail& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CompleteMemberDetail *cmd,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CompleteMemberDetail *cmd,eprosima::fastcdr::Cdr &cdr);
/*MinimalMemberDetail Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::MinimalMemberDetail& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::MinimalMemberDetail *v, eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::MinimalMemberDetail *v, eprosima::fastcdr::Cdr &cdr);
/*CompleteStructMember Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CompleteStructMember& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CompleteStructMember *v, eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CompleteStructMember *v,eprosima::fastcdr::Cdr &cdr);
/*MinimalStructMember Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::MinimalStructMember& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::MinimalStructMember *v, eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::MinimalStructMember *v, eprosima::fastcdr::Cdr &cdr);
/*AppliedBuiltinTypeAnnotations Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::AppliedBuiltinTypeAnnotations& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::AppliedBuiltinTypeAnnotations *v, eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::AppliedBuiltinTypeAnnotations *v, eprosima::fastcdr::Cdr &cdr);
/*MinimalTypeDetail Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::MinimalTypeDetail& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::MinimalTypeDetail *v, eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::MinimalTypeDetail *v, eprosima::fastcdr::Cdr &cdr);
/*CompleteTypeDetail Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CompleteTypeDetail& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CompleteTypeDetail *v, eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CompleteTypeDetail *v, eprosima::fastcdr::Cdr &cdr);
/*CompleteStructHeader Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CompleteStructHeader& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CompleteStructHeader *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CompleteStructHeader *v,eprosima::fastcdr::Cdr &cdr);
/*MinimalStructHeader Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::MinimalStructHeader& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::MinimalStructHeader *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::MinimalStructHeader *v,eprosima::fastcdr::Cdr &cdr);
/*CompleteStructType Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CompleteStructType& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CompleteStructType *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CompleteStructType *v,eprosima::fastcdr::Cdr &cdr);
/*MinimalStructType Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::MinimalStructType& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::MinimalStructType *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::MinimalStructType *v,eprosima::fastcdr::Cdr &cdr);
/*CommonUnionMember Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CommonUnionMember& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CommonUnionMember *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CommonUnionMember *v,eprosima::fastcdr::Cdr &cdr);
/*CompleteUnionMember Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CompleteUnionMember& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CompleteUnionMember *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CompleteUnionMember *v,eprosima::fastcdr::Cdr &cdr);
/*MinimalUnionMember Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::MinimalUnionMember& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::MinimalUnionMember *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::MinimalUnionMember *v,eprosima::fastcdr::Cdr &cdr);
/*CommonDiscriminatorMember Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CommonDiscriminatorMember& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CommonDiscriminatorMember *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CommonDiscriminatorMember *v,eprosima::fastcdr::Cdr &cdr);
/*CompleteDiscriminatorMember Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CompleteDiscriminatorMember& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CompleteDiscriminatorMember *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CompleteDiscriminatorMember *v,eprosima::fastcdr::Cdr &cdr);
/*MinimalDiscriminatorMember Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::MinimalDiscriminatorMember& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::MinimalDiscriminatorMember *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::MinimalDiscriminatorMember *v,eprosima::fastcdr::Cdr &cdr);
/*CompleteUnionHeader Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CompleteUnionHeader& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CompleteUnionHeader *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CompleteUnionHeader *v,eprosima::fastcdr::Cdr &cdr);
/*MinimalUnionHeader Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::MinimalUnionHeader& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::MinimalUnionHeader *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::MinimalUnionHeader *v,eprosima::fastcdr::Cdr &cdr);
/*CompleteUnionType Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CompleteUnionType& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CompleteUnionType *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CompleteUnionType *v,eprosima::fastcdr::Cdr &cdr);
/*MinimalUnionType Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::MinimalUnionType& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::MinimalUnionType *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::MinimalUnionType *v,eprosima::fastcdr::Cdr &cdr);
/*CommonAnnotationParameter Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CommonAnnotationParameter& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CommonAnnotationParameter *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CommonAnnotationParameter *v,eprosima::fastcdr::Cdr &cdr);
/*CompleteAnnotationParameter Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CompleteAnnotationParameter& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CompleteAnnotationParameter *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CompleteAnnotationParameter *v,eprosima::fastcdr::Cdr &cdr);
/*MinimalAnnotationParameter Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::MinimalAnnotationParameter& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::MinimalAnnotationParameter *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::MinimalAnnotationParameter *v,eprosima::fastcdr::Cdr &cdr);
/*CompleteAnnotationHeader Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CompleteAnnotationHeader& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CompleteAnnotationHeader *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CompleteAnnotationHeader *v,eprosima::fastcdr::Cdr &cdr);
/*MinimalAnnotationHeader Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::MinimalAnnotationHeader& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::MinimalAnnotationHeader *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::MinimalAnnotationHeader *v,eprosima::fastcdr::Cdr &cdr);
/*CompleteAnnotationType Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CompleteAnnotationType& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::CompleteAnnotationType *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::CompleteAnnotationType *v,eprosima::fastcdr::Cdr &cdr);
/*MinimalAnnotationType Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::MinimalAnnotationType& data, size_t current_alignment = 0);
    RTPS_DllAPI void serialize(eprosima::fastrtps::types::MinimalAnnotationType *v,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserialize(eprosima::fastrtps::types::MinimalAnnotationType *v,eprosima::fastcdr::Cdr &cdr);

private: //just for readibility: above methods are private as well
    Serializable p_ ;
};

/*********************
 *Serialize Operator *
 *********************/
/*
template <class Serializable>
eprosima::fastcdr::Cdr & operator<<(eprosima::fastcdr::Cdr &cdr, Serializable &S)
{
    Serializer<Serializable>(S).serialize(cdr) ;
				return cdr ;
}

template <class Serializable>
eprosima::fastcdr::Cdr & operator>>(eprosima::fastcdr::Cdr &cdr, Serializable &S)
{
    Serializer<Serializable>(S).deserialize(cdr) ;
				return cdr ;
}
*/

} //namespace serializer
} //namespace fastrtps
} //namespace eprosima
