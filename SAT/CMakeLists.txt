cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(fortran_lora LANGUAGES C CXX ASM Fortran)

pico_sdk_init()

set(BARE_METAL_FLAGS "-mcpu=cortex-m0plus -mthumb -fno-stack-protector -fno-pic -fno-pie -fno-exceptions -fno-rtti")
set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${BARE_METAL_FLAGS}")

add_executable(fortran_lora
    main.f90
    lora_baremetal.c
)

target_link_libraries(fortran_lora 
    pico_stdlib 
    hardware_spi 
    hardware_gpio
    gfortran
    m
)

target_link_options(fortran_lora PRIVATE 
    "--specs=nosys.specs"
    "-Wl,--gc-sections"
    "-Wl,--allow-multiple-definition"  # Resolves the overlap
    "-Wl,--undefined=__sync_bool_compare_and_swap_4" # Ensures our mock is used
)
pico_enable_stdio_usb(fortran_lora 1)
pico_enable_stdio_uart(fortran_lora 0)

pico_add_extra_outputs(fortran_lora)