
cmake_minimum_required( VERSION 3.0 )
project( optional )

set( CMAKE_CXX_STANDARD 17 )

find_package( benchmark REQUIRED )

add_library( getpath SHARED getpath.cpp )
add_executable( benchmark_optional benchmark_optional.cpp )
target_link_libraries( benchmark_optional benchmark::benchmark getpath )
