#include "media_projection_capture_improved.h"
#include "../common/logger.h"
#include <android/native_window.h>
#include <media/NdkMediaFormat.h>
#include <cstring>

namespace android_server::screen_capture
{
    MediaProjectionCaptureImproved::MediaProjectionCaptureImproved()
        : m_mediaProjection(nullptr)
          , m_virtualDisplay(nullptr)
          , m_surface(nullptr)
          , m_mediaCodec(nullptr)
          , m_isCapturing(false)
          , m_frameCallback(nullptr)
          , m_width(0)
          , m_height(0)
          , m_dpi(0)
    {
    }

    MediaProjectionCaptureImproved::~MediaProjectionCaptureImproved()
    {
        stopCapture();
    }

    Result MediaProjectionCaptureImproved::initialize(int width, int height, int dpi)
    {
        m_width = width;
        m_height = height;
        m_dpi = dpi;

        return Result::SUCCESS;
    }

    Result MediaProjectionCaptureImproved::createVirtualDisplay()
    {
        if (!m_mediaProjection)
        {
            return Result::ERROR_INIT_FAILED;
        }

        if (!createEncoderSurface())
        {
            return Result::ERROR_INIT_FAILED;
        }

        m_virtualDisplay = AMediaProjection_createVirtualDisplay(
            m_mediaProjection,
            "MultiDeviceController",
            m_width,
            m_height,
            m_dpi,
            AVIRTUAL_DISPLAY_FLAG_AUTO_MIRROR,
            m_surface,
            nullptr,
            nullptr
        );

        if (!m_virtualDisplay)
        {
            return Result::ERROR_INIT_FAILED;
        }

        return Result::SUCCESS;
    }

    bool MediaProjectionCaptureImproved::createEncoderSurface()
    {
        AMediaFormat* format = AMediaFormat_new();
        AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/avc");
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, m_width);
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, m_height);
        AMediaFormat_setInt32(
            format,
            AMEDIAFORMAT_KEY_COLOR_FORMAT,
            AMEDIACODEC_COLOR_FormatSurface
        );

        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_BIT_RATE, calculateOptimalBitrate());
        AMediaFormat_setFloat(format, AMEDIAFORMAT_KEY_FRAME_RATE, 60.0f);
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 10);

        AMediaFormat_setInt32(
            format,
            AMEDIAFORMAT_KEY_PROFILE,
            AMEDIACODEC_PROFILE_AVCHigh
        );
        AMediaFormat_setInt32(
            format,
            AMEDIAFORMAT_KEY_LEVEL,
            AMEDIACODEC_LEVEL_AVCLevel4
        );

        m_mediaCodec = AMediaCodec_createEncoderByType("video/avc");
        if (!m_mediaCodec)
        {
            AMediaFormat_delete(format);
            return false;
        }

        media_status_t status = AMediaCodec_configure(
            m_mediaCodec,
            format,
            nullptr,
            nullptr,
            AMEDIACODEC_CONFIGURE_FLAG_ENCODE
        );
        if (status != AMEDIA_OK)
        {
            AMediaFormat_delete(format);
            return false;
        }

        status = AMediaCodec_createInputSurface(m_mediaCodec, &m_surface);
        if (status != AMEDIA_OK)
        {
            AMediaFormat_delete(format);
            return false;
        }

        AMediaFormat_delete(format);
        return true;
    }

    int MediaProjectionCaptureImproved::calculateOptimalBitrate()
    {
        int pixels = m_width * m_height;

        if (pixels <= 1280 * 720)
        {
            return 2000000;
        }
        else if (pixels <= 1920 * 1080)
        {
            return 4000000;
        }
        else
        {
            return 8000000;
        }
    }

    Result MediaProjectionCaptureImproved::startCapture()
    {
        if (m_isCapturing)
        {
            return Result::SUCCESS;
        }

        media_status_t status = AMediaCodec_start(m_mediaCodec);
        if (status != AMEDIA_OK)
        {
            return Result::ERROR_NOT_RUNNING;
        }

        m_isCapturing = true;

        m_encodingThread = std::thread(&MediaProjectionCaptureImproved::encodingLoop, this);

        return Result::SUCCESS;
    }

    Result MediaProjectionCaptureImproved::stopCapture()
    {
        if (!m_isCapturing)
        {
            return Result::SUCCESS;
        }

        m_isCapturing = false;

        if (m_encodingThread.joinable())
        {
            m_encodingThread.join();
        }

        if (m_mediaCodec)
        {
            AMediaCodec_stop(m_mediaCodec);
            AMediaCodec_delete(m_mediaCodec);
            m_mediaCodec = nullptr;
        }

        if (m_virtualDisplay)
        {
            AVirtualDisplay_release(m_virtualDisplay);
            m_virtualDisplay = nullptr;
        }

        if (m_surface)
        {
            ANativeWindow_release(m_surface);
            m_surface = nullptr;
        }

        return Result::SUCCESS;
    }

    void MediaProjectionCaptureImproved::encodingLoop()
    {
        LOG_I("Encoding loop started");

        AMediaCodecBufferInfo bufferInfo;

        while (m_isCapturing)
        {
            ssize_t bufferIndex = AMediaCodec_dequeueOutputBuffer(
                m_mediaCodec,
                &bufferInfo,
                10000
            );

            if (bufferIndex >= 0)
            {
                processEncodedFrame(bufferIndex, bufferInfo);
                AMediaCodec_releaseOutputBuffer(m_mediaCodec, bufferIndex, false);
            }
            else if (bufferIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED)
            {
                AMediaFormat* format = AMediaCodec_getOutputFormat(m_mediaCodec);
                LOG_D("Output format changed");
                AMediaFormat_delete(format);
            }
        }

        LOG_I("Encoding loop finished");
    }

    void MediaProjectionCaptureImproved::processEncodedFrame(
        ssize_t bufferIndex,
        const AMediaCodecBufferInfo& bufferInfo
    )
    {
        size_t bufferSize;
        uint8_t* buffer = AMediaCodec_getOutputBuffer(m_mediaCodec, bufferIndex, &bufferSize);

        if (!buffer || bufferInfo.size == 0)
        {
            return;
        }

        bool isKeyframe = (bufferInfo.flags & AMEDIACODEC_BUFFER_FLAG_SYNC_FRAME) != 0;

        if (m_frameCallback)
        {
            protocol::EncodedFrame frame;
            frame.data = byte_vector(
                buffer + bufferInfo.offset,
                buffer + bufferInfo.offset + bufferInfo.size
            );
            frame.timestamp = static_cast<uint64_t>(bufferInfo.presentationTimeUs);
            frame.isKeyFrame = isKeyframe;

            m_frameCallback(frame);
        }
    }

    void MediaProjectionCaptureImproved::setFrameCallback(FrameCallback callback)
    {
        m_frameCallback = callback;
    }

    void MediaProjectionCaptureImproved::setMediaProjection(AMediaProjection* projection)
    {
        m_mediaProjection = projection;
    }
}
