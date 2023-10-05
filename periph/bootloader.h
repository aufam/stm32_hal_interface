#ifndef PERIPH_BOOTLOADER_H
#define PERIPH_BOOTLOADER_H

#if defined(STM32F405xx)
namespace Project::periph {
    void jumpToBootLoader();
}
#endif
#endif