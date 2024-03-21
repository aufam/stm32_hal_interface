#ifndef PERIPH_USB_H
#define PERIPH_USB_H

#include "main.h"
#ifdef HAL_PCD_MODULE_ENABLED

#include "config.h"
#include "usbd_cdc_if.h"
#include "etl/array.h"
#include "etl/string.h"
#include "etl/function.h"

namespace Project::periph { struct USBD; extern USBD usb; }

/// USB peripheral class
struct Project::periph::USBD {
    using Callback = etl::Function<void(const uint8_t*, size_t), void*>; ///< callback function class
    using CallbackList = detail::UniqueInstances<Callback, PERIPH_CALLBACK_LIST_MAX_SIZE>;
    using Buffer = etl::Array<uint8_t, APP_RX_DATA_SIZE>;                ///< USB rx buffer classs

    Buffer &rxBuffer;                   ///< reference to USB rx buffer
    CallbackList rxCallbackList = {};   ///< list of rx callback functions
    CallbackList txCallbackList = {};   ///< list of tx callback functions

    USBD(const USBD&) = delete; ///< disable copy constructor
    USBD& operator=(const USBD&) = delete;  ///< disable move constructor

    /// USB transmit non blocking
    /// @param buf data buffer
    /// @param len buffer length
    /// @retval @ref USBD_StatusTypeDef (see usbd_def.h)
    int transmit(const void *buf, size_t len) { return CDC_Transmit_FS((uint8_t*) buf, len); }

    int transmitBlocking(const void *buf, size_t len) { 
        int res = USBD_OK; do {
            res = transmit(buf, len);
        } while (res == USBD_BUSY);
        return res;
    }

    /// write operator for etl::string
    template <size_t N>
    USBD& operator<<(const etl::String<N>& str) { 
        transmit(str.data(), str.len()); 
        return *this; 
    }

    /// write operator for traditional string
    USBD& operator<<(const char *str) { 
        transmit(str, strlen(str)); 
        return *this; 
    }
};

#endif // HAL_PCD_MODULE_ENABLED
#endif // PERIPH_USB_H