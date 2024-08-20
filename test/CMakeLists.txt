
add_executable(AegixGLTFTest "main.cpp")

target_link_libraries(AegixGLTFTest AegixGLTF)

add_definitions(-DPROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
