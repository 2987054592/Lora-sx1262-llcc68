cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME LoraCopy)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
        common/LoraStruct.c
        common/LoraStruct.h
        App/FreertosTask/FreeRtosHandler.c
        App/FreertosTask/FreeRtosHandler.h
        App/UsartHandler/usartHandler.c
        App/UsartHandler/usartHandler.h
        App/Flash/bspFlash.c
        App/Flash/bspFlash.h
        App/Tcp/TcpDataProcess.c
        App/Tcp/TcpDataProcess.h
        common/LoraDevice.c
        common/LoraDevice.h)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    sx1262/src/driver_sx1262.c
    sx1262/interface/driver_sx1262_interface_template.c
    sx1262/example/driver_sx1262_lora.c
    llcc68/src/driver_llcc68.c
    llcc68/interface/driver_llcc68_interface_template.c
    llcc68/example/driver_llcc68_lora.c)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_CURRENT_SOURCE_DIR}/sx1262/src
    ${CMAKE_CURRENT_SOURCE_DIR}/sx1262/interface
    ${CMAKE_CURRENT_SOURCE_DIR}/sx1262/example
    ${CMAKE_CURRENT_SOURCE_DIR}/llcc68/src
    ${CMAKE_CURRENT_SOURCE_DIR}/llcc68/interface
    ${CMAKE_CURRENT_SOURCE_DIR}/llcc68/example)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)