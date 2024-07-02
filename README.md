# STM32 HAL Interface
C++ interface for STM32 HAL Driver

## Requirements
* Arm GNU toolchain
* cmake minimum version 3.10
* STM32CubeMx generated code

## How to use
* Clone this repo to your STM32 project folder. For example:
```bash
git clone https://github.com/aufam/stm32_hal_interface.git your_project_path/Middlewares/Third_Party/stm32_hal_interface
```
* Or, if Git is configured, you can add this repo as a submodule:
```bash
git submodule add https://github.com/aufam/stm32_hal_interface.git <your_project_path>/Middlewares/Third_Party/stm32_hal_interface
git submodule update --init --recursive
```
* Add these line to your project CMakeLists.txt:
```cmake
add_subdirectory(Middlewares/Third_Party/stm32_hal_interface)
target_link_libraries(${PROJECT_NAME}.elf periph)
```
