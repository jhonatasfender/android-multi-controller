#ifndef ANDROID_SERVER_VIDEO_ENCODER_ENCODER_INTERFACE_H
#define ANDROID_SERVER_VIDEO_ENCODER_ENCODER_INTERFACE_H

#include "../common/types.h"
#include <functional>
#include <memory>

namespace android_server::video_encoder
{
    using EncodedDataCallback = std::function<void(
        const byte_vector& data,
        uint64_t pts,
        uint64_t dts,
        bool keyframe,
        bool isConfig
    )>;

    using EncoderErrorCallback = std::function<void(Result error, const std::string& message)>;

    struct EncodedFrameInfo
    {
        uint64_t pts;
        uint64_t dts;
        uint32_t frameNumber;
        size_t dataSize;
        bool keyframe;
        bool configFrame;

        EncodedFrameInfo()
            : pts(0), dts(0), frameNumber(0), dataSize(0), keyframe(false), configFrame(false)
        {
        }
    };

    struct EncoderStats
    {
        uint64_t totalFramesEncoded;
        uint64_t totalKeyFrames;
        uint64_t totalBytesEncoded;
        uint64_t droppedFrames;
        double averageFrameRate;
        double averageBitrate;
        uint64_t encodingTimeTotal;
        uint64_t encodingTimeAverage;

        EncoderStats()
            : totalFramesEncoded(0), totalKeyFrames(0), totalBytesEncoded(0)
              , droppedFrames(0), averageFrameRate(0), averageBitrate(0)
              , encodingTimeTotal(0), encodingTimeAverage(0)
        {
        }
    };

    class EncoderInterface
    {
    public:
        virtual ~EncoderInterface() = default;

        virtual Result initialize(const VideoConfig& config) = 0;
        virtual Result start() = 0;
        virtual Result stop() = 0;
        virtual bool isEncoding() const = 0;

        virtual void setEncodedDataCallback(EncodedDataCallback callback) = 0;
        virtual void setErrorCallback(EncoderErrorCallback callback) = 0;

        virtual Result encodeFrame(
            const byte_vector& frameData,
            const Resolution& resolution,
            uint32_t format,
            uint64_t timestamp
        ) = 0;

        virtual Result requestKeyFrame() = 0;
        virtual Result flush() = 0;

        virtual Result setBitrate(uint32_t bitrate) = 0;
        virtual Result setFrameRate(uint32_t fps) = 0;
        virtual Result setKeyFrameInterval(uint32_t intervalSeconds) = 0;

        virtual VideoConfig getCurrentConfig() const = 0;
        virtual std::string getEncoderName() const = 0;
        virtual std::string getEncoderVersion() const = 0;
        virtual bool isHardwareAccelerated() const = 0;

        virtual std::vector<Resolution> getSupportedResolutions() const = 0;
        virtual std::vector<uint32_t> getSupportedFormats() const = 0;
        virtual std::pair<uint32_t, uint32_t> getBitrateRange() const = 0;
        virtual std::pair<uint32_t, uint32_t> getFrameRateRange() const = 0;

        virtual EncoderStats getStats() const = 0;
        virtual void resetStats() = 0;

        virtual byte_vector getConfigurationData() const = 0;
    };

    class EncoderFactory
    {
    public:
        enum class EncoderType
        {
            MEDIACODEC_HARDWARE,
            MEDIACODEC_SOFTWARE,
            AUTO_DETECT
        };

        static std::unique_ptr<EncoderInterface> createEncoder(EncoderType type, VideoCodec codec);
        static std::vector<EncoderType> getAvailableEncoders(VideoCodec codec);
        static bool isEncoderSupported(EncoderType type, VideoCodec codec);
        static std::string getEncoderTypeName(EncoderType type);
        static std::vector<std::string> getAvailableCodecNames(VideoCodec codec);
    };
}

#endif // ANDROID_SERVER_VIDEO_ENCODER_ENCODER_INTERFACE_H
