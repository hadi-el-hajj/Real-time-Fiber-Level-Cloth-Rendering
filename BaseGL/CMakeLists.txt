cmake_minimum_required(VERSION 3.0)

project(BaseGL)

add_subdirectory(External)

add_executable (
	BaseGL
	Main.cpp
)

# Copy the shader files in the binary location. 

add_custom_command(TARGET BaseGL 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:BaseGL> ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(BaseGL LINK_PRIVATE glad)

target_link_libraries(BaseGL LINK_PRIVATE glfw)

target_link_libraries(BaseGL LINK_PRIVATE glm)
