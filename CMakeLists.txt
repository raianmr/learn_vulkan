#[[setup]]

cmake_minimum_required(VERSION 3.21)
project(learn_vulkan VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)

#[[source files]]

# shader files go here
set(
	SHADER_FILES
	shaders/glsl/test.frag
	shaders/glsl/test.vert
)

# *.c/cpp/h/hpp files go here
add_executable(
	${PROJECT_NAME}
	source/main.cpp
	source/lvk/canvas.cpp
	source/lvk/canvas.hpp
	source/lvk/app.cpp
	source/lvk/app.hpp
	source/lvk/channel.cpp
	source/lvk/channel.hpp

	source/lvk/apparatus.cpp
	source/lvk/apparatus.hpp
)

#[[dependencies]]

# Vulkan
find_package(Vulkan REQUIRED)
#target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

# GLM
find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} glm::glm)

# GLFW
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} glfw)

# glslc - for compiling glsl shaders to spir-v
find_program(GLSLC_CLI NAMES glslc REQUIRED)
#message(${GLSLC_CLI})

#[[compile shaders]]

# path to the generated *.spv files
set(SPV_BIN_DIR ${CMAKE_BINARY_DIR}/shaders/spv)

foreach (SHADER_FILE ${SHADER_FILES})
	# all paths need to be absolute for some reason
	set(SHADER_FILE ${CMAKE_SOURCE_DIR}/${SHADER_FILE})
	get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
	set(SPV_BIN_FILE ${SPV_BIN_DIR}/${SHADER_NAME}.spv)

	# note that the shaders get compiled at *build* time
	add_custom_command(
		OUTPUT ${SPV_BIN_FILE}
		COMMAND ${GLSLC_CLI} ${SHADER_FILE} -o ${SPV_BIN_FILE}
		DEPENDS ${SHADER_FILE}
		COMMENT "Compiling ${SHADER_FILE}"
	)
	list(APPEND SPV_BIN_FILES ${SPV_BIN_FILE})
endforeach ()

add_custom_target(SHADERS ALL DEPENDS ${SPV_BIN_FILES})

# cmake won't do all that extra work unless you make it explicit
add_dependencies(${PROJECT_NAME} SHADERS)
