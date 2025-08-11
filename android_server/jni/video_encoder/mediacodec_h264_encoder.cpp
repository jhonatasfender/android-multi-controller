#include "mediacodec_h264_encoder.h"
#include <chrono>

namespace android_server::video_encoder
{
    MediaCodecH264Encoder::MediaCodecH264Encoder()
        : m_initialized(false), m_encoding(false), m_stopRequested(false)
          , m_codec(nullptr), m_format(nullptr), m_isHardwareAccelerated(false)
          , m_configurationSent(false), m_lastFrameTime(0)
    {
    }

    MediaCodecH264Encoder::~MediaCodecH264Encoder()
    {
        MediaCodecH264Encoder::stop();
        destroyEncoder();
    }

    Result MediaCodecH264Encoder::initialize(const VideoConfig& config)
    {
        if (m_initialized) return Result::ERROR_ALREADY_RUNNING;

        m_config = config;

        if (createMediaCodec() != Result::SUCCESS) return Result::ERROR_ENCODER_FAILED;
        if (configureEncoder() != Result::SUCCESS)
        {
            destroyEncoder();
            return Result::ERROR_ENCODER_FAILED;
        }

        m_initialized = true;
        resetStats();

        return Result::SUCCESS;
    }

    Result MediaCodecH264Encoder::start()
    {
        if (!m_initialized) return Result::ERROR_NOT_INITIALIZED;
        if (m_encoding) return Result::ERROR_ALREADY_RUNNING;

        if (startEncoder() != Result::SUCCESS) return Result::ERROR_ENCODER_FAILED;

        m_stopRequested = false;
        m_encoding = true;
        m_startTime = std::chrono::steady_clock::now();

        m_encodingThread = std::make_unique<std::thread>(&MediaCodecH264Encoder::encodingThreadFunc, this);
        m_outputThread = std::make_unique<std::thread>(&MediaCodecH264Encoder::outputThreadFunc, this);

        return Result::SUCCESS;
    }

    Result MediaCodecH264Encoder::stop()
    {
        if (!m_encoding) return Result::SUCCESS;

        m_stopRequested = true;
        m_encoding = false;
        m_inputQueueCondition.notify_all();

        if (m_encodingThread && m_encodingThread->joinable()) m_encodingThread->join();
        if (m_outputThread && m_outputThread->joinable()) m_outputThread->join();

        return stopEncoder();
    }

    bool MediaCodecH264Encoder::isEncoding() const
    {
        return m_encoding.load();
    }

    void MediaCodecH264Encoder::setEncodedDataCallback(EncodedDataCallback callback)
    {
        m_encodedDataCallback = callback;
    }

    void MediaCodecH264Encoder::setErrorCallback(EncoderErrorCallback callback)
    {
        m_errorCallback = callback;
    }

    Result MediaCodecH264Encoder::encodeFrame(const byte_vector& frameData,
                                              const Resolution& resolution,
                                              uint32_t format, uint64_t timestamp)
    {
        if (!m_encoding) return Result::ERROR_NOT_RUNNING;

        std::lock_guard lock(m_inputQueueMutex);
        if (m_inputQueue.size() >= MAX_INPUT_QUEUE_SIZE)
        {
            m_stats.droppedFrames++;
            return Result::ERROR_ENCODER_FAILED;
        }

        auto frame = std::make_unique<InputFrame>(frameData, resolution, format, timestamp);
        m_inputQueue.push(std::move(frame));
        m_inputQueueCondition.notify_one();

        return Result::SUCCESS;
    }

    VideoConfig MediaCodecH264Encoder::getCurrentConfig() const
    {
        return m_config;
    }

    std::string MediaCodecH264Encoder::getEncoderName() const
    {
        return m_codecName;
    }

    bool MediaCodecH264Encoder::isHardwareAccelerated() const
    {
        return m_isHardwareAccelerated;
    }

    EncoderStats MediaCodecH264Encoder::getStats() const
    {
        std::lock_guard lock(m_statsMutex);
        return m_stats;
    }

    void MediaCodecH264Encoder::resetStats()
    {
        std::lock_guard lock(m_statsMutex);
        m_stats = EncoderStats();
    }

    byte_vector MediaCodecH264Encoder::getConfigurationData() const
    {
        byte_vector configData;
        configData.insert(configData.end(), m_spsData.begin(), m_spsData.end());
        configData.insert(configData.end(), m_ppsData.begin(), m_ppsData.end());
        return configData;
    }

