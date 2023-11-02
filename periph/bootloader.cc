#include "periph/bootloader.h"
#include "periph/usb.h"
#include "Core/Inc/tim.h"

using namespace Project;


// bootloader for DFU
#if defined(STM32F3)
#define BOOT_ADDR	0x1FFFD800
#endif

#if defined(STM32F4) || defined(STM32F2)
#define BOOT_ADDR	0x1FFF0000
#endif

#if defined(STM32F7)
#define BOOT_ADDR	0x1FF00000
#endif


#ifdef BOOT_ADDR

struct boot_vectable_ {
    uint32_t Initial_SP;
    void (*Reset_Handler)(void);
};

#define BOOTVTAB	((struct boot_vectable_ *)BOOT_ADDR)

#ifdef HAL_PCD_MODULE_ENABLED
extern "C" USBD_HandleTypeDef hUsbDeviceFS;
#endif

void periph::jumpToBootLoader() {
    // stop USB 
	#ifdef HAL_PCD_MODULE_ENABLED
	USBD_Stop(&hUsbDeviceFS);
	USBD_DeInit(&hUsbDeviceFS);
	#endif

    // remap memory ?
	SYSCFG->MEMRMP = 0x01;

	// disable interrupts
	__disable_irq();

	// disable systick tim base
	#ifdef PERIPH_SYSTICK_TIM_BASE_SOURCE
	if ((PERIPH_SYSTICK_TIM_BASE_SOURCE->CCER & (TIM_CCER_CCxE_MASK | TIM_CCER_CCxNE_MASK)) == 0UL)
		PERIPH_SYSTICK_TIM_BASE_SOURCE->CR1 &= ~(TIM_CR1_CEN); 
	
	PERIPH_SYSTICK_TIM_BASE_SOURCE->DIER &= ~TIM_IT_UPDATE;
	#endif

	SysTick->CTRL = 0;

	// deinit clock
	HAL_RCC_DeInit();

	// clear interrupt enable register and pending register
    for (uint16_t i = 0; i < sizeof(NVIC->ICER) / sizeof(NVIC->ICER[0]); i++) {
		NVIC->ICER[i] = 0xFFFFFFFF;
		NVIC->ICPR[i] = 0xFFFFFFFF;
    }

	// reenable interrupt (?)
	__enable_irq();

	// set the stack pointer
	__set_MSP(BOOTVTAB->Initial_SP);

	// jump to bootloader
	BOOTVTAB->Reset_Handler();
}    

#else

void periph::jumpToBootLoader() {}

#endif