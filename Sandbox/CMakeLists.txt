cmake_minimum_required(VERSION 3.8)
project("Sandbox")

message(NOTICE "----- BUILDING ${PROJECT_NAME} -----")

# References NvidiaBuildOptions.cmake to figure out if system is 32/64 bit
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCHITECTURE_SHIT "x64")
else()
    set(ARCHITECTURE_SHIT "x32")
endif()

set(OUTPUT_DIR "Debug-${CMAKE_SYSTEM_NAME}-${ARCHITECTURE_SHIT}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${OUTPUT_DIR}/${PROJECT_NAME})
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

#message(NOTICE "SDL LIB DIR ${SDL2_LIBDIR} ${SDL2_INCLUDE_DIRS} ${SDL2_LIBRARIES}")

link_directories(${SDL2_LIBDIR})

add_executable(${PROJECT_NAME} src/Sandbox.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/Rui/src ${CMAKE_SOURCE_DIR}/Rui/vendor/spdlog/include ${CMAKE_SOURCE_DIR}/Rui/vendor/glm ${CMAKE_SOURCE_DIR}/Rui/vendor/vma-hpp ${Vulkan_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${PHYSX_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Rui ${Vulkan_LIBRARIES} SDL2main SDL2 glm::glm ${PHYSX_LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE RUI_PLATFORM_WINDOWS RUI_ENABLE_ASSERTS)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

#if(MSVC)
    #add_compile_options(
        #$<$<CONFIG:>:/MT> #---------|
        #$<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
        #$<$<CONFIG:Release>:/MT> #--|
    #)
#endif()
