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
    bool isBusy = false;

    const void* pending_buffer[PERIPH_CALLBACK_LIST_MAX_SIZE] = {};
    size_t pending_buffer_size[PERIPH_CALLBACK_LIST_MAX_SIZE] = {};

    USBD(const USBD&) = delete; ///< disable copy constructor
    USBD& operator=(const USBD&) = delete;  ///< disable move constructor

    /// USB transmit non blocking
    /// @param buf data buffer
    /// @param len buffer length
    /// @retval @ref USBD_StatusTypeDef (see usbd_def.h)
    int transmit(const void *buf, size_t len) { 
        if (isBusy) {
            for (size_t i = 0; i < PERIPH_CALLBACK_LIST_MAX_SIZE; ++i) {
                if (pending_buffer[i] == nullptr) {
                    pending_buffer[i] = buf;
                    pending_buffer_size[i] = len;
                    return USBD_BUSY;
                }
            }
            return USBD_FAIL;
        }

        isBusy = true;
        return CDC_Transmit_FS((uint8_t*) buf, len); 
    }

    /// USB transmit non blocking
    /// @param buf data buffer
    /// @param len buffer length
    /// @retval @ref USBD_StatusTypeDef (see usbd_def.h)
    int transmitBlocking(const void *buf, size_t len) { 
        while (isBusy);
        return transmit(buf, len); 
    }

    /// write operator for etl::string
    template <size_t N>
    USBD& operator<<(const etl::String<N>& str) { 
        transmitBlocking(str.data(), str.len()); 
        return *this; 
    }

    /// write operator for traditional string
    USBD& operator<<(const char *str) { 
        transmitBlocking(str, strlen(str)); 
        return *this; 
    }
};

#endif // HAL_PCD_MODULE_ENABLED
#endif // PERIPH_USB_H