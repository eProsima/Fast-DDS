project(fuzz_ParameterList)

set(FUZZ_TARGET fuzz_ParameterList)

add_executable(${FUZZ_TARGET} fuzz_ParameterList.cxx)

target_include_directories(${FUZZ_TARGET} PRIVATE
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/build/include
)

target_link_libraries(${FUZZ_TARGET}
    fastdds
    $ENV{LIB_FUZZING_ENGINE}
)
