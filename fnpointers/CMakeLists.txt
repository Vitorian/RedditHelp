
cmake_minimum_required( VERSION 3.0 )
set(CMAKE_CXX_STANDARD 14 )

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project( fnpointers )
enable_language( CXX ASM )
find_package( benchmark REQUIRED )

add_library( func SHARED func.cpp )
add_executable( bm_fnpointer bm_fnpointer.cpp )
target_link_libraries( bm_fnpointer func benchmark::benchmark pthread )

