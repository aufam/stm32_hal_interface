#ifndef PERIPH_I2C_H
#define PERIPH_I2C_H

#include "main.h"
#ifdef HAL_I2C_MODULE_ENABLED

#include "periph/config.h"
#include "Core/Inc/i2c.h"
#include "etl/function.h"
#include "etl/time.h"

namespace Project::periph {

    /// I2C peripheral class
    /// @note requirements: event interrupt, tx DMA/IT
    struct I2C {
        using Callback = etl::Function<void(), void*>; 
        inline static detail::UniqueInstances<I2C*, 16> Instances;

        I2C_HandleTypeDef &hi2c;    ///< I2C handler configured by cubeMX
        Callback txCallback = {};   ///< transmit complete callback function

        I2C(const I2C&) = delete;               ///< disable copy constructor
        I2C& operator=(const I2C&) = delete;    ///< disable copy assignment

        /// register this instance
        void init() { Instances.push(this); }

        /// unregister this instance
        void deinit() { Instances.pop(this); }

        struct ReadWriteBlockingArgs { 
            uint16_t deviceAddr, memAddr; 
            const uint8_t* buf; uint16_t len; 
            etl::Time timeout = etl::time::infinite; 
        };

        struct ReadWriteArgs { 
            uint16_t deviceAddr, memAddr; 
            const uint8_t* buf; uint16_t len; 
        };

        /// I2C write blocking
        /// @param args
        ///     - .deviceAddr device address
        ///     - .memAddr memory address
        ///     - .buf pointer to data buffer
        ///     - .len data length
        ///     - .timeout write timeout, default time::infinite
        /// @retval HAL_StatusTypeDef. see stm32fXxx_hal_def.h
        int writeBlocking(ReadWriteBlockingArgs args) {
            while (hi2c.State != HAL_I2C_STATE_READY);
            return HAL_I2C_Mem_Write(&hi2c, args.deviceAddr, args.memAddr, 1, const_cast<uint8_t*>(args.buf), args.len, args.timeout.tick);
        }

        /// I2C write non blocking
        /// @param args
        ///     - .deviceAddr device address
        ///     - .memAddr memory address
        ///     - .buf pointer to data buffer
        ///     - .len data length
        /// @retval HAL_StatusTypeDef (see stm32fXxx_hal_def.h)
        int write(ReadWriteArgs args) {
            #ifdef PERIPH_I2C_MEM_WRITE_USE_IT
            return HAL_I2C_Mem_Write_IT(&hi2c, args.deviceAddr, args.memAddr, 1, const_cast<uint8_t*>(args.buf), args.len);
            #endif
            #ifdef PERIPH_I2C_MEM_WRITE_USE_DMA
            return HAL_I2C_Mem_Write_DMA(&hi2c, args.deviceAddr, args.memAddr, 1, const_cast<uint8_t*>(args.buf), args.len);
            #endif
        }

        /// I2C read blocking
        /// @param args
        ///     - .deviceAddr device address
        ///     - .memAddr memory address
        ///     - .buf pointer to data buffer
        ///     - .len data length
        ///     - .timeout write timeout, default time::infinite
        /// @retval HAL_StatusTypeDef (see stm32fXxx_hal_def.h)
        int readBlocking(ReadWriteBlockingArgs args) {
            while (hi2c.State != HAL_I2C_STATE_READY);
            return HAL_I2C_Mem_Read(&hi2c, args.deviceAddr, args.memAddr, 1, const_cast<uint8_t*>(args.buf), args.len, args.timeout.tick);
        }
    };
} // namespace Project

#endif // HAL_I2C_MODULE_ENABLED
#endif // PERIPH_I2C_H