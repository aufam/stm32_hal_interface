#ifndef PERIPH_CAN_H
#define PERIPH_CAN_H

#include "main.h"
#ifdef HAL_CAN_MODULE_ENABLED

#include "periph/config.h"
#include "Core/Inc/can.h"
#include "etl/function.h"
#include "etl/getter_setter.h"
#include "etl/linked_list.h"

namespace Project::periph { struct CAN; }

/// CAN peripheral class
/// @note requirements: CAN RXx interrupt
struct Project::periph::CAN {
    struct Message : CAN_RxHeaderTypeDef { uint8_t data[8]; };
    using Callback = etl::Function<void(Message &), void*>;
    using CallbackList = detail::UniqueInstances<Callback, PERIPH_CALLBACK_LIST_MAX_SIZE>;

    template <typename T>
    using GetterSetter = etl::GetterSetter<T, etl::Function<T(), const CAN*>, etl::Function<void(T), CAN*>>;

    static detail::UniqueInstances<CAN*, 3> Instances;
    
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

    CAN_HandleTypeDef &hcan;            ///< CAN handler configured by CubeMX
    CAN_FilterTypeDef canFilter = {}; 
    CAN_TxHeaderTypeDef txHeader = {};
    uint32_t txMailbox = {};
    CallbackList rxCallbackList = {};

    CAN(const CAN&) = delete;               ///< disable copy constructor
    CAN& operator=(const CAN&) = delete;    ///< disable copy assignment

    void init() {
        txHeader.RTR = CAN_RTR_DATA;
        txHeader.TransmitGlobalTime = DISABLE;
        HAL_CAN_Start(&hcan);
        HAL_CAN_ActivateNotification(&hcan, IT_RX_FIFO);
        Instances.push(this);
    }

    struct InitArgs { uint32_t idType, idTx, filter, mask; Callback rxCallback = {}; };

    /// start CAN and activate notification at RX FIFO message pending
    /// @param args
    ///     - .idType CAN_ID_STD or CAN_ID_EXT
    ///     - .idTx
    ///     - .filter
    ///     - .mask
    void init(InitArgs args) {
        idType = args.idType;
        idTx = args.idTx;
        filter = args.filter;
        mask = args.filter;
        rxCallbackList.push(args.rxCallback);
        init();
    }

    /// stop CAN 
    void deinit() { 
        if (rxCallbackList.isEmpty()) {
            HAL_CAN_Stop(&hcan); 
            Instances.pop(this);
        }
    }
    struct DeinitArgs { Callback rxCallback; };
    void deinit(DeinitArgs args) { 
        rxCallbackList.pop(args.rxCallback);
        deinit();
    }

    /// get and set id type, CAN_ID_STD or CAN_ID_EXT
    const GetterSetter<uint32_t> idType = {
        {+[] (const CAN* self) { return self->txHeader.IDE; }, this},
        {+[] (CAN* self, uint32_t value) { self->txHeader.IDE = value == CAN_ID_STD ? CAN_ID_STD : CAN_ID_EXT; }, this}
    };

    /// get and set id transmit
    const GetterSetter<uint32_t> idTx = {
        {+[] (const CAN* self) { return self->idType == CAN_ID_STD ? self->txHeader.StdId : self->txHeader.ExtId; }, this},
        {+[] (CAN* self, uint32_t value) { if (self->idType == CAN_ID_STD) self->txHeader.StdId = value; else self->txHeader.ExtId = value; }, this}
    };

    /// get and set filter register
    /// @example filter = 0b1100, mask = 0b1111 -> allowed rx id: only 0b1100
    /// @example filter = 0b1100, mask = 0b1110 -> allowed rx id: 0b1100 and 0b1101
    const GetterSetter<uint32_t> filter = {
        {+[] (const CAN* self) { 
            if (self->idType == CAN_ID_STD) {
                // 11 bits, left padding, high half-word
                return (self->canFilter.FilterIdHigh & 0xFFFFu) >> (16 - 11);
            } else {
                // 18 bits, 3 bits offset, low half-word
                return ((self->canFilter.FilterIdLow & 0xFFFFu) >> (16 - 13)) |   // 13 bits from low half-word
                        ((self->canFilter.FilterIdHigh & 0b11111u) << 13);         // 5 bits from high half-word
            }
        }, this},
        {+[] (CAN* self, uint32_t value) { 
            if (self->idType == CAN_ID_STD) {
                // 11 bits, left padding, high half-word
                self->canFilter.FilterIdLow      = 0;
                self->canFilter.FilterIdHigh     = (value << (16 - 11)) & 0xFFFFu;
            } else {
                // 18 bits, 3 bits offset, low half-word
                self->canFilter.FilterIdLow      = (value << (16 - 13)) & 0xFFFFu;  // 13 bits to low half-word
                self->canFilter.FilterIdHigh     = (value >> 13) & 0b11111u;        // 5 bits to high half-word
            }
            self->configureFilter();
        }, this}
    };

