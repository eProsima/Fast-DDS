#add_subdirectory(C++/DeadlineQoSExample)
#add_subdirectory(C++/OwnershipStrengthTest)
#add_subdirectory(C++/HelloWorldTest)

if(NOT ((MSVC OR MSVC_IDE) AND EPROSIMA_INSTALLER) AND fastcdr_FOUND)
    file(WRITE ${${package}ExternalDir}/CMakeLists.txt
        "cmake_minimum_required(VERSION 2.8.11)\n"
        "include(ExternalProject)\n"
        "set(SOURCE_DIR_ \"${PROJECT_SOURCE_DIR}/thirdparty/${package}\")\n"
        "set(GENERATOR_ -G \"${CMAKE_GENERATOR}\")\n"
        "set(CMAKE_INSTALL_PREFIX_ \"-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX_}\")\n"
        "set(CMAKE_PREFIX_PATH_ -DCMAKE_PREFIX_PATH=\"${CMAKE_PREFIX_PATH_}\")\n"
        "ExternalProject_Add(${package}\n"
        "CONFIGURE_COMMAND \"${CMAKE_COMMAND}\"\n"
        "${${package}_CMAKE_ARGS}\n"
        "DOWNLOAD_COMMAND echo\n"
        "UPDATE_COMMAND cd \"${PROJECT_SOURCE_DIR}\" && git submodule update --recursive --init \"thirdparty/${package}\"\n"
        "SOURCE_DIR \${SOURCE_DIR_}\n"
        "BINARY_DIR \"${${package}ExternalDir}/build\"\n"
        ")\n")
endif()
