cmake_minimum_required ( VERSION 2.8 )

project ( luq C )
include ( cmake/dist.cmake )
include ( lua )

set ( CMAKE_THREAD_PREFER_PTHREAD TRUE )
include ( FindThreads )

include_directories ( ${CMAKE_CURRENT_SOURCE_DIR}/src )

set ( LUQ_SOURCE 
  src/l52util.c  src/luq_library_lock.c src/luq.c
  src/luq_map.c  src/luq_pthread.c      src/luq_qvoid.c
)

set ( LUQ_SOURCE USE_SHA1 )

install_lua_module ( luq ${LUQ_SOURCE} LINK ${CMAKE_THREAD_LIBS_INIT} )

install_data ( README.md LICENSE )

install_test ( test/ )
