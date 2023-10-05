#ifndef PERIPH_UART_H
#define PERIPH_UART_H

#include "main.h"
#ifdef HAL_UART_MODULE_ENABLED

#include "periph/config.h"
#include "Core/Inc/usart.h"
#include "etl/array.h"
#include "etl/function.h"
#include "etl/string.h"
#include "etl/time.h"

namespace Project::periph {

    /// UART peripheral class.
    /// @note requirements: global interrupt, rx DMA
    struct UART {
        using RxCallback = etl::Function<void(const uint8_t*, size_t), void*>;  ///< rx callback function class
        using TxCallback = etl::Function<void(), void*>;                        ///< tx callback function class
        using Buffer = etl::Array<uint8_t, 64>;                                 ///< UART rx buffer class
        inline static detail::UniqueInstances<UART*, 16> Instances;

        UART_HandleTypeDef &huart;  ///< UART handler configured by cubeMX
        RxCallback rxCallback = {}; ///< rx callback function
        TxCallback txCallback = {}; ///< tx callback function
        Buffer rxBuffer = {};       ///< rx buffer

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
        }

        /// disable receive
        void deinit() { 
            #ifdef PERIPH_UART_RECEIVE_USE_IT
            HAL_UART_Abort_IT(&huart); 
            #endif
            #ifdef PERIPH_UART_RECEIVE_USE_DMA
            HAL_UART_DMAStop(&huart); 
            #endif
        }

        struct TransmitBlockingArgs { const void *buf; size_t len; etl::Time timeout = etl::time::infinite; };

        /// UART transmit blocking
        /// @param args
        ///     - .buf data buffer
        ///     - .len buffer length
        ///     - .timeout default = time::infinite
        /// @retval HAL_StatusTypeDef (see stm32fXxx_hal_def.h)
        int transmitBlocking(TransmitBlockingArgs args) {
            while (huart.gState != HAL_UART_STATE_READY);
            return HAL_UART_Transmit(&huart, (uint8_t *) args.buf, args.len, args.timeout.tick);
        }

        struct TransmitArgs { const void *buf; size_t len; };

        /// UART transmit non blocking
        /// @param args
        ///     - .buf data buffer
        ///     - .len buffer length
        /// @retval HAL_StatusTypeDef (see stm32fXxx_hal_def.h)
        int transmit(TransmitArgs args) { 
            #ifdef PERIPH_UART_TRANSMIT_USE_IT
            return HAL_UART_Transmit_IT(&huart, (uint8_t *) args.buf, args.len); 
            #endif
            #ifdef PERIPH_UART_TRANSMIT_USE_DMA
            return HAL_UART_Transmit_DMA(&huart, (uint8_t *) args.buf, args.len); 
            #endif
        }

        /// set UART baud rate
        /// @param baud desired baud rate
        /// @retval HAL_StatusTypeDef (see stm32fXxx_hal_def.h)
        int setBaudRate(uint32_t baud) { huart.Init.BaudRate = baud; return HAL_UART_Init(&huart); }

        /// get UART baud rate
        /// @retval baud rate
        [[nodiscard]] 
        uint32_t getBaudRate() const { return huart.Init.BaudRate; }

        /// write blocking operator for etl::string
        template <size_t N>
        UART& operator<<(const etl::String<N>& str) { 
            transmitBlocking({.buf=str.data(), .len=str.len()}); 
            return *this; 
        }

        /// write blocking operator for traditional string
        UART& operator<<(const char *str) { 
            transmitBlocking({.buf=str, .len=strlen(str)}); 
            return *this; 
        }
    };
} // namespace Project

#endif // HAL_TIM_MODULE_ENABLED
#endif // PERIPH_UART_H