file(GLOB_RECURSE ALL_HEADERS
    ${PROJECT_SOURCE_DIR}/include/dds/core/**/*.hpp
    )

set(CORE_STANDALONE ${PROJECT_NAME}_core_standalone)

set(${CORE_STANDALONE}_source_files
    ${ALL_HEADERS}
    core.cpp
    conformance.cpp
    Exception.cpp
    macros.cpp
    Reference.cpp
    refmacros.cpp
    ref_traits.cpp
    SafeEnumeration.cpp
    types.cpp
    detail/conformance.cpp
    detail/inttypes.cpp
    detail/macros.cpp
    detail/ref_traits.cpp
    xtypes/Annotation.cpp
    xtypes/CollectionTypes.cpp
    xtypes/DynamicData.cpp
    xtypes/DynamicType.cpp
    xtypes/MemberType.cpp
    xtypes/PrimitiveTypes.cpp
    xtypes/StructType.cpp
    xtypes/TypeKind.cpp
    xtypes/TypeProvider.cpp
    xtypes/UnionCase.cpp
    xtypes/UnionType.cpp
    xtypes/detail/Annotation.cpp
    xtypes/detail/CollectionTypes.cpp
    xtypes/detail/DynamicData.cpp
    xtypes/detail/DynamicType.cpp
    xtypes/detail/MemberType.cpp
    xtypes/detail/PrimitiveTypes.cpp
    xtypes/detail/StructType.cpp
    xtypes/detail/TypeProvider.cpp
    xtypes/detail/UnionCase.cpp
    xtypes/detail/UnionType.cpp
    )

add_library(${CORE_STANDALONE} ${${CORE_STANDALONE}_source_files})

target_include_directories(${CORE_STANDALONE}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    )

set_target_properties(${CORE_STANDALONE}
    PROPERTIES
        CXX_STANDARD 11
    )
