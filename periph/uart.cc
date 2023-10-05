#include "periph/uart.h"

#ifdef HAL_UART_MODULE_ENABLED

using namespace Project::periph;

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

    uart->rxCallback(uart->rxBuffer.data(), Size);
    uart->init();
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    auto uart = selector(huart);
    if (uart == nullptr)
        return;

    uart->txCallback();
}

#endif