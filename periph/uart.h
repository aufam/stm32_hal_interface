#ifndef PERIPH_UART_H
#define PERIPH_UART_H

#include "main.h"
#ifdef HAL_UART_MODULE_ENABLED

#include "periph/config.h"
#include "Core/Inc/usart.h"
#include "etl/array.h"
#include "etl/function.h"
#include "etl/getter_setter.h"
#include "etl/string.h"
#include "etl/time.h"

namespace Project::periph {

    /// UART peripheral class.
    /// @note requirements: global interrupt, rx DMA
    struct UART {
        using RxCallback = etl::Function<void(const uint8_t*, size_t), void*>;  ///< rx callback function class
        using TxCallback = etl::Function<void(), void*>;                        ///< tx callback function class
        using Buffer = etl::Array<uint8_t, PERIPH_UART_RX_BUFFER_SIZE>;         ///< UART rx buffer class

        template <typename T>
        using GetterSetter = etl::GetterSetter<T, etl::Function<T(), const UART*>, etl::Function<void(T), const UART*>>;
        
        static detail::UniqueInstances<UART*, 16> Instances;

        UART_HandleTypeDef &huart;                              ///< UART handler configured by cubeMX
        detail::UniqueInstances<RxCallback, 16> rxCallbackList = {}; ///< rx callback function
        detail::UniqueInstances<TxCallback, 16> txCallbackList = {}; ///< tx callback function
        Buffer rxBuffer = {};                                   ///< rx buffer

        UART(const UART&) = delete;             ///< disable copy constructor
        UART& operator=(const UART&) = delete;  ///< disable copy assignment

        /// start receive to idle
        void init() {
            #ifdef PERIPH_UART_RECEIVE_USE_IT
            HAL_UARTEx_ReceiveToIdle_IT(&huart, rxBuffer.data(), rxBuffer.len());
            #endif
            #ifdef PERIPH_UART_RECEIVE_USE_DMA
            HAL_UARTEx_ReceiveToIdle_DMA(&huart, rxBuffer.data(), rxBuffer.len());
            #endif
            __HAL_DMA_DISABLE_IT(huart.hdmarx, DMA_IT_HT);
            Instances.push(this);
        }

        struct InitArgs { uint32_t baudrate; RxCallback rxCallback = {}; TxCallback txCallback = {}; };

        void init(InitArgs args) {
            baudrate = args.baudrate;
            rxCallbackList.push(args.rxCallback);
            txCallbackList.push(args.txCallback);
            init();
        }

        /// disable receive
        void deinit() { 
            if (rxCallbackList.isEmpty() && txCallbackList.isEmpty()) {
                #ifdef PERIPH_UART_RECEIVE_USE_IT
                HAL_UART_Abort_IT(&huart); 
                #endif
                #ifdef PERIPH_UART_RECEIVE_USE_DMA
                HAL_UART_DMAStop(&huart); 
                #endif
                Instances.pop(this);
            }
        }

        struct DeinitArgs { RxCallback rxCallback = {}; TxCallback txCallback = {}; };

        void deinit(DeinitArgs args) {
            rxCallbackList.pop(args.rxCallback); 
            txCallbackList.pop(args.txCallback); 
            deinit();
        }

        struct TransmitBlockingTimeoutArgs { etl::Time timeout; };

        /// UART transmit blocking
        /// @param args
        ///     - .buf data buffer
        ///     - .len buffer length
        ///     - .timeout default = time::infinite
        /// @retval HAL_StatusTypeDef (see stm32fXxx_hal_def.h)
        int transmitBlocking(const void *buf, size_t len, TransmitBlockingTimeoutArgs args = {.timeout=etl::time::infinite}) {
            while (huart.gState != HAL_UART_STATE_READY);
            return HAL_UART_Transmit(&huart, (uint8_t *) buf, len, args.timeout.tick);
        }

        /// UART transmit non blocking
        /// @param args
        ///     - .buf data buffer
        ///     - .len buffer length
        /// @retval HAL_StatusTypeDef (see stm32fXxx_hal_def.h)
        int transmit(const void *buf, size_t len) { 
            #ifdef PERIPH_UART_TRANSMIT_USE_IT
            return HAL_UART_Transmit_IT(&huart, (uint8_t *) buf, len); 
            #endif
            #ifdef PERIPH_UART_TRANSMIT_USE_DMA
            return HAL_UART_Transmit_DMA(&huart, (uint8_t *) buf, len); 
            #endif
        }

        /// get and set baudrate
        const GetterSetter<uint32_t> baudrate = {
            {+[] (const UART* self) { return self->huart.Init.BaudRate; }, this},
            {+[] (const UART* self, uint32_t value) { self->huart.Init.BaudRate = value; HAL_UART_Init(&self->huart); }, this}
        };

        /// write blocking operator for etl::string
        template <size_t N>
        UART& operator<<(const etl::String<N>& str) { 
            transmitBlocking(str.data(), str.len()); 
            return *this; 
        }

        /// write blocking operator for traditional string
        UART& operator<<(const char *str) { 
            transmitBlocking(str, strlen(str)); 
            return *this; 
        }
    };
} // namespace Project

#endif // HAL_TIM_MODULE_ENABLED
#endif // PERIPH_UART_H