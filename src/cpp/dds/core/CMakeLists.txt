file(GLOB_RECURSE ALL_HEADERS
    ${PROJECT_SOURCE_DIR}/include/dds/core/**/*.hpp
    )

set(CORE_STANDALONE ${PROJECT_NAME}_core_standalone)

set(${CORE_STANDALONE}_source_files
    ${ALL_HEADERS}
    core.cpp
    xtypes/Annotation.cpp
    xtypes/CollectionTypes.cpp
    xtypes/DynamicData.cpp
    xtypes/DynamicType.cpp
    xtypes/MemberType.cpp
    xtypes/StructType.cpp
    xtypes/TypeProvider.cpp
    xtypes/UnionCase.cpp
    xtypes/UnionType.cpp
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
