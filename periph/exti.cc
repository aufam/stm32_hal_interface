#include "periph/exti.h"
#include "etl/bit.h"
#include "cmsis_os2.h" // osKernelGetTickCount
#include "main.h"

using namespace Project;
using namespace Project::periph;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    uint16_t static pinNow;
    etl::Time static timeNow;

    auto pinPrev = pinNow;
    auto timePrev = timeNow;

    // don't proceed if the kernel is not running
    if (osKernelGetState() != osKernelRunning)
        return;
        
    timeNow = etl::time::now();
    pinNow = GPIO_Pin;

    for (auto instance : Exti::Instances.instances) {
        if (instance == nullptr)
            continue; // skip if instance is null
        
        if (!(pinNow & instance->pin))
            continue; // skip if the triggered pin is not instance pin
        
        if (pinNow != pinPrev || etl::time::elapsed(timePrev) > instance->debounceDelay) {
            instance->callback();
            instance->counter++;
            return;
        }
    }
}
