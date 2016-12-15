project( multi_index )

set( CMAKE_CXX_FLAGS  "-Wall -Werror -std=c++11 -m64 -msse4.2 ")

add_executable( test_multi_index main.cpp data.cpp collection.cpp )
target_link_libraries( test_multi_index rt )
