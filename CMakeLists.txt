cmake_minimum_required(VERSION 3.1)
project(Emerald)

find_package(OpenGL REQUIRED)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_subdirectory(external)

include_directories(
	external/glfw-3.1.2/include/
	external/glm-0.9.7.1/
	external/glad-opengl-3.3/include/
	external/tinygltf-2.9.3/
	external/
	src/
)

add_executable(main
	src/render/shader.cpp
	src/main.cpp
	src/helpers.cpp

	src/objects/obj/gltfObj.cpp
	src/objects/skybox/skybox.cpp
)
target_link_libraries(main
	${OPENGL_LIBRARY}
	glfw
	glad
)
