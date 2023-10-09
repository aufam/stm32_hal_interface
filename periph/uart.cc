#include "periph/uart.h"

#ifdef HAL_UART_MODULE_ENABLED

using namespace Project::periph;

detail::UniqueInstances<UART*, 16> UART::Instances;

static UART* selector(UART_HandleTypeDef *huart) {
    for (auto instance : UART::Instances.instances) {
        if (instance == nullptr)
            continue;

        if (huart->Instance == instance->huart.Instance)
            return instance;
    }

    return nullptr;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    auto uart = selector(huart);
    if (uart == nullptr)
        return;

    for (auto& callback : uart->rxCallbackList.instances) {
        callback(uart->rxBuffer.data(), Size);
    }
    uart->init();
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    auto uart = selector(huart);
    if (uart == nullptr)
        return;

    for (auto& callback : uart->txCallbackList.instances) {
        callback();
    }
}

#endif