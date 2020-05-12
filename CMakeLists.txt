cmake_minimum_required(VERSION 2.6)

project(grammar2code)

# make sure that the default is a RELEASE
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build,
options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release
RelWithDebInfo MinSizeRel." FORCE)
else()
   set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of
build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug
Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# define CMAKE_BUILD_TYPE distribution
set(CMAKE_CXX_FLAGS_DISTRIBUTION ${CMAKE_CXX_FLAGS_RELEASE} CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_DISTRIBUTION ${CMAKE_C_FLAGS_DISTRIBUTION} CACHE STRING "" FORCE)
if(CMAKE_BUILD_TYPE MATCHES Distribution)
  set(Boost_USE_STATIC_LIBS ON)
  if("${CMAKE_SYSTEM_NAME}" MATCHES "LINUX")
    set(CMAKE_EXE_LINKER_FLAGS "-static -pthread" CACHE STRING "" FORCE)
  endif()
endif()

# this would be CMAKE_CXX_FLAGS but these are mandatory
# we write c++0x for compatibility with old compilers
add_definitions("-std=c++0x -Wall")

# this is necessary to have a full -std=c++11 implementation
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_definitions("-stdlib=libc++")
    set(LIBS ${LIBS} "-stdlib=libc++")
endif()

# the same has to be done for the generated xcode project
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

if(NOT CMAKE_CXX_FLAGS_RELEASE AND NOT CMAKE_CXX_FLAGS)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native" CACHE STRING "" FORCE)
endif()

if(NOT CMAKE_CXX_FLAGS_DEBUG)
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb" CACHE STRING "" FORCE)
endif()
find_package(Boost 1.53 COMPONENTS program_options system filesystem regex REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARY_DIRS})
message(WARNING, ${Boost_LIBRARY_DIRS} )
# forcing the linker search path, link_directories is ignore, or the libs
# paths are stripped because they are in the LIBRARY PATH, forcing the
# search path works in these cases and when there are severak versions of
# boost installed in the system
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_LD_FLAGS} "${CMAKE_EXE_LINKER_FLAGS} -L ${Boost_LIBRARY_DIRS}")

add_library (pugixml STATIC
             3d_party/pugixml/src/pugixml.cpp 3d_party/pugixml/src/pugixml.hpp
             3d_party/pugixml/src/pugiconfig.hpp)
set(LIBS pugixml ${LIBS})
include_directories(3d_party/pugixml/src/)

add_library (grammar STATIC
             src/model.cpp src/walker.cpp src/configuration.cpp
             src/irace_conf.cpp src/paramils_conf.cpp src/smac_conf.cpp
             src/error.cpp src/error.hpp src/params2code.cpp src/grammar.hpp)
set(LIBS grammar ${LIBS})

add_executable(grammar2code
               src/grammar.hpp src/main.cpp)
target_link_libraries(grammar2code ${LIBS})
