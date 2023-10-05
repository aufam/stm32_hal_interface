#include "periph/i2c.h"

#ifdef HAL_I2C_MODULE_ENABLED

using namespace Project::periph;

static I2C* selector(I2C_HandleTypeDef *hi2c) {
    for (auto instance : I2C::Instances.instances) {
        if (instance == nullptr)
            continue;
        
        if (hi2c->Instance == instance->hi2c.Instance)
            return instance;
    }

    return nullptr;
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
    auto i2c = selector(hi2c);
    if (i2c == nullptr)
        return;

    i2c->txCallback();
}

#endif
