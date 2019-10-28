###############################################################################
# Library
###############################################################################

file(GLOB_RECURSE ALL_HEADERS
    ${PROJECT_SOURCE_DIR}/include/dds/core/xtypes/*.hpp
    )

set(XTYPES_STANDALONE ${PROJECT_NAME})

set(${XTYPES_STANDALONE}_source_files
    ${ALL_HEADERS}
    TypeKind.cpp
    DynamicType.cpp
    SequenceInstance.cpp
    PrimitiveTypes.cpp
    CollectionType.cpp
    ArrayType.cpp
    MutableCollectionType.cpp
    StringType.cpp
    SequenceType.cpp
    Member.cpp
    AggregationType.cpp
    StructType.cpp
    DynamicData.cpp
    )

add_library(${XTYPES_STANDALONE} ${${XTYPES_STANDALONE}_source_files})

target_include_directories(${XTYPES_STANDALONE}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/fastrtps>
    )

set_target_properties(${XTYPES_STANDALONE}
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
