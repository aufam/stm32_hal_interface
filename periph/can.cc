#include "periph/can.h"

#ifdef HAL_CAN_MODULE_ENABLED

using namespace Project::periph;

static CAN* selector(CAN_HandleTypeDef *hcan_) {
    for (auto instance : CAN::Instances.instances) {
        if (instance == nullptr)
            continue;

        if (hcan_->Instance == instance->hcan_.Instance)
            return instance;
    }

    return nullptr;
}

#ifdef PERIPH_CAN_USE_FIFO0
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan_) {
#endif
#ifdef PERIPH_CAN_USE_FIFO1
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan_) {
#endif
    
    auto can = selector(hcan_);
    if (can == nullptr)
        return;

    CAN::Message msg = {};
    HAL_CAN_GetRxMessage(&can->hcan, CAN::RX_FIFO, reinterpret_cast<CAN_RxHeaderTypeDef *>(&msg), msg.data);
    for (auto callback : adc->callbackList.instances)
        callback(msg);
}

#endif
