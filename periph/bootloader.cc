#include "periph/bootloader.h"
#include "periph/usb.h"
#include "Core/Inc/tim.h"

#if defined(STM32F405xx)

extern "C" USBD_HandleTypeDef hUsbDeviceFS;
extern "C" TIM_HandleTypeDef        htim11;

#define BOOT_ADDR	0x1FFF0000	// stm32f405 system memory base addres, check on Ref Manual, Table 5. Flash module organization (STM32F40x and STM32F41x)
#define	MCU_IRQS	82u	// no. of NVIC IRQ inputs, check NVIC Feature on Ref Manual

struct boot_vectable_ {
    uint32_t Initial_SP;
    void (*Reset_Handler)(void);
};

#define BOOTVTAB	((struct boot_vectable_ *)BOOT_ADDR)

using namespace Project;


void periph::jumpToBootLoader() {
    //stop USB 
	USBD_Stop(&hUsbDeviceFS);
	USBD_DeInit(&hUsbDeviceFS);

    //remap memory 
	SYSCFG->MEMRMP = 0x01;

	/* Disable all interrupts */
	__disable_irq();

	/* Disable Systick timer (timebase) */
	HAL_TIM_Base_Stop(&htim11);
	__HAL_TIM_DISABLE_IT(&htim11, TIM_IT_UPDATE);
	__HAL_RCC_TIM11_CLK_DISABLE();

	SysTick->CTRL = 0;

	/* Set the clock to the default state */
	HAL_RCC_DeInit();

	/* Clear Interrupt Enable Register & Interrupt Pending Register */
    for (uint16_t i = 0; i < sizeof(NVIC->ICER) / sizeof(NVIC->ICER[0]); i++) {
		NVIC->ICER[i] = 0xFFFFFFFF;
		NVIC->ICPR[i] = 0xFFFFFFFF;
    }

	/* Re-enable all interrupts */
	__enable_irq();

	// Set the MSP
	__set_MSP(BOOTVTAB->Initial_SP);

	// Jump to app firmware
	BOOTVTAB->Reset_Handler();
}    

#endif