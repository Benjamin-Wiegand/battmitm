# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(BattMITM_2.0 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(BattMITM_2.0
        main.c 
        smbus.c
        static_queue.c 
        mitm.c 
        status.c 
        display.c 
        font.c 
        battery.c 
        button.c
        defused/gui.c
)

pico_set_program_name(BattMITM_2.0 "BattMITM_2.0")
pico_set_program_version(BattMITM_2.0 "0.1")

# Add the standard library to the build
target_link_libraries(BattMITM_2.0
        pico_stdlib
        hardware_i2c
        pico_i2c_slave
        hardware_pwm
        hardware_spi
        pico_rand
        pico_multicore
)

# Add the standard include files to the build
target_include_directories(BattMITM_2.0 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(BattMITM_2.0)

# enable USB serial
pico_enable_stdio_usb(BattMITM_2.0 1)
