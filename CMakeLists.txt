#--------------------------------------------------------------------
# prelude
#--------------------------------------------------------------------

project( ni-media )

cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0025 NEW)

get_filename_component(nilibs_path ../nilibs ABSOLUTE)
set_property(GLOBAL PROPERTY NILIBS_DIR ${nilibs_path})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${nilibs_path}/support/cmake)

include( ni-compiler-base )

enable_testing()
option( NIMEDIA_TESTS "Build ni-media Tests" ON )


# beatiful colors when using clang
if(APPLE)
  set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fcolor-diagnostics" )
endif()


#--------------------------------------------------------------------
# add targets
#--------------------------------------------------------------------
set(NI_UNITY_BUILDS false)
add_subdirectory( source )

if( NIMEDIA_TESTS )
  add_subdirectory( test )
endif()


#--------------------------------------------------------------------
# group in project
#--------------------------------------------------------------------

foreach( target alac ni-media ni-media_test )
  if( TARGET ${target} )
    set_property( TARGET ${target} PROPERTY FOLDER ni-media )
  endif()
endforeach()
