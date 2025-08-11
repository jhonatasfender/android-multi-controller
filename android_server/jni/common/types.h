#ifndef ANDROID_SERVER_COMMON_TYPES_H
#define ANDROID_SERVER_COMMON_TYPES_H

#include <memory>
#include <vector>
#include <string>

namespace android_server
{
    using byte_t = uint8_t;
    using byte_vector = std::vector<byte_t>;
    using byte_ptr = std::unique_ptr<byte_t[]>;

    struct Resolution
    {
        uint32_t width;
        uint32_t height;

        Resolution() : width(0), height(0)
        {
        }

        Resolution(uint32_t w, uint32_t h) : width(w), height(h)
        {
        }

        bool isValid() const
        {
            return width > 0 && height > 0;
        }

        double aspectRatio() const
        {
            return height > 0 ? static_cast<double>(width) / height : 0.0;
        }
    };

    struct VideoConfig
    {
        Resolution resolution;
        uint32_t bitrate;
        uint32_t fps;
        uint32_t keyFrameInterval;
        uint32_t profile;
        uint32_t level;

        VideoConfig()
            : resolution(1280, 720)
              , bitrate(4000000)
              , fps(30)
              , keyFrameInterval(2)
              , profile(1)
              , level(31)
        {
        }
    };

    struct AudioConfig
    {
        uint32_t sampleRate;
        uint32_t channelCount;
        uint32_t bitrate;

        AudioConfig()
            : sampleRate(44100)
              , channelCount(2)
              , bitrate(128000)
        {
        }
    };

    struct AudioCaptureStats
    {
        uint64_t totalFramesCaptured;
        uint64_t totalBytesProcessed;
        uint32_t currentLatency;
        uint32_t averageLatency;
        uint32_t droppedFrames;
        bool isCapturing;

        AudioCaptureStats()
            : totalFramesCaptured(0)
              , totalBytesProcessed(0)
              , currentLatency(0)
              , averageLatency(0)
              , droppedFrames(0)
              , isCapturing(false)
        {
        }
    };

    enum class ServerStatus
    {
        STOPPED,
        STARTING,
        RUNNING,
        STOPPING,
        ERROR
    };

    enum class VideoCodec : uint32_t
    {
        H264 = 1,
        H265 = 2
    };

    enum class AudioCodec : uint32_t
    {
        AAC = 1,
        OPUS = 2
    };

    enum class InputEventType : uint8_t
    {
        TOUCH_DOWN = 1,
        TOUCH_UP = 2,
        TOUCH_MOVE = 3,
        KEY_DOWN = 4,
        KEY_UP = 5,
        SCROLL = 6
    };

    struct TouchEvent
    {
        InputEventType type;
        uint32_t pointerId;
        float x;
        float y;
        float pressure;
        uint64_t timestamp;
    };

    struct KeyEvent
    {
        InputEventType type;
        uint32_t keyCode;
        uint32_t scanCode;
        uint32_t metaState;
        uint64_t timestamp;
    };

    struct ScrollEvent
    {
        float x;
        float y;
        float scrollX;
        float scrollY;
        uint64_t timestamp;
    };

    struct DeviceInfo
    {
        std::string model;
        std::string manufacturer;
        std::string brand;
        std::string device;
        std::string product;
        std::string androidVersion;
        uint32_t apiLevel;
        Resolution screenResolution;
        float screenDensity;

        DeviceInfo()
            : apiLevel(0)
              , screenDensity(1.0f)
        {
        }
    };

    enum class Result
    {
        SUCCESS,
        ERROR_INVALID_PARAMS,
        ERROR_NOT_INITIALIZED,
        ERROR_ALREADY_RUNNING,
        ERROR_NOT_RUNNING,
        ERROR_PERMISSION_DENIED,
        ERROR_DEVICE_NOT_SUPPORTED,
        ERROR_NETWORK_FAILED,
        ERROR_ENCODER_FAILED,
        ERROR_CAPTURE_FAILED,
        ERROR_UNKNOWN
    };

    inline const char* resultToString(Result result)
    {
        switch (result)
        {
        case Result::SUCCESS: return "SUCCESS";
        case Result::ERROR_INVALID_PARAMS: return "ERROR_INVALID_PARAMS";
        case Result::ERROR_NOT_INITIALIZED: return "ERROR_NOT_INITIALIZED";
        case Result::ERROR_ALREADY_RUNNING: return "ERROR_ALREADY_RUNNING";
        case Result::ERROR_NOT_RUNNING: return "ERROR_NOT_RUNNING";
        case Result::ERROR_PERMISSION_DENIED: return "ERROR_PERMISSION_DENIED";
        case Result::ERROR_DEVICE_NOT_SUPPORTED: return "ERROR_DEVICE_NOT_SUPPORTED";
        case Result::ERROR_NETWORK_FAILED: return "ERROR_NETWORK_FAILED";
        case Result::ERROR_ENCODER_FAILED: return "ERROR_ENCODER_FAILED";
        case Result::ERROR_CAPTURE_FAILED: return "ERROR_CAPTURE_FAILED";
        default: return "ERROR_UNKNOWN";
        }
    }
}

#endif // ANDROID_SERVER_COMMON_TYPES_H
