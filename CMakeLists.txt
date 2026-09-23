cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME SDK_Keyboard)

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
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        # Ваши файлы дисплея, клавиатуры, зуммера и шрифтов:
        Src/buzzer.c
        Src/fonts.c
        Src/kb.c
        Src/oled.c
        Src/pca9538.c
        Src/sdk_uart.c
        Src/tim.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        Inc
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
)

# Настройка скрипта линкера и параметров Cortex-M4
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32F407VGTx_FLASH.ld)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
        -T${LINKER_SCRIPT}
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        -specs=nano.specs
        -specs=nosys.specs
        -Wl,-Map=${CMAKE_PROJECT_NAME}.map,--cref
        -Wl,--gc-sections
)