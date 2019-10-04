#ifndef OMG_DDS_CORE_ANNOTATIONKIND_H
#define OMG_DDS_CORE_ANNOTATIONKIND_H


#include <dds/core/SafeEnumeration.hpp>

namespace dds {
namespace core {
namespace xtypes {

struct AnnotationKind_def
{
    enum Type
    {
        ID_ANNOTATION_TYPE,
        OPTIONAL_ANNOTATION_TYPE,
        KEY_ANNOTATION_TYPE,
        SHARED_ANNOTATION_TYPE,
        NESTED_ANNOTATION_TYPE,
        EXTENSIBILITY_ANNOTATION_TYPE,
        MUST_UNDERSTAND_ANNOTATION_TYPE,
        VERBATIM_ANNOTATION_TYPE,
        BITSET_ANNOTATION_TYPE,
        BITSETBOUND_ANNOTATION_TYPE
    };
};

typedef dds::core::SafeEnum<AnnotationKind_def> AnnotationKind;

struct ExtensibilityKind_def
{
    enum Type
    {
        FINAL,
        EXTENSIBLE,
        MUTABLE
    };
};

typedef dds::core::SafeEnum<ExtensibilityKind_def> ExtensibilityKind;

} //xtypes
} //core
} //dds

#endif //OMG_DDS_CORE_ANNOTATIONKIND_H

