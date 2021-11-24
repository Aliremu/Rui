cmake_minimum_required(VERSION 3.8)
project("Rui")

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

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/*.cpp" "src/*.h")

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

message(NOTICE "SDL LIB DIR ${SDL2_LIBDIR} ${SDL2_INCLUDE_DIRS} ${SDL2_LIBRARIES}")

link_directories(${SDL2_LIBDIR})

add_library(${PROJECT_NAME} ${sources})

#### Build PhysX library ####
# PHYSX_PATH - path to the `{cloned repository}/physx` repo directory git://github.com/NVIDIAGameWorks/PhysX.git
set( PHYSX_ROOT_DIR ${CMAKE_SOURCE_DIR}/Rui/vendor/PhysX/physx ) #This is needed for $ENV{PHYSX_PATH}/compiler/public/CMakeLists.txt
set( PHYSX_INCLUDE_DIRS ${PHYSX_ROOT_DIR}/include/ ${PHYSX_ROOT_DIR}/../pxshared/include/ )
set( PHYSX_LIBRARIES
        PhysXExtensions
        PhysX
        PhysXPvdSDK
        PhysXVehicle
        PhysXCharacterKinematic
        PhysXCooking
        PhysXCommon
        PhysXFoundation
        # SnippetUtils
        )

set(TARGET_BUILD_PLATFORM "windows") # has to match the TARGET_BUILD_PLATFORM in $ENV{PHYSX_PATH}/physix/buildtools/cmake_generate_projects.py
set(PX_BUILDSNIPPETS OFF CACHE BOOL "Generate the snippets")
set(PX_BUILDPUBLICSAMPLES OFF CACHE BOOL "Generate the samples projects")
set(PX_GENERATE_STATIC_LIBRARIES ON CACHE BOOL "Generate static libraries")
set(PX_FLOAT_POINT_PRECISE_MATH OFF CACHE BOOL "Float point precise math")
set(NV_USE_STATIC_WINCRT ON CACHE BOOL "Use the statically linked windows CRT")
set(NV_USE_DEBUG_WINCRT ON CACHE BOOL "Use the debug version of the CRT")
set(PXSHARED_PATH ${PHYSX_ROOT_DIR}/../pxshared)
set(PXSHARED_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CMAKEMODULES_VERSION "1.27")
set(CMAKEMODULES_PATH ${PHYSX_ROOT_DIR}/../externals/cmakemodules)
set(PX_OUTPUT_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/externals/physx)
set(PX_OUTPUT_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/externals/physx)

# Call into PhysX's CMake scripts
add_subdirectory(${PHYSX_ROOT_DIR}/compiler/public externals/physx)

# Add physx libraries to target
#### Windows only: Copy the Physx dll files to the simulation executable####

if (TARGET_BUILD_PLATFORM STREQUAL "windows")
    # References NvidiaBuildOptions.cmake to figure out if system is 32/64 bit
    IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
        SET(LIBPATH_SUFFIX "64")
    ELSE()
        SET(LIBPATH_SUFFIX "32")
    ENDIF()
    GetPlatformBinName(PLATFORM_BIN_NAME ${LIBPATH_SUFFIX})
    set(PhysxOutputPath ${PX_OUTPUT_LIB_DIR}/bin/${PLATFORM_BIN_NAME}/)
    message("Physx Output Path: " ${PhysxOutputPath})

    # copy PhysX dll's to build dir. Happens on every build.
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${PhysxOutputPath}" "$<TARGET_FILE_DIR:Rui>/..")
endif()

message(NOTICE "SDL LIB DIR ${PHYSX_LIBRARIES}")

#add_library(${PROJECT_NAME} SHARED src/Rui/Core.h src/Rui/EntryPoint.h src/Rui/Application.cpp src/Rui/Application.h src/Rui/Log.cpp src/Rui/Log.h src/Events/Event.h src/Events/KeyEvent.h src/Events/MouseEvent.h src/Events/TickEvent.h src/Events/WindowEvent.h src/Events/Listener.h src/Rui.h)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/Rui/src ${CMAKE_SOURCE_DIR}/Rui/vendor/spdlog/include ${CMAKE_SOURCE_DIR}/Rui/vendor/glm ${CMAKE_SOURCE_DIR}/Rui/vendor/vma-hpp ${Vulkan_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${PHYSX_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES} SDL2main SDL2 glm::glm ${PHYSX_LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE RUI_PLATFORM_WINDOWS RUI_BUILD_DLL RUI_ENABLE_ASSERTS)
target_precompile_headers(${PROJECT_NAME} PUBLIC src/ruipch.h)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

if(MSVC)
    add_compile_options(
        $<$<CONFIG:>:/MT> #---------|
        $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MT> #--|
    )
endif()