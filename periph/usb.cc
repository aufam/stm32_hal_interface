#include "periph/usb.h"
#ifdef HAL_PCD_MODULE_ENABLED

extern uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

using namespace Project::periph;
USBD Project::periph::usb { .rxBuffer = *(USBD::Buffer *) UserRxBufferFS };

void CDC_ReceiveCplt_Callback(const uint8_t *pbuf, uint32_t len) {
    (void) pbuf;
    for (auto& callback : usb.rxCallbackList.instances) {
        callback(usb.rxBuffer.data(), len);
    }
}

void CDC_TransmitCplt_Callback(const uint8_t *pbuf, uint32_t len) {
    for (auto& callback : usb.txCallbackList.instances) {
        callback(pbuf, len);
    }
}

#endif