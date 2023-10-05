#ifndef PERIPH_GPIO_H
#define PERIPH_GPIO_H

#include "main.h"
#ifdef HAL_GPIO_MODULE_ENABLED

#include "etl/time.h"

#define GPIO_ACTIVE_HIGH Project::periph::GPIO::activeHigh
#define GPIO_ACTIVE_LOW  Project::periph::GPIO::activeLow

namespace Project::periph {

    struct GPIO {
        enum { activeLow, activeHigh };

        GPIO_TypeDef *port = nullptr;   ///< GPIOx
        uint16_t pin = 0;               ///< GPIO_PIN_x
        bool activeMode = false;        ///< activeLow or activeHigh

        struct InitArgs { uint32_t mode; uint32_t pull = GPIO_NOPULL; uint32_t speed = GPIO_SPEED_FREQ_LOW; };

        /// hal init GPIO and turn off
        /// @param args
        ///     - .mode GPIO_MODE_xxx
        ///     - .pull @ref GPIO_NOPULL (default), @ref GPIO_PULLUP, @ref GPIO_PULLDOWN
        ///     - .speed @ref GPIO_SPEED_FREQ_LOW (default), @ref GPIO_SPEED_FREQ_MEDIUM, @ref GPIO_SPEED_FREQ_HIGH
        void init(InitArgs args) const {
            // enable clock
            if (port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
            if (port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
            if (port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
            #if defined(GPIOD)
                if (port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
            #endif
            #if defined(GPIOE)
                if (port == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
            #endif
            #if defined(GPIOF)
                if (port == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
            #endif
            #if defined(GPIOG)
                if (port == GPIOG) __HAL_RCC_GPIOG_CLK_ENABLE();
            #endif
            #if defined(GPIOH)
                if (port == GPIOH) __HAL_RCC_GPIOH_CLK_ENABLE();
            #endif
            #if defined(GPIOI)
                if (port == GPIOI) __HAL_RCC_GPIOI_CLK_ENABLE();
            #endif

            // hal init
            GPIO_InitTypeDef gpioInitStruct = { 
                .Pin = pin,
                .Mode = args.mode,
                .Pull = args.pull,
                .Speed = args.speed,
                .Alternate = 0
            };
            HAL_GPIO_Init(port, &gpioInitStruct);

            // turn off if mode is output
            if (args.mode == GPIO_MODE_OUTPUT_OD || args.mode == GPIO_MODE_OUTPUT_PP)
                off();
        }

        /// return true if this object is valid
        explicit operator bool() { return bool(port); }

        /// write pin high (true) or low (false)
        void write(bool highLow) const { HAL_GPIO_WritePin(port, pin, highLow ? GPIO_PIN_SET : GPIO_PIN_RESET); }

        /// toggle pin
        void toggle() const { HAL_GPIO_TogglePin(port, pin); }

        /// read pin
        /// @retval high (true) or low (false)
        [[nodiscard]] 
        bool read() const { return HAL_GPIO_ReadPin(port, pin); }

        struct OnOffArgs { etl::Time sleepFor; };
        inline static constexpr OnOffArgs OnOffDefault = {.sleepFor=etl::time::immediate};

        /// turn on
        /// @param args
        ///     - .sleepFor sleep for a while. default = time::immediate
        void on(OnOffArgs args = OnOffDefault) const {
            write(activeMode);
            etl::time::sleep(args.sleepFor);
        }

        /// turn off
        /// @param args
        ///     - .sleepFor sleep for a while. default = time::immediate
        void off(OnOffArgs args = OnOffDefault) const {
            write(!activeMode);
            etl::time::sleep(args.sleepFor);
        }

        [[nodiscard]] 
        bool isOn() const { return !(read() ^ activeMode); }
        
        [[nodiscard]] 
        bool isOff() const { return (read() ^ activeMode); }

    };

} // periph

#endif
#endif // PERIPH_GPIO_H
