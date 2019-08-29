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
 * THE POINTER UNION *
 *********************/

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
        serializeMe(p_, cdr) ;
    }
    void deserialize(eprosima::fastcdr::Cdr& cdr)
				{
        deserializeMe(p_, cdr) ;
    }


private:
// FATTI PER ORA

/*DynamicData Methods*/
    void serializeMe(eprosima::fastrtps::types::DynamicData *dd, eprosima::fastcdr::Cdr& cdr) const ;
    void serialize_discriminator(eprosima::fastrtps::types::DynamicData *dd, eprosima::fastcdr::Cdr& cdr) const ;
    void serialize_empty_data(eprosima::fastrtps::types::DynamicType *dt, eprosima::fastcdr::Cdr& cdr) const ;
    bool deserializeMe(eprosima::fastrtps::types::DynamicData *dd, eprosima::fastcdr::Cdr& cdr) const ;
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
				void serializeMe(eprosima::fastrtps::types::MemberFlag *mf, eprosima::fastcdr::Cdr &cdr) const ;
				void deserializeMe(eprosima::fastrtps::types::MemberFlag *mf, eprosima::fastcdr::Cdr &cdr);
				size_t getCdrSerializedSize(const eprosima::fastrtps::types::MemberFlag&, size_t current_alignment);
/*TypeFlag Methods*/
				void serializeMe(eprosima::fastrtps::types::TypeFlag *tf, eprosima::fastcdr::Cdr &cdr) const;
				void deserializeMe(eprosima::fastrtps::types::TypeFlag *tf, eprosima::fastcdr::Cdr &cdr);
				size_t getCdrSerializedSize(const eprosima::fastrtps::types::TypeFlag&, size_t current_alignment);
/*TypeObjectHashId Methods*/
    static size_t getCdrSerializedSize(const eprosima::fastrtps::types::TypeObjectHashId& data, size_t current_alignment = 0);
    void serializeMe(eprosima::fastrtps::types::TypeObjectHashId *tohi, eprosima::fastcdr::Cdr &cdr) const;
    void deserializeMe(eprosima::fastrtps::types::TypeObjectHashId *tohi, eprosima::fastcdr::Cdr &cdr);
/*CommonStructMember Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CommonStructMember& data, size_t current_alignment = 0);
    RTPS_DllAPI void serializeMe(eprosima::fastrtps::types::CommonStructMember *csm, eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserializeMe(eprosima::fastrtps::types::CommonStructMember *csm, eprosima::fastcdr::Cdr &cdr);
/*CompleteMemberDetail Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CompleteMemberDetail& data, size_t current_alignment = 0);
    RTPS_DllAPI void serializeMe(eprosima::fastrtps::types::CompleteMemberDetail *cmd,eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserializeMe(eprosima::fastrtps::types::CompleteMemberDetail *cmd,eprosima::fastcdr::Cdr &cdr);
/*MinimalMemberDetail Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::MinimalMemberDetail& data, size_t current_alignment = 0);
    RTPS_DllAPI void serializeMe(eprosima::fastrtps::types::MinimalMemberDetail *v, eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserializeMe(eprosima::fastrtps::types::MinimalMemberDetail *v, eprosima::fastcdr::Cdr &cdr);
/*CompleteStructMember Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CompleteStructMember& data, size_t current_alignment = 0);
    RTPS_DllAPI void serializeMe(eprosima::fastrtps::types::CompleteStructMember *v, eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserializeMe(eprosima::fastrtps::types::CompleteStructMember *v,eprosima::fastcdr::Cdr &cdr);
/*MinimalStructMember Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::MinimalStructMember& data, size_t current_alignment = 0);
    RTPS_DllAPI void serializeMe(eprosima::fastrtps::types::MinimalStructMember *v, eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserializeMe(eprosima::fastrtps::types::MinimalStructMember *v, eprosima::fastcdr::Cdr &cdr);
/*AppliedBuiltinTypeAnnotations Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::AppliedBuiltinTypeAnnotations& data, size_t current_alignment = 0);
    RTPS_DllAPI void serializeMe(eprosima::fastrtps::types::AppliedBuiltinTypeAnnotations *v, eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserializeMe(eprosima::fastrtps::types::AppliedBuiltinTypeAnnotations *v, eprosima::fastcdr::Cdr &cdr);
/*MinimalTypeDetail Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::MinimalTypeDetail& data, size_t current_alignment = 0);
    RTPS_DllAPI void serializeMe(eprosima::fastrtps::types::MinimalTypeDetail *v, eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserializeMe(eprosima::fastrtps::types::MinimalTypeDetail *v, eprosima::fastcdr::Cdr &cdr);
/*CompleteTypeDetail Methods*/
    RTPS_DllAPI static size_t getCdrSerializedSize(const eprosima::fastrtps::types::CompleteTypeDetail& data, size_t current_alignment = 0);
    RTPS_DllAPI void serializeMe(eprosima::fastrtps::types::CompleteTypeDetail *v, eprosima::fastcdr::Cdr &cdr) const;
    RTPS_DllAPI void deserializeMe(eprosima::fastrtps::types::CompleteTypeDetail *v, eprosima::fastcdr::Cdr &cdr);

private: //just for readibility: above methods are private as well
    Serializable p_ ;
};

/*********************
 *Serialize Operator *
 *********************/
// DD SERIALIZER


} //namespace serializer
} //namespace fastrtps
} //namespace eprosima
