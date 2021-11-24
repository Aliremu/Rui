cmake_minimum_required(VERSION 3.8)
project("VulkanEngine")
set(CMAKE_SUPPRESS_REGENERATION true)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Sandbox)
# Add sub-directories
add_subdirectory("Sandbox")
add_subdirectory("Rui")