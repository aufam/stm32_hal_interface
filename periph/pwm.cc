#include "periph/pwm.h"

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

static PWM* selector(TIM_HandleTypeDef *htim) {
    for (auto instance : PWM::Instances.instances) {
        if (instance == nullptr)
            continue;

        if (htim->Instance == instance->htim.Instance && htim->Channel == activeChannel(instance->channel))
            return instance;
    }

    return nullptr;
}

void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim) {
    auto pwm = selector(htim);
    if (pwm == nullptr)
        return;

    pwm->halfCallback();
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
    auto pwm = selector(htim);
    if (pwm == nullptr)
        return;

    pwm->fullCallback();
}

#endif // HAL_TIM_MODULE_ENABLED
