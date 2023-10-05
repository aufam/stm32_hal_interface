#ifndef PERIPH_I2S_H
#define PERIPH_I2S_H

#include "main.h"
#ifdef HAL_I2S_MODULE_ENABLED

#include "periph/config.h"
#include "Core/Inc/i2s.h"
#include "etl/array.h"
#include "etl/event.h"

namespace Project::periph {

    /// I2S peripheral class.
    /// @note requirements: 
    ///     - Full duplex master
    ///     - SPIx global interrupt
    ///     - standard I2S Philips
    ///     - 16 bits data on 16 bits frame
    ///     - tx & rx DMA circular 16 bit
    struct I2S {
        inline static detail::UniqueInstances<I2S*, 16> Instances;

        typedef int16_t Mono;
        struct Stereo { Mono left, right; };

        static const size_t nSamples = PERIPH_I2S_N_SAMPLES;
        static const size_t audioRate = PERIPH_I2S_AUDIO_RATE;
        static constexpr float samplingTime = PERIPH_I2S_SAMPLING_TIME;
        inline static const etl::Time eventTimeout = etl::time::milliseconds(samplingTime * 1000 * 2);

        using BufferMono = etl::Array<Mono, nSamples>;
        using BufferStereo = etl::Array<Stereo, nSamples>;

        #ifdef PERIPH_I2S_CHANNEL_STEREO
        static const size_t nChannels = 2;
        using DualBuffer = etl::Array<Stereo, nSamples * 2>;
        #endif
        #ifdef PERIPH_I2S_CHANNEL_MONO
        using DualBuffer = etl::Array<Mono, nSamples * 2>;
        static const size_t nChannels = 1;
        #endif
        
        enum { FLAG_HALF = 1 << 0, FLAG_FULL = 1 << 1 };

        I2S_HandleTypeDef &hi2s; ///< I2S handler configured in cubeMX
        DualBuffer txBuffer = {};
        DualBuffer rxBuffer = {};
        etl::Event evt = {};

        /// start transmit receive DMA and register this instance
        void init() {
            HAL_I2SEx_TransmitReceive_DMA(&hi2s, (uint16_t*) &txBuffer, (uint16_t*) &rxBuffer, DualBuffer::size() * nChannels);
            evt.init();
            Instances.push(this);
        }

        /// stop DMA and unregister this instance
        void deinit() {
            HAL_I2S_DMAStop(&hi2s);
            evt.deinit();
            Instances.pop(this);
        }

        void halfCallback() {
            evt.resetFlags(FLAG_FULL);
            evt.setFlags(FLAG_HALF);
        }

        void fullCallback() {
            evt.resetFlags(FLAG_HALF);
            evt.setFlags(FLAG_FULL);
        }

        struct ReadMonoArgs { BufferMono& buffer; bool leftOrRight = false; };

        /// read audio data and store to buffer mono
        /// @param args 
        ///     - .buffer[out] buffer mono to store the audio data
        ///     - .leftOrRight left (false) or right (true) channel, default left 
        /// @return osStatus
        osStatus_t read(ReadMonoArgs args) {
            auto flag = evt.waitFlagsAny({.timeout=eventTimeout});
            auto buf = rxBuffer.begin();

            if (!flag)
                return osErrorTimeout;
            
            if (flag & FLAG_FULL)
                buf = rxBuffer.begin() + nSamples;
            else if (flag & FLAG_HALF)
                buf = rxBuffer.begin();
            else 
                return osErrorResource;

            #ifdef PERIPH_I2S_CHANNEL_STEREO
            if (args.leftOrRight)
                for (size_t i = 0; i < nSamples; i++)
                    args.buffer[i] = buf[i].right;
            else
                for (size_t i = 0; i < nSamples; i++)
                    args.buffer[i] = buf[i].left;
            #endif
            #ifdef PERIPH_I2S_CHANNEL_MONO
            UNUSED(args.leftOrRight);
            for (size_t i = 0; i < nSamples; i++)
                args.buffer[i] = buf[i];
            #endif

            return osOK;
        }

        struct ReadStereoArgs { BufferStereo& buffer; };

        /// read audio data and store to buffer stereo
        /// @param args 
        ///     - .buffer[out] buffer stereo to store the audio data
        /// @return osStatus
        osStatus_t read(ReadStereoArgs args) {
            auto flag = evt.waitFlagsAny({.timeout=eventTimeout});
            auto buf = rxBuffer.begin();

            if (!flag)
                return osErrorTimeout;
            
            if (flag & FLAG_FULL)
                buf = rxBuffer.begin() + nSamples;
            else if (flag & FLAG_HALF)
                buf = rxBuffer.begin();
            else 
                return osErrorResource;

            #ifdef PERIPH_I2S_CHANNEL_STEREO
            for (size_t i = 0; i < nSamples; i++)
                args.buffer[i] = buf[i];
            #endif
            #ifdef PERIPH_I2S_CHANNEL_MONO
            for (size_t i = 0; i < nSamples; i++) {
                args.buffer[i].left = buf[i];
                args.buffer[i].right = buf[i];
            }
            #endif

            return osOK;
        }

        struct WriteMonoArgs { const BufferMono& buffer; bool leftOrRight = false; };

        /// write audio data from buffer mono
        /// @param args 
        ///     - .buffer[in] audio data as buffer mono
        ///     - .leftOrRight left (false) or right (true) channel, default left 
        /// @return osStatus
        osStatus_t write(WriteMonoArgs args) {
            auto flag = evt.waitFlagsAny({.timeout=eventTimeout});
            auto buf = rxBuffer.begin();

            if (!flag)
                return osErrorTimeout;
            
            if (flag & FLAG_FULL)
                buf = rxBuffer.begin();
            else if (flag & FLAG_HALF)
                buf = rxBuffer.begin() + nSamples;
            else 
                return osErrorResource;

            #ifdef PERIPH_I2S_CHANNEL_STEREO
            if (args.leftOrRight)
                for (size_t i = 0; i < nSamples; i++)
                    buf[i].right = args.buffer[i];
            else
                for (size_t i = 0; i < nSamples; i++)
                    buf[i].left = args.buffer[i];
            #endif
            #ifdef PERIPH_I2S_CHANNEL_MONO
            UNUSED(args.leftOrRight);
            for (size_t i = 0; i < nSamples; i++)
                buf[i] = args.buffer[i];
            #endif

            return osOK;
        }

        struct WriteStereoArgs { const BufferStereo& buffer; };

        /// write audio data from buffer stereo
        /// @param args 
        ///     - .buffer[in] audio data as buffer stereo
        ///     - .leftOrRight left (false) or right (true) channel, default left 
        /// @return osStatus
        osStatus_t write(WriteStereoArgs args) {
            auto flag = evt.waitFlagsAny({.timeout=eventTimeout});
            auto buf = rxBuffer.begin();

            if (!flag)
                return osErrorTimeout;
            
            if (flag & FLAG_FULL)
                buf = rxBuffer.begin();
            else if (flag & FLAG_HALF)
                buf = rxBuffer.begin() + nSamples;
            else 
                return osErrorResource;

            #ifdef PERIPH_I2S_CHANNEL_STEREO
            for (size_t i = 0; i < nSamples; i++)
                buf[i] = args.buffer[i];
            #endif
            #ifdef PERIPH_I2S_CHANNEL_MONO
            for (size_t i = 0; i < nSamples; i++)
                buf[i] = args.buffer[i].left / 2 + args.buffer[i].right / 2;
            #endif

            return osOK;
        }
    };
} // namespace Project

#endif // HAL_I2S_MODULE_ENABLED
#endif // PERIPH_I2S_H