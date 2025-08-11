#ifndef ANDROID_SERVER_VIDEO_ENCODER_MEDIACODEC_H264_ENCODER_H
#define ANDROID_SERVER_VIDEO_ENCODER_MEDIACODEC_H264_ENCODER_H

#include "encoder_interface.h"
#include "../common/types.h"
#include "../common/logger.h"

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace android_server {
namespace video_encoder {

struct InputFrame {
    byte_vector data;
    Resolution resolution;
    uint32_t format;
    uint64_t timestamp;
    
    InputFrame(const byte_vector& frameData, const Resolution& res, uint32_t fmt, uint64_t ts)
        : data(frameData), resolution(res), format(fmt), timestamp(ts) {}
};

class MediaCodecH264Encoder : public EncoderInterface {
public:
    MediaCodecH264Encoder();
    ~MediaCodecH264Encoder() override;

    Result initialize(const VideoConfig& config) override;
    Result start() override;
    Result stop() override;
    bool isEncoding() const override;

    void setEncodedDataCallback(EncodedDataCallback callback) override;
    void setErrorCallback(EncoderErrorCallback callback) override;

    Result encodeFrame(const byte_vector& frameData, 
                      const Resolution& resolution,
                      uint32_t format,
                      uint64_t timestamp) override;

    Result requestKeyFrame() override;
    Result flush() override;

    Result setBitrate(uint32_t bitrate) override;
    Result setFrameRate(uint32_t fps) override;
    Result setKeyFrameInterval(uint32_t intervalSeconds) override;

    VideoConfig getCurrentConfig() const override;
    std::string getEncoderName() const override;
    std::string getEncoderVersion() const override;
    bool isHardwareAccelerated() const override;

    std::vector<Resolution> getSupportedResolutions() const override;
    std::vector<uint32_t> getSupportedFormats() const override;
    std::pair<uint32_t, uint32_t> getBitrateRange() const override;
    std::pair<uint32_t, uint32_t> getFrameRateRange() const override;

    EncoderStats getStats() const override;
    void resetStats() override;

    byte_vector getConfigurationData() const override;

private:
    VideoConfig m_config;
    std::atomic<bool> m_initialized;
    std::atomic<bool> m_encoding;
    std::atomic<bool> m_stopRequested;

    AMediaCodec* m_codec;
    AMediaFormat* m_format;
    std::string m_codecName;
    bool m_isHardwareAccelerated;

    std::unique_ptr<std::thread> m_encodingThread;
    std::unique_ptr<std::thread> m_outputThread;

    mutable std::mutex m_inputQueueMutex;
    std::condition_variable m_inputQueueCondition;
    std::queue<std::unique_ptr<InputFrame>> m_inputQueue;
    static constexpr size_t MAX_INPUT_QUEUE_SIZE = 10;

    mutable std::mutex m_configMutex;
    byte_vector m_spsData;
    byte_vector m_ppsData;
    bool m_configurationSent;

    EncodedDataCallback m_encodedDataCallback;
    EncoderErrorCallback m_errorCallback;

    mutable std::mutex m_statsMutex;
    EncoderStats m_stats;
    std::chrono::steady_clock::time_point m_startTime;
    uint64_t m_lastFrameTime;

    Result createMediaCodec();
    Result configureEncoder();
    Result startEncoder();
    Result stopEncoder();
    void destroyEncoder();

    void encodingThreadFunc();
    void outputThreadFunc();

    Result processInputFrame();
    Result processOutputBuffer();

    void handleConfigurationBuffer(uint8_t* data, size_t size, uint64_t timestamp);
    void handleEncodedFrame(uint8_t* data, size_t size, uint64_t timestamp, bool keyframe);

    bool extractSpsLps(uint8_t* data, size_t size);
    void updateStats(size_t frameSize, bool keyframe);

    static media_status_t onAsyncNotify(AMediaCodec* codec, void* userdata);
    void onCodecNotify();

    std::vector<std::string> getAvailableEncoders() const;
    bool isEncoderHardwareAccelerated(const std::string& codecName) const;
    Result selectBestEncoder();

    uint32_t colorFormatToMediaFormat(uint32_t format) const;
    std::string mediaFormatToString(uint32_t format) const;
};

} // namespace video_encoder
} // namespace android_server

#endif // ANDROID_SERVER_VIDEO_ENCODER_MEDIACODEC_H264_ENCODER_H 