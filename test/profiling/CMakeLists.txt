set(RTPSCOMMSPROFILING_SOURCE
   RTPSCommsProfiling.cpp
   RTPSAsSocketReader.cpp
   RTPSAsSocketWriter.cpp)

add_executable(RTPSCommsProfiling ${RTPSCOMMSPROFILING_SOURCE})
target_compile_definitions(RTPSCommsProfiling PRIVATE FASTRTPS_NO_LIB)
target_include_directories(RTPSCommsProfiling PRIVATE ../ ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME})
target_link_libraries(RTPSCommsProfiling ${Boost_LIBRARIES} fastrtps fastcdr -lpthread)

set(RTPSPUBSUBPROFILING_SOURCE
    RTPSPubSubProfiling.cpp
    types/HelloWorld.cpp
    types/HelloWorldType.cpp)

add_executable(RTPSPubSubProfiling ${RTPSPUBSUBPROFILING_SOURCE})
target_compile_definitions(RTPSPubSubProfiling PRIVATE FASTRTPS_NO_LIB)
target_include_directories(RTPSPubSubProfiling PRIVATE ../ ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME})
target_link_libraries(RTPSPubSubProfiling ${Boost_LIBRARIES} fastrtps fastcdr -lpthread)

add_subdirectory(memory)
add_subdirectory(cycles)
