# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "C:/Users/m3925/Documents/Pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(picoPioUartRx C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(picoPioUartRx picoPioUartRx.c )

#この１行を追加
pico_generate_pio_header(picoPioUartRx ${CMAKE_CURRENT_LIST_DIR}/picoPioUartRx.pio)

pico_set_program_name(picoPioUartRx "picoPioUartRx")
pico_set_program_version(picoPioUartRx "0.1")

pico_enable_stdio_uart(picoPioUartRx 0)
pico_enable_stdio_usb(picoPioUartRx 0)

# Add the standard library to the build
target_link_libraries(picoPioUartRx pico_stdlib)

# Add any user requested libraries
target_link_libraries(picoPioUartRx
        hardware_pio
        )

pico_add_extra_outputs(picoPioUartRx)

