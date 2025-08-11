#ifndef ANDROID_SERVER_AUDIO_CAPTURE_INTERFACE_H
#define ANDROID_SERVER_AUDIO_CAPTURE_INTERFACE_H

#include "../common/types.h"
#include <functional>

namespace android_server
{
    namespace audio_capture
    {
        class AudioCaptureInterface
        {
        public:
            using AudioFrameCallback = std::function<void(const byte_vector&, uint64_t)>;

            virtual ~AudioCaptureInterface() = default;

            virtual Result initialize(const AudioConfig& config) = 0;
            virtual Result start() = 0;
            virtual Result stop() = 0;
            virtual bool isCapturing() const = 0;

            virtual void setFrameCallback(AudioFrameCallback callback) = 0;
            virtual AudioConfig getConfig() const = 0;
            virtual AudioCaptureStats getStats() const = 0;
            virtual Result updateConfig(const AudioConfig& config) = 0;
            virtual void cleanup() = 0;

            virtual const char* getName() const = 0;
            virtual bool isHardwareAccelerated() const = 0;

        protected:
            AudioCaptureInterface() = default;
        };
    }
}

#endif // ANDROID_SERVER_AUDIO_CAPTURE_INTERFACE_H
