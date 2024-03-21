#ifndef PERIPH_INPUT_CAPTURE_H
#define PERIPH_INPUT_CAPTURE_H

#include "main.h"
#ifdef HAL_TIM_MODULE_ENABLED

#include "periph/config.h"
#include "Core/Inc/tim.h"
#include "etl/getter_setter.h"
#include "etl/async.h"

namespace Project::periph { struct InputCapture; }

/// input capture
/// @note requirements: TIMx input capture mode, TIMx global interrupt
struct Project::periph::InputCapture {
    using Callback = etl::Function<void(), void*>;

    template <typename T>
    using GetterSetter = etl::GetterSetter<T, etl::Function<T(), const InputCapture*>, etl::Function<void(T), const InputCapture*>>;
    
    static detail::UniqueInstances<InputCapture*, 16> Instances;

    TIM_HandleTypeDef& htim;        ///< TIM handler configured by cubeMX
    uint32_t channel;               ///< TIM_CHANNEL_x
    etl::Queue<uint32_t, 1> que = {};

    InputCapture(const InputCapture&) = delete;             ///< disable copy constructor
    InputCapture& operator=(const InputCapture&) = delete;  ///< disable copy assignment

    /// start input capture and register this instance
    void init(
        #ifdef PERIPH_PWM_USE_DMA
        uint32_t* dmaBuffer, uint16_t len
        #endif
    ) { 
        #ifdef PERIPH_INPUT_CAPTURE_USE_IT
        HAL_TIM_IC_Start_IT(&htim, channel); 
        #endif 
        #ifdef PERIPH_INPUT_CAPTURE_USE_DMA
        HAL_TIM_IC_Start_DMA(&htim, channel, dmaBuffer, len); 
        #endif
        que.init();
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

    /// get and set polarity, TIM_INPUTCHANNELPOLARITY_XXX
    const GetterSetter<uint32_t> polarity = {
        {+[] (const InputCapture* self) -> uint32_t  { 
            switch (self->channel) {
                case TIM_CHANNEL_1: return self->htim.Instance->CCER & (TIM_CCER_CC1P | TIM_CCER_CC1NP);
                case TIM_CHANNEL_2: return self->htim.Instance->CCER & (TIM_CCER_CC2P | TIM_CCER_CC2NP);
                case TIM_CHANNEL_3: return self->htim.Instance->CCER & (TIM_CCER_CC3P | TIM_CCER_CC3NP);
                case TIM_CHANNEL_4: return self->htim.Instance->CCER & (TIM_CCER_CC4P 
                #if defined(TIM_CCER_CC4NP)
                | TIM_CCER_CC4NP
                #endif
                );
                default: return 0;
            }
        }, this},
        {+[] (const InputCapture* self, uint32_t value) { 
            __HAL_TIM_SET_CAPTUREPOLARITY(&self->htim, self->channel, value); 
        }, this}
    };

    etl::Future<uint32_t> read() {
        return [this] (etl::Time timeout) -> etl::Result<uint32_t, osStatus_t> {
            que.clear();
            return que.pop().wait(timeout);
        };
    }
};

#endif // HAL_TIM_MODULE_ENABLED
#endif // PERIPH_INPUT_CAPTURE_H
