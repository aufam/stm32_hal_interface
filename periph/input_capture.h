#ifndef PERIPH_INPUT_CAPTURE_H
#define PERIPH_INPUT_CAPTURE_H

#include "main.h"
#ifdef HAL_TIM_MODULE_ENABLED

#include "periph/config.h"
#include "Core/Inc/tim.h"
#include "etl/function.h"

namespace Project::periph {

    /// input capture
    /// @note requirements: TIMx input capture mode, TIMx global interrupt
    struct InputCapture {
        using Callback = etl::Function<void(), void*>;
        inline static detail::UniqueInstances<InputCapture*, 16> Instances;

        TIM_HandleTypeDef& htim;        ///< TIM handler configured by cubeMX
        uint32_t channel;               ///< TIM_CHANNEL_x
        Callback callback = {};         ///< capture callback function

        InputCapture(const InputCapture&) = delete;             ///< disable copy constructor
        InputCapture& operator=(const InputCapture&) = delete;  ///< disable copy assignment

        /// start input capture and register this instance
        void init() { 
            #ifdef PERIPH_INPUT_CAPTURE_USE_IT
            HAL_TIM_IC_Start_IT(&htim, channel); 
            #endif 
            #ifdef PERIPH_INPUT_CAPTURE_USE_DMA
            HAL_TIM_IC_Start_DMA(&htim, channel); 
            #endif
            Instances.push(this);
        }

        /// stop input capture and unregister this instance
        void deinit() { 
            #ifdef PERIPH_INPUT_CAPTURE_USE_IT
            HAL_TIM_IC_Stop_IT(&htim, channel); 
            #endif 
            #ifdef PERIPH_INPUT_CAPTURE_USE_DMA
            HAL_TIM_IC_Stop_DMA(&htim, channel); 
            #endif
            Instances.pop(this);
        }

        /// enable interrupt
        void enable() {
            switch (channel) {
                case TIM_CHANNEL_1: __HAL_TIM_ENABLE_IT(&htim, TIM_IT_CC1); break;
                case TIM_CHANNEL_2: __HAL_TIM_ENABLE_IT(&htim, TIM_IT_CC2); break;
                case TIM_CHANNEL_3: __HAL_TIM_ENABLE_IT(&htim, TIM_IT_CC3); break;
                case TIM_CHANNEL_4: __HAL_TIM_ENABLE_IT(&htim, TIM_IT_CC4); break;
                default: break;
            }
        }

        /// disable interrupt
        void disable() {
            switch (channel) {
                case TIM_CHANNEL_1: __HAL_TIM_DISABLE_IT(&htim, TIM_IT_CC1); break;
                case TIM_CHANNEL_2: __HAL_TIM_DISABLE_IT(&htim, TIM_IT_CC2); break;
                case TIM_CHANNEL_3: __HAL_TIM_DISABLE_IT(&htim, TIM_IT_CC3); break;
                case TIM_CHANNEL_4: __HAL_TIM_DISABLE_IT(&htim, TIM_IT_CC4); break;
                default: break;
            }
        }

        /// set capture polarity
        /// @param polarity TIM_INPUTCHANNELPOLARITY_xxx
        void setPolarity(uint32_t polarity) { __HAL_TIM_SET_CAPTUREPOLARITY(&htim, channel, polarity); }

        /// set counter TIMx->CNT
        /// @param value desired value
        void setCounter(uint32_t value) { __HAL_TIM_SET_COUNTER(&htim, value); }

        /// read captured value TIMx->CCRy
        uint32_t read() { return HAL_TIM_ReadCapturedValue(&htim, channel); }
    };
}

#endif // HAL_TIM_MODULE_ENABLED
#endif // PERIPH_INPUT_CAPTURE_H
