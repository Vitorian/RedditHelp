cmake_minimum_required( VERSION 3.12 )
project(calculator)

set( CMAKE_CXX_STANDARD 17 )

find_package( Boost REQUIRED )

add_executable( calculator calculator.cpp )
target_link_libraries( calculator PRIVATE Boost::boost )

find_package( GTest REQUIRED )
enable_testing()
add_executable( calculator_test calculator_test.cpp )
target_link_libraries( calculator_test PRIVATE Boost::boost GTest::gtest GTest::gtest_main )
add_test( NAME calculator_test COMMAND calculator_test )
