cmake_minimum_required(VERSION 3.24)
project(digiasset_core)

option(BUILD_CLI "Build CLI" ON)
option(BUILD_WEB "Build Web Server" ON)
option(BUILD_TEST "Build Tests" OFF)

# Check if CMAKE_PREFIX_PATH is set.
if (WIN32)
  if(DEFINED CMAKE_PREFIX_PATH)
      foreach(prefix_path ${CMAKE_PREFIX_PATH})
      include_directories("${prefix_path}/include")
      link_directories("${prefix_path}/lib")
    endforeach()
  endif()
endif()

# Set compiler settings
set(CMAKE_CXX_STANDARD 11)
if(MSVC)
  # MSVC equivalent flags:
  # -std=c++11 : Not needed as CMAKE_CXX_STANDARD handles this
  # -g : Debug info is controlled by CMAKE_BUILD_TYPE in MSVC
  # -Wall : MSVC uses /W4 for comprehensive warnings
  set(CMAKE_CXX_FLAGS "/W4 /EHsc")

  # Disable Windows features not needed.
  add_definitions(-D_WIN32_WINNT=0x0601)
else()
  SET(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall")
endif()

# Set version number
SET(MAJOR_VERSION 0)
SET(MINOR_VERSION 3)
SET(PATCH_VERSION 0)
SET(SO_VERSION    0)

# Add source directory
include_directories(src)
ADD_SUBDIRECTORY(src)

#create cli
if(BUILD_CLI)
    include_directories(cli)
    ADD_SUBDIRECTORY(cli)
endif()

#create web
if(BUILD_WEB)
    include_directories(web)
    ADD_SUBDIRECTORY(web)
endif()

#build test script
if(BUILD_TEST)
    set(GOOGLETEST_VERSION 1.11.0)
    ADD_SUBDIRECTORY(tests)
endif()
