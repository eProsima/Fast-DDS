project(fuzz_TypeLookupTypes)

set(FUZZ_TARGET fuzz_TypeLookupTypes)

add_executable(${FUZZ_TARGET} fuzz_TypeLookupTypes.cxx)

target_include_directories(${FUZZ_TARGET} PRIVATE
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/build/include
    ${PROJECT_SOURCE_DIR}/src/cpp/fastdds/builtin/type_lookup_service/detail
)

target_link_libraries(${FUZZ_TARGET}
    fastdds
    $ENV{LIB_FUZZING_ENGINE}
)
