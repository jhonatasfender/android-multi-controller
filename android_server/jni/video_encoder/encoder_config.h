#ifndef ANDROID_SERVER_VIDEO_ENCODER_ENCODER_CONFIG_H
#define ANDROID_SERVER_VIDEO_ENCODER_ENCODER_CONFIG_H

#include "../common/types.h"
#include <string>
#include <vector>
#include <map>

namespace android_server::video_encoder
{
    enum class H264Profile : uint32_t
    {
        BASELINE = 66,
        MAIN = 77,
        HIGH = 100
    };

    enum class H264Level : uint32_t
    {
        LEVEL_1 = 10,
        LEVEL_1_1 = 11,
        LEVEL_1_2 = 12,
        LEVEL_1_3 = 13,
        LEVEL_2 = 20,
        LEVEL_2_1 = 21,
        LEVEL_2_2 = 22,
        LEVEL_3 = 30,
        LEVEL_3_1 = 31,
        LEVEL_3_2 = 32,
        LEVEL_4 = 40,
        LEVEL_4_1 = 41,
        LEVEL_4_2 = 42,
        LEVEL_5 = 50,
        LEVEL_5_1 = 51,
        LEVEL_5_2 = 52
    };

    enum class BitrateMode : uint32_t
    {
        CONSTANT = 0,
        VARIABLE = 1,
        ADAPTIVE = 2
    };

    enum class ColorFormat : uint32_t
    {
        YUV420_PLANAR = 19,
        YUV420_SEMI_PLANAR = 21,
        RGBA_8888 = 0x7F000001,
        BGRA_8888 = 0x7F000002,
        ARGB_8888 = 0x7F000003
    };

    struct H264EncoderConfig
    {
        Resolution resolution;
        uint32_t bitrate;
        uint32_t fps;
        uint32_t keyFrameInterval;
        H264Profile profile;
        H264Level level;
        BitrateMode bitrateMode;
        ColorFormat colorFormat;

        bool enableBFrames;
        uint32_t bFrameCount;
        uint32_t gopSize;

        bool enableCabac;
        bool enable8x8Transform;

        uint32_t quality;
        uint32_t complexity;

        bool enableLowLatency;
        bool enableRealtime;

        uint32_t maxBitrate;
        uint32_t minBitrate;

        std::string encoderName;
        bool forceHardware;
        bool allowSoftwareFallback;

        H264EncoderConfig()
            : resolution(1280, 720)
              , bitrate(4000000)
              , fps(30)
              , keyFrameInterval(2)
              , profile(H264Profile::BASELINE)
              , level(H264Level::LEVEL_3_1)
              , bitrateMode(BitrateMode::VARIABLE)
              , colorFormat(ColorFormat::YUV420_SEMI_PLANAR)
              , enableBFrames(false)
              , bFrameCount(0)
              , gopSize(30)
              , enableCabac(false)
              , enable8x8Transform(false)
              , quality(50)
              , complexity(0)
              , enableLowLatency(true)
              , enableRealtime(true)
              , maxBitrate(0)
              , minBitrate(0)
              , forceHardware(false)
              , allowSoftwareFallback(true)
        {
        }
    };

    struct EncoderCapabilities
    {
        std::string name;
        bool isHardware;
        bool supportsAdaptiveBitrate;
        bool supportsLowLatency;

        std::vector<Resolution> supportedResolutions;
        std::vector<ColorFormat> supportedColorFormats;
        std::vector<H264Profile> supportedProfiles;
        std::vector<H264Level> supportedLevels;

        uint32_t maxBitrate;
        uint32_t minBitrate;
        uint32_t maxFps;
        uint32_t minFps;

        uint32_t maxWidth;
        uint32_t maxHeight;
        uint32_t minWidth;
        uint32_t minHeight;

        uint32_t alignment;

        EncoderCapabilities()
            : isHardware(false)
              , supportsAdaptiveBitrate(false)
              , supportsLowLatency(false)
              , maxBitrate(0)
              , minBitrate(0)
              , maxFps(0)
              , minFps(0)
              , maxWidth(0)
              , maxHeight(0)
              , minWidth(0)
              , minHeight(0)
              , alignment(16)
        {
        }
    };

    class EncoderConfigManager
    {
    public:
        static EncoderConfigManager& getInstance();

        std::vector<EncoderCapabilities> getAvailableEncoders() const;
        EncoderCapabilities getBestEncoder(const H264EncoderConfig& requirements) const;

        H264EncoderConfig optimizeConfig(const H264EncoderConfig& config,
                                         const EncoderCapabilities& capabilities) const;

        bool validateConfig(const H264EncoderConfig& config,
                            const EncoderCapabilities& capabilities) const;

        H264Level calculateMinimumLevel(const Resolution& resolution, uint32_t fps) const;
        uint32_t calculateMaxBitrate(const Resolution& resolution, uint32_t fps) const;

        VideoConfig toVideoConfig(const H264EncoderConfig& config) const;
        H264EncoderConfig fromVideoConfig(const VideoConfig& config) const;

        std::string profileToString(H264Profile profile) const;
        std::string levelToString(H264Level level) const;
        std::string colorFormatToString(ColorFormat format) const;
        std::string bitrateModeToString(BitrateMode mode) const;

    private:
        EncoderConfigManager() = default;

        void loadEncoderCapabilities();
        EncoderCapabilities queryEncoderCapabilities(const std::string& codecName) const;

        mutable std::map<std::string, EncoderCapabilities> m_encoderCache;
        mutable bool m_capabilitiesLoaded = false;
    };
}

#endif // ANDROID_SERVER_VIDEO_ENCODER_ENCODER_CONFIG_H
