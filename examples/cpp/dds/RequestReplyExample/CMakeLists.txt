
cmake_minimum_required(VERSION 2.8.12)

project("generated_code")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find requirements
find_package(fastcdr REQUIRED)
find_package(fastrtps REQUIRED)


message(STATUS "Configuring Calculator...")
add_library(Calculator_lib Calculator.cxx)
target_link_libraries(Calculator_lib fastcdr fastrtps)

add_executable(Calculator CalculatorPubSubTypes.cxx CalculatorPublisher.cxx CalculatorSubscriber.cxx CalculatorPubSubMain.cxx)
target_link_libraries(Calculator fastcdr fastrtps
        Calculator_lib )



