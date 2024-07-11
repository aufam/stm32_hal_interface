#include "periph/usb.h"
#ifdef HAL_PCD_MODULE_ENABLED

extern uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

using namespace Project::periph;
USBD Project::periph::usb { .rxBuffer = *(USBD::Buffer *) UserRxBufferFS };

extern "C" void CDC_ReceiveCplt_Callback(const uint8_t *pbuf, uint32_t len) {
    (void) pbuf;
    for (auto& callback : usb.rxCallbackList.instances) {
        callback(usb.rxBuffer.data(), len);
    }
}

extern "C" void CDC_TransmitCplt_Callback(const uint8_t *pbuf, uint32_t len) {
    for (auto& callback : usb.txCallbackList.instances) {
        callback(pbuf, len);
    }

    usb.isBusy = false;
    for (size_t i = 0; i < PERIPH_CALLBACK_LIST_MAX_SIZE; ++i) {
        if (usb.pending_buffer[i] != nullptr) {
            usb.transmit(usb.pending_buffer[i], usb.pending_buffer_size[i]);
            usb.pending_buffer[i] = nullptr;
            return;
        }
    }
}

#endif
