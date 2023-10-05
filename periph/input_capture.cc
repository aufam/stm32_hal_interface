#include "periph/input_capture.h"
#include "periph/encoder.h"

#ifdef HAL_TIM_MODULE_ENABLED

using namespace Project::periph;

static HAL_TIM_ActiveChannel activeChannel(uint32_t channel) {
    switch (channel) {
        case TIM_CHANNEL_1: return HAL_TIM_ACTIVE_CHANNEL_1;
        case TIM_CHANNEL_2: return HAL_TIM_ACTIVE_CHANNEL_2;
        case TIM_CHANNEL_3: return HAL_TIM_ACTIVE_CHANNEL_3;
        case TIM_CHANNEL_4: return HAL_TIM_ACTIVE_CHANNEL_4;
        default: return HAL_TIM_ACTIVE_CHANNEL_CLEARED;
    }
}

static InputCapture* selector(TIM_HandleTypeDef *htim) {
    for (auto instance : InputCapture::Instances.instances) {
        if (instance == nullptr)
            continue;

        if (htim->Instance == instance->htim.Instance && htim->Channel == activeChannel(instance->channel))
            return instance;
    }
    
    return nullptr;
}

static Encoder* selectorEncoder(TIM_HandleTypeDef *htim) {
    for (auto instance : Encoder::Instances.instances) {
        if (instance == nullptr)
            continue;

        if (htim->Instance == instance->htim.Instance)
            return instance;
    }

    return nullptr;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    auto ic = selector(htim);
    if (ic) ic->callback();

    auto enc = selectorEncoder(htim);
    if (enc) enc->inputCaptureCallback();
}

#endif