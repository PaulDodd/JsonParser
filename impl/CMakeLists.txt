
# observed that these FLAGS are not being updated
SET (CMAKE_CXX_FLAGS          "${CMAKE_CXX_FLAGS} -std=c++11 -fabi-version=7 -lstdc++ -ljansson")# -stdlib=libc++ -lstdc++ -std=c++11
SET (CMAKE_CXX_FLAGS          "${CMAKE_CXX_FLAGS} -Wno-deprecated")


# build test programs.
set(TEST_SOURCE_FILE "${PROJECT_SOURCE_DIR}/json_wrapper.cxx")
set(TEST_PROG_NAME "json_test")
add_executable(${TEST_PROG_NAME} ${TEST_SOURCE_FILE})
install (TARGETS ${TEST_PROG_NAME} DESTINATION ${PROJECT_BINARY_DIR})