    Result MediaCodecH264Encoder::createMediaCodec()
    {
        m_codec = AMediaCodec_createEncoderByType("video/avc");
        if (!m_codec) return Result::ERROR_ENCODER_FAILED;

        m_codecName = "MediaCodec H.264";
        m_isHardwareAccelerated = true;
        return Result::SUCCESS;
    }

    Result MediaCodecH264Encoder::configureEncoder()
    {
        m_format = AMediaFormat_new();
        AMediaFormat_setString(m_format, AMEDIAFORMAT_KEY_MIME, "video/avc");
        AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_WIDTH, m_config.resolution.width);
        AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_HEIGHT, m_config.resolution.height);
        AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_BIT_RATE, m_config.bitrate);
        AMediaFormat_setFloat(m_format, AMEDIAFORMAT_KEY_FRAME_RATE, m_config.fps);
        AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, m_config.keyFrameInterval);
        AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_COLOR_FORMAT, 21);

        return AMediaCodec_configure(
                   m_codec,
                   m_format,
                   nullptr,
                   nullptr,
                   AMEDIACODEC_CONFIGURE_FLAG_ENCODE
               ) ==
               AMEDIA_OK
                   ? Result::SUCCESS
                   : Result::ERROR_ENCODER_FAILED;
    }

    Result MediaCodecH264Encoder::startEncoder()
    {
        return AMediaCodec_start(m_codec) == AMEDIA_OK ? Result::SUCCESS : Result::ERROR_ENCODER_FAILED;
    }

    Result MediaCodecH264Encoder::stopEncoder()
    {
        return AMediaCodec_stop(m_codec) == AMEDIA_OK ? Result::SUCCESS : Result::ERROR_ENCODER_FAILED;
    }

    void MediaCodecH264Encoder::destroyEncoder()
    {
        if (m_format)
        {
            AMediaFormat_delete(m_format);
            m_format = nullptr;
        }
        if (m_codec)
        {
            AMediaCodec_delete(m_codec);
            m_codec = nullptr;
        }
        m_initialized = false;
    }

    void MediaCodecH264Encoder::encodingThreadFunc()
    {
        while (!m_stopRequested)
        {
            processInputFrame();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void MediaCodecH264Encoder::outputThreadFunc()
    {
        while (!m_stopRequested)
        {
            processOutputBuffer();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    Result MediaCodecH264Encoder::processInputFrame()
    {
        std::unique_ptr<InputFrame> frame;
        {
            std::unique_lock<std::mutex> lock(m_inputQueueMutex);
            if (m_inputQueue.empty())
            {
                m_inputQueueCondition.wait_for(lock, std::chrono::milliseconds(10));
                if (m_inputQueue.empty()) return Result::ERROR_NOT_RUNNING;
            }
            frame = std::move(m_inputQueue.front());
            m_inputQueue.pop();
        }

        ssize_t bufferIndex = AMediaCodec_dequeueInputBuffer(m_codec, 10000);
        if (bufferIndex < 0) return Result::ERROR_ENCODER_FAILED;

        size_t bufferSize;
        uint8_t* buffer = AMediaCodec_getInputBuffer(m_codec, bufferIndex, &bufferSize);
        if (!buffer) return Result::ERROR_ENCODER_FAILED;

        size_t frameSize = std::min(frame->data.size(), bufferSize);
        std::memcpy(buffer, frame->data.data(), frameSize);

        return AMediaCodec_queueInputBuffer(m_codec, bufferIndex, 0, frameSize, frame->timestamp, 0) == AMEDIA_OK
                   ? Result::SUCCESS
                   : Result::ERROR_ENCODER_FAILED;
    }

    Result MediaCodecH264Encoder::processOutputBuffer()
    {
        AMediaCodecBufferInfo bufferInfo;
        ssize_t bufferIndex = AMediaCodec_dequeueOutputBuffer(m_codec, &bufferInfo, 10000);

        if (bufferIndex < 0) return Result::ERROR_NOT_RUNNING;

        size_t bufferSize;
        uint8_t* buffer = AMediaCodec_getOutputBuffer(m_codec, bufferIndex, &bufferSize);
        if (!buffer || bufferInfo.size == 0)
        {
            AMediaCodec_releaseOutputBuffer(m_codec, bufferIndex, false);
            return Result::ERROR_ENCODER_FAILED;
        }

        const bool isKeyframe = (bufferInfo.flags & AMEDIACODEC_BUFFER_FLAG_CODEC_CONFIG) != 0;

        if (m_encodedDataCallback)
        {
            const byte_vector frameData(buffer, buffer + bufferInfo.size);
            m_encodedDataCallback(
                frameData,
                bufferInfo.presentationTimeUs,
                bufferInfo.presentationTimeUs,
                isKeyframe,
                false
            );
        }

        {
            std::lock_guard lock(m_statsMutex);
            m_stats.totalFramesEncoded++;
            m_stats.totalBytesEncoded += bufferInfo.size;
            if (isKeyframe) m_stats.totalKeyFrames++;
        }

        AMediaCodec_releaseOutputBuffer(m_codec, bufferIndex, false);
        return Result::SUCCESS;
    }

    Result MediaCodecH264Encoder::requestKeyFrame()
    {
        if (!m_codec || !m_encoding)
        {
            return Result::ERROR_NOT_RUNNING;
        }

        // Request keyframe through MediaCodec
        AMediaFormat* params = AMediaFormat_new();
        AMediaFormat_setInt32(params, "request-sync", 1);

        media_status_t status = AMediaCodec_setParameters(m_codec, params);
        AMediaFormat_delete(params);

        return (status == AMEDIA_OK) ? Result::SUCCESS : Result::ERROR_ENCODER_FAILED;
    }

    Result MediaCodecH264Encoder::flush()
    {
        if (!m_codec || !m_encoding)
        {
            return Result::ERROR_NOT_RUNNING;
        }

        media_status_t status = AMediaCodec_flush(m_codec);
        return (status == AMEDIA_OK) ? Result::SUCCESS : Result::ERROR_ENCODER_FAILED;
    }

    Result MediaCodecH264Encoder::setBitrate(uint32_t bitrate)
    {
        m_config.bitrate = bitrate;

        if (m_codec && m_encoding)
        {
            AMediaFormat* params = AMediaFormat_new();
            AMediaFormat_setInt32(params, AMEDIAFORMAT_KEY_BIT_RATE, static_cast<int32_t>(bitrate));

            media_status_t status = AMediaCodec_setParameters(m_codec, params);
            AMediaFormat_delete(params);

            return (status == AMEDIA_OK) ? Result::SUCCESS : Result::ERROR_ENCODER_FAILED;
        }

        return Result::SUCCESS;
    }

    Result MediaCodecH264Encoder::setFrameRate(uint32_t fps)
    {
        m_config.fps = fps;

        if (m_codec && m_encoding)
        {
            AMediaFormat* params = AMediaFormat_new();
            AMediaFormat_setFloat(params, AMEDIAFORMAT_KEY_FRAME_RATE, static_cast<float>(fps));

            media_status_t status = AMediaCodec_setParameters(m_codec, params);
            AMediaFormat_delete(params);

            return (status == AMEDIA_OK) ? Result::SUCCESS : Result::ERROR_ENCODER_FAILED;
        }

        return Result::SUCCESS;
    }

    Result MediaCodecH264Encoder::setKeyFrameInterval(uint32_t intervalSeconds)
    {
        m_config.keyFrameInterval = intervalSeconds;

        if (m_codec && m_encoding)
        {
            AMediaFormat* params = AMediaFormat_new();
            AMediaFormat_setInt32(params, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, static_cast<int32_t>(intervalSeconds));

            media_status_t status = AMediaCodec_setParameters(m_codec, params);
            AMediaFormat_delete(params);

            return (status == AMEDIA_OK) ? Result::SUCCESS : Result::ERROR_ENCODER_FAILED;
        }

        return Result::SUCCESS;
    }

    std::string MediaCodecH264Encoder::getEncoderVersion() const
    {
        return "Android MediaCodec NDK";
    }

    std::vector<Resolution> MediaCodecH264Encoder::getSupportedResolutions() const
    {
        return {
            Resolution(640, 480),
            Resolution(720, 480),
            Resolution(1280, 720),
            Resolution(1920, 1080),
            Resolution(2560, 1440),
            Resolution(3840, 2160)
        };
    }

    std::vector<uint32_t> MediaCodecH264Encoder::getSupportedFormats() const
    {
        return {
            21, // COLOR_FormatYUV420SemiPlanar
            19, // COLOR_FormatYUV420Planar
            0x7F000001 // COLOR_FormatRGBA8888
        };
    }

    std::pair<uint32_t, uint32_t> MediaCodecH264Encoder::getBitrateRange() const
    {
        return {64000, 100000000}; // 64 Kbps to 100 Mbps
    }

    std::pair<uint32_t, uint32_t> MediaCodecH264Encoder::getFrameRateRange() const
    {
        return {1, 120}; // 1 fps to 120 fps
    }
}
