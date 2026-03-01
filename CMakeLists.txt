cmake_minimum_required( VERSION 3.12 )

set( CMAKE_C_COMPILER clang )
set( CMAKE_CXX_COMPILER clang++ )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" )
set( CMAKE_CXX_STANDARD 17 )

project( RedditHelp )

add_executable( biblecount BibleCount.cpp )
add_executable( reusableids ReusableIds.cpp )

find_package( Threads REQUIRED )
find_package( GTest REQUIRED )
enable_testing()
add_executable( bitstreamwriter_test BitStreamWriter_test.cpp )
target_link_libraries( bitstreamwriter_test PRIVATE GTest::gtest GTest::gtest_main Threads::Threads )
add_test( NAME bitstreamwriter_test COMMAND bitstreamwriter_test )