    /// get and set mask register
    /// @example filter = 0b1100, mask = 0b1111 -> allowed rx id: only 0b1100
    /// @example filter = 0b1100, mask = 0b1110 -> allowed rx id: 0b1100 and 0b1101
    const GetterSetter<uint32_t> mask = {
        {+[] (const CAN* self) { 
            if (self->idType == CAN_ID_STD) {
                // 11 bits, left padding, high half-word
                return (self->canFilter.FilterMaskIdHigh & 0xFFFFu) >> (16 - 11);
            } else {
                // 18 bits, 3 bits offset, low half-word
                return ((self->canFilter.FilterMaskIdLow & 0xFFFFu) >> 3) |           // 13 bits from low half-word
                        ((self->canFilter.FilterMaskIdHigh & 0b11111u) << 13);         // 5 bits from high half-word
            }
        }, this},
        {+[] (CAN* self, uint32_t value) { 
            if (self->idType == CAN_ID_STD) {
                // 11 bits, left padding, high half-word
                self->canFilter.FilterMaskIdLow      = 0;
                self->canFilter.FilterMaskIdHigh     = (value << (16 - 11)) & 0xFFFFu;
            } else {
                // 18 bits, 3 bits offset, low half-word
                self->canFilter.FilterMaskIdLow      = (value << 3) & 0xFFFFu;          // 13 bits to low half-word
                self->canFilter.FilterMaskIdHigh     = (value >> 13) & 0b11111u;        // 5 bits to high half-word
            }
            self->configureFilter();
        }, this}
    };

    /// CAN transmit non blocking
    /// @param buf pointer to data buffer
    /// @param len buffer length, maximum 8 bytes, default 8
    /// @retval HAL_StatusTypeDef. see stm32fXxx_hal_def.h
    int transmit(const uint8_t* buf, uint16_t len = 8) {
        if (len > 8) len = 8;
        txHeader.DLC = len;
        return HAL_CAN_AddTxMessage(&hcan, &txHeader, const_cast<uint8_t*>(buf), &txMailbox);
    }

    struct TransmitArgs { const uint8_t* buf; uint16_t len = 8; };

    /// CAN transmit non blocking
    /// @param args
    ///     - .buf pointer to data buffer
    ///     - .len buffer length, maximum 8 bytes, default 8
    /// @retval HAL_StatusTypeDef. see stm32fXxx_hal_def.h
    int transmit(TransmitArgs args) {
        return transmit(args.buf, args.len);
    }

    struct TransmitIdTxArgs { uint32_t idTx; const uint8_t* buf; uint16_t len = 8; };

    /// CAN transmit non blocking with specific tx ID
    /// @param args
    ///     - .idTx destination id
    ///     - .buf pointer to data buffer
    ///     - .len buffer length, maximum 8 bytes, default 8
    /// @retval HAL_StatusTypeDef. see stm32fXxx_hal_def.h
    int transmit(TransmitIdTxArgs args) {
        idTx = args.idTx;
        return transmit(args.buf, args.len);
    }

    struct TransmitIdTypeIdTxArgs { uint32_t idType; uint32_t idTx; const uint8_t* buf; uint16_t len = 8; };

    /// CAN transmit non blocking with specific tx ID and set ID type
    /// @param args
    ///     - .idType ID_TYPE_STD or ID_TYPE_EXT
    ///     - .idTx destination id
    ///     - .buf pointer to data buffer
    ///     - .len buffer length, maximum 8 bytes, default 8
    /// @retval HAL_StatusTypeDef. see stm32fXxx_hal_def.h
    int transmit(TransmitIdTypeIdTxArgs args) {
        idType = args.idType;
        idTx = args.idTx;
        return transmit(args.buf, args.len);
    }

private:
    void configureFilter() {
        canFilter.FilterActivation = CAN_FILTER_ENABLE;
        canFilter.FilterFIFOAssignment = FILTER_FIFO;
        canFilter.FilterMode = CAN_FILTERMODE_IDMASK;
        canFilter.FilterScale = CAN_FILTERSCALE_32BIT;
        canFilter.FilterBank = 10;
        canFilter.SlaveStartFilterBank = 0;
        HAL_CAN_ConfigFilter(&hcan, &canFilter);
    }
};

#endif // HAL_CAN_MODULE_ENABLED
#endif // PERIPH_CAN_H