cmake_minimum_required(VERSION 3.11.0)

message(STATUS "BUILDDIR: ${CMAKE_BUILD_DIRECTORY} or ${CMAKE_BUILD_FILES_DIRECTORY}")
set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 17)
SET(CMAKE_SHARED_LINKER_FLAGS "-llog")

#add loguru
ADD_LIBRARY(libloguru STATIC
  ${PROJECT_SOURCE_DIR}/../../cpp/external/V2X-Native_Core/external/loguru/loguru.cpp
)
TARGET_INCLUDE_DIRECTORIES(libloguru PUBLIC ${PROJECT_SOURCE_DIR}/../../cpp/external/V2X-Native_Core/external/loguru)
TARGET_LINK_LIBRARIES(libloguru PRIVATE ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
SET_TARGET_PROPERTIES(libloguru PROPERTIES CXX_CLANG_TIDY "") # do not run clang-tidy here
TARGET_COMPILE_OPTIONS(libloguru PRIVATE -DLOGURU_WITH_FILEABS=1)

#add libmosquitto (client only)
SET(WITH_TLS OFF CACHE BOOL "" FORCE)
SET(WITH_SOCKS OFF CACHE BOOL "" FORCE)
SET(WITH_CJSON OFF CACHE BOOL "" FORCE)
SET(WITH_BROKER OFF CACHE BOOL "" FORCE)
SET(WITH_APPS OFF CACHE BOOL "" FORCE)
SET(WITH_PLUGINS OFF CACHE BOOL "" FORCE)
SET(DOCUMENTATION OFF CACHE BOOL "" FORCE)
SET(WITH_STATIC_LIBRARIES ON CACHE BOOL "" FORCE)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/../../cpp/external/mosquitto ${PROJECT_SOURCE_DIR}/build/eclipse/mosquitto/${ANDROID_ABI})
TARGET_INCLUDE_DIRECTORIES(libmosquitto PUBLIC ${PROJECT_SOURCE_DIR}/../../cpp/external/mosquitto/include)

add_library(cpp
            SHARED
            ../../cpp/RnCore.cpp
            cpp-adapter.cpp
)

add_subdirectory(../../cpp/external/V2X-Native_Core ${PROJECT_SOURCE_DIR}/build/nativecore/${ANDROID_ABI})

TARGET_LINK_LIBRARIES(cpp PUBLIC libmosquitto)
TARGET_LINK_LIBRARIES(cpp PRIVATE libloguru)
TARGET_LINK_LIBRARIES(cpp PUBLIC native_core)

# Specifies a path to native header files.
include_directories(
            ../../cpp
            ../../cpp/external/V2X-Native_Core/include
)
