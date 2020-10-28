project(gtest)

add_executable(gtest_run ../helper.cpp ${MODULE_DATABASE} ${MODULE_SYS} ${MODULE_PARSER} ${MODULE_TESTS})
target_link_libraries(gtest_run gtest gtest_main fmt::fmt ${Boost_LIBRARIES} pthread)