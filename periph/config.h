#ifndef PERIPH_CONFIG_H
#define PERIPH_CONFIG_H

// callback list
#if !defined(PERIPH_CALLBACK_LIST_MAX_SIZE)
#define PERIPH_CALLBACK_LIST_MAX_SIZE 16
#endif

// Timebase source
#if !defined(PERIPH_SYSTICK_TIM_BASE_SOURCE)
// example:
// #define PERIPH_SYSTICK_TIM_BASE_SOURCE TIM14
#endif

// ADC
#if !defined(PERIPH_ADC_N_CHANNEL)
#define PERIPH_ADC_N_CHANNEL 4
#endif

#if !defined(PERIPH_ADC_VREF)
#define PERIPH_ADC_VREF 3.3
#endif

#if !defined(PERIPH_ADC_RESOLUTION_BITS)
#define PERIPH_ADC_RESOLUTION_BITS 12
#endif

// CAN
#if !defined(PERIPH_CAN_USE_FIFO0) && !defined(PERIPH_CAN_USE_FIFO1)
#define PERIPH_CAN_USE_FIFO1
#endif

// TIM encoder
#if !defined(PERIPH_ENCODER_USE_IT) && !defined(PERIPH_ENCODER_USE_DMA)
#define PERIPH_ENCODER_USE_IT
#endif

// TIM input capture
#if !defined(PERIPH_INPUT_CAPTURE_USE_IT) && !defined(PERIPH_INPUT_CAPTURE_USE_DMA)
#define PERIPH_INPUT_CAPTURE_USE_IT
#endif

// TIM PWM 
#if !defined(PERIPH_PWM_USE_IT) && !defined(PERIPH_PWM_USE_DMA)
#define PERIPH_PWM_USE_IT
#endif

// I2C
#if !defined(PERIPH_I2C_MEM_WRITE_USE_IT) && !defined(PERIPH_I2C_MEM_WRITE_USE_DMA)
#define PERIPH_I2C_MEM_WRITE_USE_DMA
#endif

// I2S
#if !defined(PERIPH_I2S_AUDIO_RATE)
#define PERIPH_I2S_AUDIO_RATE 8000
#endif

#if !defined(PERIPH_I2S_N_SAMPLES)
#define PERIPH_I2S_N_SAMPLES 160
#endif

#if !defined(PERIPH_I2S_CHANNEL_MONO) && !defined(PERIPH_I2S_CHANNEL_STEREO)
#define PERIPH_I2S_CHANNEL_STEREO
#endif

#define PERIPH_I2S_SAMPLING_TIME ((double) PERIPH_I2S_N_SAMPLES / (double) PERIPH_I2S_AUDIO_RATE)

// UART
#if !defined(PERIPH_UART_RECEIVE_USE_IT) && !defined(PERIPH_UART_RECEIVE_USE_DMA)
#define PERIPH_UART_RECEIVE_USE_IT
#endif

#if !defined(PERIPH_UART_TRANSMIT_USE_IT) && !defined(PERIPH_UART_TRANSMIT_USE_DMA)
#define PERIPH_UART_TRANSMIT_USE_IT
#endif

#if !defined(PERIPH_UART_RX_BUFFER_SIZE)
#define PERIPH_UART_RX_BUFFER_SIZE 64
#endif

namespace Project::periph::detail {
    template <typename T, unsigned int N> 
    class UniqueInstances {
    public:
        T instances[N] = {};

        void push(T it);
        void pop(T it);
        bool isEmpty();
        T* find(T it);
    };
}

template <typename T, unsigned int N>
void Project::periph::detail::UniqueInstances<T, N>::push(T it) {
    if (find(it))
        return;

    T empty = {};
    T* ptr = find(empty);
    if (ptr)
        *ptr = it;
}

template <typename T, unsigned int N>
void Project::periph::detail::UniqueInstances<T, N>::pop(T it) {
    T empty = {};
    for (T* ptr = find(it); ptr != nullptr; ptr = find(it))
        *ptr = empty;
}

template <typename T, unsigned int N>
T* Project::periph::detail::UniqueInstances<T, N>::find(T it) {
        for (auto& instance : instances) if (instance == it) {
            return &instance;
        }
        return nullptr;
    }

template <typename T, unsigned int N>
bool Project::periph::detail::UniqueInstances<T, N>::isEmpty() {
    T empty = {};
    for (auto& instance : instances) if (instance != empty)
        return false;
    
    return true;
}

#endif // PERIPH_CONFIG_H