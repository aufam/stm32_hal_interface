#include "periph/i2s.h"

#ifdef HAL_I2S_MODULE_ENABLED

using namespace Project::periph;

static I2S* selector(I2S_HandleTypeDef *hi2s) {
    for (auto instance : I2S::Instances.instances) {
        if (instance == nullptr)
            continue;
        
        if (hi2s->Instance == instance->hi2s.Instance)
            return instance;
    }

    return nullptr;
}

void HAL_I2SEx_TxRxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
    auto i2s = selector(hi2s);
    if (i2s == nullptr)
        return;

    i2s->halfCallback();
}

void HAL_I2SEx_TxRxCpltCallback(I2S_HandleTypeDef *hi2s) {
    auto i2s = selector(hi2s);
    if (i2s == nullptr)
        return;

    i2s->fullCallback();
}

#endif