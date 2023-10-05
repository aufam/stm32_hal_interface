#include "periph/adc.h"

#ifdef HAL_ADC_MODULE_ENABLED

using namespace Project::periph;

static ADCD* selector(ADC_HandleTypeDef* hadc) {
    for (auto instance : ADCD::Instances.instances) {
        if (instance == nullptr)
            continue;

        if (hadc->Instance == instance->hadc.Instance)
            return instance;
    }

    return nullptr;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    auto adc = selector(hadc);
    if (adc == nullptr)
        return;

    for (auto callback : adc->callbackList.instances)
        callback();
}

#endif