#ifndef PERIPH_INPUT_CAPTURE_H
#define PERIPH_INPUT_CAPTURE_H

#include "main.h"
#ifdef HAL_TIM_MODULE_ENABLED

#include "periph/config.h"
#include "Core/Inc/tim.h"
#include "etl/getter_setter.h"
#include "etl/function.h"

namespace Project::periph {

    /// input capture
    /// @note requirements: TIMx input capture mode, TIMx global interrupt
    struct InputCapture {
        using Callback = etl::Function<void(), void*>;

        template <typename T>
        using GetterSetter = etl::GetterSetter<T, etl::Function<T(), const InputCapture*>, etl::Function<void(T), const InputCapture*>>;
        
        static detail::UniqueInstances<InputCapture*, 16> Instances;

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
        
        struct InitArgs { Callback callback; bool enableNow = false; };
        void init(InitArgs args) {
            if (args.callback) callback = args.callback;
            init();
            if (args.enableNow) enable();
        }

        /// stop input capture and unregister this instance
        void deinit() { 
            callback = Callback();
            #ifdef PERIPH_INPUT_CAPTURE_USE_IT
            HAL_TIM_IC_Stop_IT(&htim, channel); 
            #endif 
            #ifdef PERIPH_INPUT_CAPTURE_USE_DMA
            HAL_TIM_IC_Stop_DMA(&htim, channel); 
            #endif
            Instances.pop(this);
        }

        /// enable interrupt
        void enable() const {
            switch (channel) {
                case TIM_CHANNEL_1: __HAL_TIM_ENABLE_IT(&htim, TIM_IT_CC1); break;
                case TIM_CHANNEL_2: __HAL_TIM_ENABLE_IT(&htim, TIM_IT_CC2); break;
                case TIM_CHANNEL_3: __HAL_TIM_ENABLE_IT(&htim, TIM_IT_CC3); break;
                case TIM_CHANNEL_4: __HAL_TIM_ENABLE_IT(&htim, TIM_IT_CC4); break;
                default: break;
            }
        }

        /// disable interrupt
        void disable() const {
            switch (channel) {
                case TIM_CHANNEL_1: __HAL_TIM_DISABLE_IT(&htim, TIM_IT_CC1); break;
                case TIM_CHANNEL_2: __HAL_TIM_DISABLE_IT(&htim, TIM_IT_CC2); break;
                case TIM_CHANNEL_3: __HAL_TIM_DISABLE_IT(&htim, TIM_IT_CC3); break;
                case TIM_CHANNEL_4: __HAL_TIM_DISABLE_IT(&htim, TIM_IT_CC4); break;
                default: break;
            }
        }

        /// get and set polarity, TIM_INPUTCHANNELPOLARITY_XXX
        const GetterSetter<uint32_t> polarity = {
            {+[] (const InputCapture* self) -> uint32_t  { 
                switch (self->channel) {
                    case TIM_CHANNEL_1: return self->htim.Instance->CCER & (TIM_CCER_CC1P | TIM_CCER_CC1NP);
                    case TIM_CHANNEL_2: return self->htim.Instance->CCER & (TIM_CCER_CC2P | TIM_CCER_CC2NP);
                    case TIM_CHANNEL_3: return self->htim.Instance->CCER & (TIM_CCER_CC3P | TIM_CCER_CC3NP);
                    case TIM_CHANNEL_4: return self->htim.Instance->CCER & (TIM_CCER_CC4P | TIM_CCER_CC4NP);
                    default: return 0;
                }
            }, this},
            {+[] (const InputCapture* self, uint32_t value) { 
                __HAL_TIM_SET_CAPTUREPOLARITY(&self->htim, self->channel, value); 
            }, this}
        };

        /// TIMx->CNT
        const GetterSetter<uint32_t> counter = {
            {+[] (const InputCapture* self) { return self->htim.Instance->CNT; }, this},
            {+[] (const InputCapture* self, uint32_t value) { self->htim.Instance->CNT = value; }, this}
        };

        /// read captured value TIMx->CCR
        uint32_t read() const { return HAL_TIM_ReadCapturedValue(&htim, channel); }
    };
}

#endif // HAL_TIM_MODULE_ENABLED
#endif // PERIPH_INPUT_CAPTURE_H
