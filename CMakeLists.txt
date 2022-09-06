cmake_minimum_required(VERSION 3.18.2)

project(tinybot)

set(CMAKE_CXX_STANDARD 14)

set(TENSORRT_INCLUDE_DIR /home/xom/TensorRT-8.4.1.5/include)
set(TENSORRT_LIBRARY /home/xom/TensorRT-8.4.1.5/lib/libnvinfer.so)
set(ENV{CUDAToolkit_ROOT} /opt/cuda)
set(ENV{CUDACXX} /opt/cuda/bin/nvcc)

find_library(TCMALLOC_LIB NAMES tcmalloc_minimal HINTS /usr)
if(NOT TCMALLOC_LIB)
        message(FATAL_ERROR "Could not find tcmalloc")
endif()

find_package(ZLIB REQUIRED)

include_directories(${ZLIB_INCLUDE_DIRS})

add_executable(tinybot
        cnpy/cnpy.cpp
        pcg/pcg_uint128.cpp
        pcg/pcg_extras.cpp
        pcg/pcg_random.cpp
        unionfind.cpp
        misc.cpp
        constants.cpp
        board.cpp
        deck.cpp
        pool.cpp
        infer.cpp
        search.cpp
        npy.cpp
        main.cpp)
target_link_libraries(tinybot ${TCMALLOC_LIB})
target_link_libraries(tinybot ${ZLIB_LIBRARIES})

find_package(CUDAToolkit REQUIRED)
find_path(TENSORRT_INCLUDE_DIR NvInfer.h HINTS ${TENSORRT_ROOT_DIR} ${CUDAToolkit_INCLUDE_DIRS} PATH_SUFFIXES include)
if((NOT TENSORRT_INCLUDE_DIR))
        message(FATAL_ERROR "${ColorBoldRed} NvInfer.h was NOT found, specify TENSORRT_INCLUDE_DIR to indicate where it is. ${ColorReset}")
endif()
find_library(TENSORRT_LIBRARY nvinfer HINTS ${TENSORRT_ROOT_DIR} ${CUDAToolkit_LIBRARY_DIR} PATH_SUFFIXES lib64)
include_directories(SYSTEM ${CUDAToolkit_INCLUDE_DIRS} ${TENSORRT_INCLUDE_DIR}) #SYSTEM is for suppressing some compiler warnings in thrust libraries
target_link_libraries(tinybot CUDA::cudart_static ${TENSORRT_LIBRARY})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")
