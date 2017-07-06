cmake_minimum_required (VERSION 3.1)

project (Imgine)
set (Imgine_VERSION_MAJOR 0)
set (Imgine_VERSION_MINOR 0)
set (Imgine_VERSION_PATCH 0)
set (Imgine_VERSION ${Imgine_VERSION_MAJOR}.${Imgine_VERSION_MINOR}.${Imgine_VERSION_PATCH})

configure_file (
  "${PROJECT_SOURCE_DIR}/ImgineConfig.h.in"
  "${PROJECT_BINARY_DIR}/ImgineConfig.h"
  )
include_directories ("${PROJECT_BINARY_DIR}")

set (CMAKE_CXX_STANDARD 14)

find_package (OpenCV 3.1 REQUIRED)
message (STATUS "OpenCV library status:")
message (STATUS "    version: ${OpenCV_VERSION}")
message (STATUS "    libraries: ${OpenCV_LIBS}")
message (STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

find_package (Boost 1.55 REQUIRED COMPONENTS
  program_options)

find_package (Threads)

add_executable (imgine main.cpp img_core.cpp img_core_algo.cpp util_color.cpp util_term.cpp)
target_link_libraries (imgine ${OpenCV_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} edit)
