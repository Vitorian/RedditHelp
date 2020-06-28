set( CMAKE_C_COMPILER clang )
set( CMAKE_CXX_COMPILER clang++ )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" )

project( RedditHelp )

add_executable( biblecount BibleCount.cpp )
add_executable( reusableids ReusableIds.cpp )

