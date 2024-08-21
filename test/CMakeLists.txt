
project(aegix-gltf-testing)

add_executable(${PROJECT_NAME} "main.cpp")

target_link_libraries(${PROJECT_NAME} Aegix::GLTF)

add_definitions(-DPROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
