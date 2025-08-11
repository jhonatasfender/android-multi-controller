#ifndef ANDROID_SERVER_SCREEN_CAPTURE_CAPTURE_INTERFACE_H
#define ANDROID_SERVER_SCREEN_CAPTURE_CAPTURE_INTERFACE_H

#include "../common/types.h"
#include <functional>
#include <memory>

namespace android_server::screen_capture
{
    using FrameCallback = std::function<void(const byte_vector& frameData,
                                             uint64_t timestamp,
                                             const Resolution& resolution)>;

    using ErrorCallback = std::function<void(Result error, const std::string& message)>;

    struct FrameInfo
    {
        Resolution resolution;
        uint32_t format;
        uint32_t stride;
        uint64_t timestamp;
        size_t dataSize;

        FrameInfo()
            : format(0), stride(0), timestamp(0), dataSize(0)
        {
        }
    };

    class CaptureInterface
    {
    public:
        virtual ~CaptureInterface() = default;

        virtual Result initialize(const Resolution& resolution) = 0;
        virtual Result start() = 0;
        virtual Result stop() = 0;
        virtual bool isCapturing() const = 0;

        virtual void setFrameCallback(FrameCallback callback) = 0;
        virtual void setErrorCallback(ErrorCallback callback) = 0;

        virtual Resolution getCurrentResolution() const = 0;
        virtual std::vector<Resolution> getSupportedResolutions() const = 0;
        virtual uint32_t getCurrentFormat() const = 0;
        virtual std::vector<uint32_t> getSupportedFormats() const = 0;

        virtual Result setResolution(const Resolution& resolution) = 0;
        virtual Result setFormat(uint32_t format) = 0;
        virtual Result setFrameRate(uint32_t fps) = 0;

        virtual uint64_t getTotalFramesCaptured() const = 0;
        virtual uint64_t getDroppedFrames() const = 0;
        virtual double getAverageFrameRate() const = 0;

        virtual std::string getCaptureName() const = 0;
        virtual std::string getCaptureVersion() const = 0;
        virtual bool requiresRoot() const = 0;
        virtual bool supportsHardwareAcceleration() const = 0;
    };

    class CaptureFactory
    {
    public:
        enum class CaptureType
        {
            MEDIA_PROJECTION,
            SURFACE_FLINGER,
            AUTO_DETECT
        };

        static std::unique_ptr<CaptureInterface> createCapture(CaptureType type);
        static std::vector<CaptureType> getAvailableCaptures();
        static bool isCaptureSupported(CaptureType type);
        static std::string getCaptureTypeName(CaptureType type);
    };
}

#endif // ANDROID_SERVER_SCREEN_CAPTURE_CAPTURE_INTERFACE_H
