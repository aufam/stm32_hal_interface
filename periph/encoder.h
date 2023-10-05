#ifndef PERIPH_ENCODER_H
#define PERIPH_ENCODER_H

#include "main.h"
#ifdef HAL_TIM_MODULE_ENABLED

#include "periph/config.h"
#include "Core/Inc/tim.h"
#include "etl/function.h"

namespace Project::periph {

    /// rotary encoder using TIM
    /// @note requirements: TIMx encoder mode, TIMx global interrupt
    struct Encoder {
        using Callback = etl::Function<void(), void*>;
        inline static detail::UniqueInstances<Encoder*, 16> Instances;

        TIM_HandleTypeDef &htim;            ///< TIM handler configured by cubeMX
        int16_t value = 0;                  ///< current value
        Callback incrementCallback = {};    ///< increment callback
        Callback decrementCallback = {};    ///< decrement callback

        Encoder(const Encoder&) = delete;             ///< disable copy constructor
        Encoder& operator=(const Encoder&) = delete;  ///< disable copy assignment

        /// start encoder and register this instance
        void init() { 
            #ifdef PERIPH_ENCODER_USE_IT
            HAL_TIM_Encoder_Start_IT(&htim, TIM_CHANNEL_ALL); 
            #endif
            #ifdef PERIPH_ENCODER_USE_DMA
            HAL_TIM_Encoder_Start_DMA(&htim, TIM_CHANNEL_ALL); 
            #endif
            Instances.push(this);
        }

        /// stop encoder and unregister this instance
        void deinit() { 
            #ifdef PERIPH_ENCODER_USE_IT
            HAL_TIM_Encoder_Stop_IT(&htim, TIM_CHANNEL_ALL); 
            #endif
            #ifdef PERIPH_ENCODER_USE_DMA
            HAL_TIM_Encoder_Stop_DMA(&htim, TIM_CHANNEL_ALL); 
            #endif
            Instances.pop(this);
        }

        void inputCaptureCallback() {
            uint16_t counter = htim.Instance->CNT;
            int cnt = counter / 4;
            if (cnt > value) incrementCallback();
            if (cnt < value) decrementCallback();
            value = static_cast<int16_t> (cnt);
        }
    };
} // periph

#endif // HAL_TIM_MODULE_ENABLED
#endif // PERIPH_ENCODER_H