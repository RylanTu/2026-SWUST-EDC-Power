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
set(CMAKE_PROJECT_NAME STM32_TLC594)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
set(MY_USER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/lcd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/Display.c
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/FunctionSet.c
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/key.c
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/My_KEY.c
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/LED.c
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/FAN.c
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/AT24CXX.c
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/MyAdc_Apply.c
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/My_Init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/My_Printf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/My_PWM.c
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/My_Timer2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/Public.c
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/System.c
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User/CallBack.c
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${MY_USER_SOURCES}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_CURRENT_SOURCE_DIR}/My_User
)

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
