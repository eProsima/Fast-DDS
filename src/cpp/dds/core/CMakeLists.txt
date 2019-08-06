###############################################################################
# Library
###############################################################################

file(GLOB_RECURSE ALL_HEADERS
    ${PROJECT_SOURCE_DIR}/include/dds/core/**/*.hpp
    )

set(CORE_STANDALONE ${PROJECT_NAME})

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
        $<INSTALL_INTERFACE:include/fastrtps>
    )

set_target_properties(${CORE_STANDALONE}
    PROPERTIES
        CXX_STANDARD 11
    )

###############################################################################
# Packaging
###############################################################################

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/dds
    DESTINATION ${INCLUDE_INSTALL_DIR}/fastrtps
    FILES_MATCHING PATTERN "*.hpp"
    )

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}${MSVCARCH_DIR_EXTENSION}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}${MSVCARCH_DIR_EXTENSION}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}${MSVCARCH_DIR_EXTENSION}
    COMPONENT libraries${MSVCARCH_EXTENSION}
    )

export(TARGETS ${PROJECT_NAME} FILE ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-targets.cmake)

if(INSTALLER_PLATFORM)
    set(INSTALL_DESTINATION_PATH ${DATA_INSTALL_DIR}/${PROJECT_NAME}-${INSTALLER_PLATFORM}/cmake)
else()
    set(INSTALL_DESTINATION_PATH ${DATA_INSTALL_DIR}/${PROJECT_NAME}/cmake${MSVCARCH_DIR_EXTENSION_EXT})
endif()

install(EXPORT ${PROJECT_NAME}-targets
    DESTINATION ${INSTALL_DESTINATION_PATH}
    COMPONENT cmake
    )

###############################################################################
# Create CMake package config file
###############################################################################
include(CMakePackageConfigHelpers)

set(FASTRTPS_PACKAGE_OPT_BIN_DIR_CONDITION "if(0)")

configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/packaging/Config.cmake.in
    ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config.cmake
INSTALL_DESTINATION ${INSTALL_DESTINATION_PATH}
    PATH_VARS BIN_INSTALL_DIR INCLUDE_INSTALL_DIR LIB_INSTALL_DIR
    )
write_basic_package_version_file(${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
    )
install(FILES ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config.cmake
    ${PROJECT_BINARY_DIR}/cmake/config/${PROJECT_NAME}-config-version.cmake
DESTINATION ${INSTALL_DESTINATION_PATH}
    COMPONENT cmake
    )
