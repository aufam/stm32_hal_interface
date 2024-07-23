#ifndef PERIPH_BOOTLOADER_H
#define PERIPH_BOOTLOADER_H

#include <cstddef>

namespace Project::periph {
    #if defined(STM32F3)
    inline constexpr size_t DefaultBootAddress = 0x1FFFD800;
    #endif

    #if defined(STM32F4) || defined(STM32F2)
    inline constexpr size_t DefaultBootAddress = 0x1FFF0000;
    #endif

    #if defined(STM32F7)
    inline constexpr size_t DefaultBootAddress = 0x1FF00000;
    #endif

    void jumpToBootLoader(size_t addr);
}

#endif