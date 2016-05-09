cmake_minimum_required(VERSION 2.8.12)
project("DeadlineQoSExample")

message(STATUS "Configuring DeadlineQoS example...")
file(GLOB DLQOS_SOURCES "*.cxx")

add_executable(DeadlineQoSExample ${DLQOS_SOURCES})
target_include_directories(DeadlineQoSExample PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(DeadlineQosExample fastrtps fastcdr ${Boost_LIBRARIES})
