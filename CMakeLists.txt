cmake_minimum_required(VERSION 3.28)

project(AegixGLTF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TEST "Build test project" ON)

add_library(AegixGLTF STATIC)

target_sources(AegixGLTF PRIVATE
    "gltf.cpp"
)

target_include_directories(AegixGLTF PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}/lib"
)


if(BUILD_TEST)
	add_subdirectory("test")
endif()
