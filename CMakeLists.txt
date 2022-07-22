cmake_minimum_required(VERSION 3.12)

set(PIMORONI_PICO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/pimoroni-pico")
include(pimoroni_pico_import.cmake)
include(pico-sdk/pico_sdk_init.cmake)

project(picow-wlan-setup-webinterface C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPICO_USE_MALLOC_MUTEX=1")
add_subdirectory(littlefs-lib)

add_executable(${PROJECT_NAME}
    main.cpp
    display.cpp
    setupwebinterface.cpp
    wlanscanner.cpp
    QR-Code-generator/c/qrcodegen.c
    pico-sdk/lib/tinyusb/lib/networking/dhserver.c
    pico-sdk/lib/tinyusb/lib/networking/dnserver.c
)

include(common/pimoroni_i2c)
include(common/pimoroni_bus)
include(drivers/button/button)
include(libraries/pico_explorer/pico_explorer)
include(libraries/pico_display/pico_display)
include(libraries/hershey_fonts/hershey_fonts)
include(libraries/bitmap_fonts/bitmap_fonts)

target_link_libraries(${PROJECT_NAME}
    pico_cyw43_arch_lwip_poll pico_lwip_http        
    pico_stdlib pico_explorer pico_graphics st7789 button
    littlefs-lib
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(${PROJECT_NAME})
