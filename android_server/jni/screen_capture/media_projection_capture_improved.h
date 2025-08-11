#ifndef MEDIA_PROJECTION_CAPTURE_IMPROVED_H
#define MEDIA_PROJECTION_CAPTURE_IMPROVED_H

#include "capture_interface.h"
#include "../common/types.h"
#include "../protocol/packet_types.h"
#include <android/native_window.h>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaProjection.h>
#include <thread>
#include <functional>
#include <atomic>

namespace android_server::screen_capture
{
    class MediaProjectionCaptureImproved : public CaptureInterface
    {
    public:
        using FrameCallback = std::function<void(const protocol::EncodedFrame &)>;

        MediaProjectionCaptureImproved();
        ~MediaProjectionCaptureImproved() override;

        Result initialize(int width, int height, int dpi) override;
        Result startCapture() override;
        Result stopCapture() override;
        bool isCapturing() const override { return m_isCapturing.load(); }

        void setMediaProjection(AMediaProjection* projection);
        void setFrameCallback(FrameCallback callback);

        void setTargetFPS(float fps) { m_targetFPS = fps; }
        void setKeyFrameInterval(int interval) { m_keyFrameInterval = interval; }
        void setBitrateMode(int mode) { m_bitrateMode = mode; }

    private:
        Result createVirtualDisplay();
        bool createEncoderSurface();
        int calculateOptimalBitrate();

        void encodingLoop();
        void processEncodedFrame(ssize_t bufferIndex, const AMediaCodecBufferInfo& bufferInfo);

        AMediaProjection* m_mediaProjection;
        AVirtualDisplay* m_virtualDisplay;
        ANativeWindow* m_surface;
        AMediaCodec* m_mediaCodec;

        std::atomic<bool> m_isCapturing;
        std::thread m_encodingThread;
        FrameCallback m_frameCallback;

        int m_width;
        int m_height;
        int m_dpi;
        float m_targetFPS = 60.0f;
        int m_keyFrameInterval = 10;
        int m_bitrateMode = AMEDIACODEC_BITRATE_MODE_VBR;

        uint64_t m_frameCount = 0;
        uint64_t m_keyFrameCount = 0;
    };
}

#endif // MEDIA_PROJECTION_CAPTURE_IMPROVED_H
