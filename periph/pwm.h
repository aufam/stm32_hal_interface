#ifndef PERIPH_PWM_H
#define PERIPH_PWM_H


#include "main.h"
#ifdef HAL_TIM_MODULE_ENABLED

#include "periph/config.h"

#include "Core/Inc/tim.h"
#include "etl/function.h"
#include "etl/getter_setter.h"

namespace Project::periph {

    /// PWM generation.
    /// @note requires: TIMx PWM generation mode, TIMx global interrupt
    struct PWM {
        using Callback = etl::Function<void(), void*>; ///< callback function class

        template <typename T>
        using GetterSetter = etl::GetterSetter<T, etl::Function<T(), const PWM*>, etl::Function<void(T), const PWM*>>;
        
        static detail::UniqueInstances<PWM*, 16> Instances;

        TIM_HandleTypeDef &htim;        ///< tim handler generated by cubeMX
        uint32_t channel;               ///< TIM_CHANNEL_x
        bool hasInverseChannel = false;
        bool useOutputCompare = false;
        Callback fullCallback = {};
        Callback halfCallback = {};

        PWM(const PWM&) = delete;               ///< disable copy constructor
        PWM& operator=(const PWM&) = delete;    ///< disable copy assignment

        /// register this instance
        void init() {
            Instances.push(this);
        }

        struct InitArgs { uint32_t prescaler, period, pulse; Callback fullCallback = {}, halfCallback = {}; bool startNow = false; };

        /// setup prescaler, period, and pulse, and register this instance
        /// @param args
        ///     - .prescaler set TIMx->PSC
        ///     - .period set TIMx->ARR
        ///     - .pulse set TIMx->CCRchannel
        ///     - .pulse set TIMx->CCRchannel
        void init(InitArgs args) {
            prescaler = args.prescaler;
            period = args.period;
            pulse = args.pulse;
            if (args.fullCallback) fullCallback = args.fullCallback;
            if (args.halfCallback) halfCallback = args.halfCallback;
            init();
            if (args.startNow) start();
        }

        /// stop pwm and unregister this instance
        void deinit() { 
            fullCallback = Callback();
            halfCallback = Callback();
            stop(); 
            Instances.pop(this);
        }

        /// start pwm interrupt
        void start(
            #ifdef PERIPH_PWM_USE_DMA
            uint32_t* dmaBuffer, uint16_t len,
            uint32_t* dmaBufferInverseChannel = nullptr, uint16_t lenInverseChannel = 0
            #endif
        ) { 
            #ifdef PERIPH_PWM_USE_IT
            if (useOutputCompare) {
                HAL_TIM_OC_Start_IT(&htim, channel);
                if (hasInverseChannel)
                    HAL_TIMEx_OCN_Start_IT(&htim, channel);
            } else {
                HAL_TIM_PWM_Start_IT(&htim, channel);
                if (hasInverseChannel)
                    HAL_TIMEx_PWMN_Start_IT(&htim, channel);
            }
            #endif

            #ifdef PERIPH_PWM_USE_DMA
            if (useOutputCompare) {
                HAL_TIM_OC_Start_DMA(&htim, channel, dmaBuffer, len);
                if (hasInverseChannel)
                    HAL_TIMEx_OCN_Start_DMA(&htim, channel, dmaBufferInverseChannel, lenInverseChannel);
            } else {
                HAL_TIM_PWM_Start_DMA(&htim, channel, dmaBuffer, len);
                if (hasInverseChannel)
                    HAL_TIMEx_PWMN_Start_DMA(&htim, channel, dmaBufferInverseChannel, lenInverseChannel);
            }
            #endif
        }

        /// stop pwm interrupt
        void stop() { 
            #ifdef PERIPH_PWM_USE_IT
            if (useOutputCompare) {
                HAL_TIM_OC_Stop_IT(&htim, channel);
                if (hasInverseChannel)
                    HAL_TIMEx_OCN_Stop_IT(&htim, channel);
            } else {
                HAL_TIM_PWM_Stop_IT(&htim, channel);
                if (hasInverseChannel)
                    HAL_TIMEx_PWMN_Stop_IT(&htim, channel);
            }
            #endif

            #ifdef PERIPH_PWM_USE_DMA
            if (useOutputCompare) {
                HAL_TIM_OC_Stop_DMA(&htim, channel);
                if (hasInverseChannel)
                    HAL_TIMEx_OCN_Stop_DMA(&htim, channel);
            } else {
                HAL_TIM_PWM_Stop_DMA(&htim, channel);
                if (hasInverseChannel)
                    HAL_TIMEx_PWMN_Stop_DMA(&htim, channel);
            }
            #endif
        }

        /// TIMx->PSC
        const GetterSetter<uint32_t> prescaler = {
            { +[] (const PWM* self) { return self->htim.Instance->PSC; }, this},
            { +[] (const PWM* self, uint32_t value) { self->htim.Instance->PSC = value; }, this}
        };

        /// TIMx->ARR
        const GetterSetter<uint32_t> period = {
            { +[] (const PWM* self) { return self->htim.Instance->ARR; }, this},
            { +[] (const PWM* self, uint32_t value) { self->htim.Instance->ARR = value; }, this}
        };

        /// TIMx->CCR
        const GetterSetter<uint32_t> pulse = {
            { +[] (const PWM* self) { 
                switch (self->channel) {
                    case TIM_CHANNEL_1: return self->htim.Instance->CCR1;
                    case TIM_CHANNEL_2: return self->htim.Instance->CCR2;
                    case TIM_CHANNEL_3: return self->htim.Instance->CCR3;
                    case TIM_CHANNEL_4: return self->htim.Instance->CCR4;
                    default: return 0ul;
                }
            }, this },
            { +[] (const PWM* self, uint32_t value) { 
                switch (self->channel) {
                    case TIM_CHANNEL_1: self->htim.Instance->CCR1 = value; break;
                    case TIM_CHANNEL_2: self->htim.Instance->CCR2 = value; break;
                    case TIM_CHANNEL_3: self->htim.Instance->CCR3 = value; break;
                    case TIM_CHANNEL_4: self->htim.Instance->CCR4 = value; break;
                    default: break;
                }
            }, this }
        };
    };
}

#endif // HAL_TIM_MODULE_ENABLED
#endif // PERIPH_PWM_H
