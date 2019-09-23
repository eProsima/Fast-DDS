cmake_minimum_required(VERSION 3.13)


if(GTEST_FOUND)

				set(XTYPES_TEST xtypes_test.cpp)

				add_executable(xtypes_test ${XTYPES_TEST})
				target_include_directories(xtypes_test PRIVATE ${GTEST_INCLUDE_DIRS})
				target_link_libraries(xtypes_test ${GTEST_LIBRARIES})


				add_gtest(NAME xtypes_test COMMAND xtypes_test SOURCES ${XTYPES_TEST})

endif()
