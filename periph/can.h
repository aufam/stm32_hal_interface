#ifndef PERIPH_CAN_H
#define PERIPH_CAN_H

#include "main.h"
#ifdef HAL_CAN_MODULE_ENABLED

#include "periph/config.h"
#include "Core/Inc/can.h"
#include "etl/function.h"
#include "etl/linked_list.h"

namespace Project::periph {

    /// CAN peripheral class
    /// @note requirements: CAN RXx interrupt
    struct CAN {
        struct Message : CAN_RxHeaderTypeDef { uint8_t data[8]; };
        using Callback = etl::Function<void(Message &), void*>;
        using CallbackList = detail::UniqueInstances<Callback, PERIPH_CALLBACK_LIST_MAX_SIZE>
        inline static detail::UniqueInstances<CAN*, 3> Instances;

        enum {
            #ifdef PERIPH_CAN_USE_FIFO0
            RX_FIFO = CAN_RX_FIFO0,
            IT_RX_FIFO = CAN_IT_RX_FIFO0_MSG_PENDING,
            FILTER_FIFO = CAN_FILTER_FIFO0,
            #endif
            #ifdef PERIPH_CAN_USE_FIFO1
            RX_FIFO = CAN_RX_FIFO1,
            IT_RX_FIFO = CAN_IT_RX_FIFO1_MSG_PENDING,
            FILTER_FIFO = CAN_FILTER_FIFO1,
            #endif
        };

        enum { ID_TYPE_STD, ID_TYPE_EXT };

        CAN_HandleTypeDef &hcan;            ///< CAN handler configured by CubeMX
        CAN_FilterTypeDef canFilter = {}; 
        CAN_TxHeaderTypeDef txHeader = {};
        uint32_t txMailbox = {};
        CallbackList rxCallbackList = {};

        CAN(const CAN&) = delete;               ///< disable copy constructor
        CAN& operator=(const CAN&) = delete;    ///< disable copy assignment

        struct InitArgs { bool idType };

        /// start CAN and activate notification at RX FIFO message pending
        /// @param args
        ///     - .idType ID_TYPE_STD or ID_TYPE_EXT
        void init(InitArgs args = {.idType=ID_TYPE_STD}) {
            txHeader.RTR = CAN_RTR_DATA;
            txHeader.TransmitGlobalTime = DISABLE;
            setIdType(args.idType);
            setFilter();
            HAL_CAN_Start(&hcan);
            HAL_CAN_ActivateNotification(&hcan, IT_RX_FIFO);
            Instances.push(this);
        }

        /// stop CAN 
        void deinit() { 
            if (callbackList.isEmpty()) {
                HAL_CAN_Stop(&hadc); 
                Instances.pop(this);
            }
        }

        /// set filter by hardware
        /// @param filter default = 0
        /// @param mask default = 0
        /// @example filter = 0b1100, mask = 0b1111 -> allowed rx id: only 0b1100
        /// @example filter = 0b1100, mask = 0b1110 -> allowed rx id: 0b1100 and 0b1101
        void setFilter(uint32_t filter = 0, uint32_t mask = 0) {
            canFilter.FilterActivation = CAN_FILTER_ENABLE;

            if (isUsingExtId()) {
                // 18 bits, 3 bits offset, low half-word
                canFilter.FilterMaskIdLow  = (mask << 3) & 0xFFFFu;
                canFilter.FilterMaskIdHigh = (mask >> (18 - 5)) & 0b11111u;
                canFilter.FilterIdLow      = (filter << 3) & 0xFFFFu;
                canFilter.FilterIdHigh     = (filter >> (18 - 5)) & 0b11111u;
            } else {
                // 11 bits, left padding, high half-word
                canFilter.FilterMaskIdLow  = 0;
                canFilter.FilterMaskIdHigh = (mask << 5) & 0xFFFFu;
                canFilter.FilterIdLow      = 0;
                canFilter.FilterIdHigh     = (filter << 5) & 0xFFFFu;
            }

            canFilter.FilterFIFOAssignment = FILTER_FIFO;
            canFilter.FilterMode = CAN_FILTERMODE_IDMASK;
            canFilter.FilterScale = CAN_FILTERSCALE_32BIT;
            canFilter.FilterBank = 10;
            canFilter.SlaveStartFilterBank = 0;

            HAL_CAN_ConfigFilter(&hcan, &canFilter);
        }

        /// set tx ID type, standard or extended
        /// @param idType ID_TYPE_STD or ID_TYPE_EXT
        void setIdType(bool idType) {
            txHeader.IDE = idType ? CAN_ID_EXT : CAN_ID_STD;
        }

        /// set tx ID
        void setId(uint32_t txId) {
            if (isUsingExtId()) txHeader.ExtId = txId;
            else txHeader.StdId = txId;
        }

        [[nodiscard]] bool isUsingExtId() const {
            return txHeader.IDE == CAN_ID_EXT;
        }

        struct TransmitArgs { const uint8_t* buf; uint16_t len = 8; };

        /// CAN transmit non blocking
        /// @param args
        ///     - .buf pointer to data buffer
        ///     - .len buffer length, maximum 8 bytes, default 8
        /// @retval HAL_StatusTypeDef. see stm32fXxx_hal_def.h
        int transmit(TransmitArgs args) {
            if (args.len > 8) args.len = 8;
            txHeader.DLC = args.len;
            return HAL_CAN_AddTxMessage(&hcan, &txHeader, const_cast<uint8_t*>(args.buf), &txMailbox);
        }

        struct TransmitTxIdArgs { uint32_t txId; const uint8_t* buf; uint16_t len = 8; };

        /// CAN transmit non blocking with specific tx ID
        /// @param args
        ///     - .txId destination id
        ///     - .buf pointer to data buffer
        ///     - .len buffer length, maximum 8 bytes, default 8
        /// @retval HAL_StatusTypeDef. see stm32fXxx_hal_def.h
        int transmit(TransmitTxIdArgs args) {
            setId(args.txId);
            return transmit(args.buf, args.len);
        }

        struct TransmitIdTypeTxIdArgs { uint32_t txId; const uint8_t* buf; uint16_t len = 8; };

        /// CAN transmit non blocking with specific tx ID and set ID type
        /// @param args
        ///     - .idType ID_TYPE_STD or ID_TYPE_EXT
        ///     - .txId destination id
        ///     - .buf pointer to data buffer
        ///     - .len buffer length, maximum 8 bytes, default 8
        /// @retval HAL_StatusTypeDef. see stm32fXxx_hal_def.h
        int transmit(TransmitIdTypeTxIdArgs args) {
            setIdType(args.idType);
            setId(args.txId);
            return transmit(args.buf, args.len);
        }
    };
} // namespace Project

#endif // HAL_CAN_MODULE_ENABLED
#endif // PERIPH_CAN_H