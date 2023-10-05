#ifndef PERIPH_EXTI_H
#define PERIPH_EXTI_H

#include "main.h"
#ifdef HAL_EXTI_MODULE_ENABLED

#include "periph/config.h"
#include "etl/function.h"
#include "etl/time.h"

namespace Project::periph {

    /// external interrupt class
    /// @note requirements: configured external interrupt by CubeMX
    struct Exti {
        using Callback = etl::Function<void(), void*>;
        static constexpr etl::Time debounceDelayDefault = etl::time::milliseconds(250);
        inline static detail::UniqueInstances<Exti*, 16> Instances;

        uint16_t pin;                                    ///< GPIO_PIN_x
        Callback callback = {};                          ///< callback functions for the pin
        etl::Time debounceDelay = debounceDelayDefault;  ///< debounce delay filter
        uint32_t counter = 0;                            ///< counts how many times the pin has been triggered

        Exti(const Exti&) = delete;             ///< disable copy constructor
        Exti& operator=(const Exti&) = delete;  ///< disable copy assignment

        // register this instance
        void init() { Instances.push(this); }

        // unregister this instance
        void deinit() { Instances.pop(this); }
    };

} // namespace Project

#endif // HAL_EXTI_MODULE_ENABLED
#endif // PERIPH_EXTI_H