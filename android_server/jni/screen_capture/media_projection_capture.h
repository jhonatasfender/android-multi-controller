#ifndef ANDROID_SERVER_SCREEN_CAPTURE_MEDIA_PROJECTION_CAPTURE_H
#define ANDROID_SERVER_SCREEN_CAPTURE_MEDIA_PROJECTION_CAPTURE_H

#include "capture_interface.h"
#include "../common/types.h"
#include "../common/logger.h"

#include <media/NdkImageReader.h>
#include <android/native_window.h>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

namespace android_server::screen_capture
{
    class MediaProjectionCapture final : public CaptureInterface
    {
    public:
        MediaProjectionCapture();
        ~MediaProjectionCapture() override;

        Result initialize(const Resolution& resolution) override;
        Result start() override;
        Result stop() override;
        bool isCapturing() const override;

        void setFrameCallback(FrameCallback callback) override;
        void setErrorCallback(ErrorCallback callback) override;

        Resolution getCurrentResolution() const override;
        std::vector<Resolution> getSupportedResolutions() const override;
        uint32_t getCurrentFormat() const override;
        std::vector<uint32_t> getSupportedFormats() const override;

        Result setResolution(const Resolution& resolution) override;
        Result setFormat(uint32_t format) override;
        Result setFrameRate(uint32_t fps) override;

        uint64_t getTotalFramesCaptured() const override;
        uint64_t getDroppedFrames() const override;
        double getAverageFrameRate() const override;

        std::string getCaptureName() const override;
        std::string getCaptureVersion() const override;
        bool requiresRoot() const override;
        bool supportsHardwareAcceleration() const override;

    private:
        Resolution m_resolution;
        uint32_t m_format;
        uint32_t m_fps;

        std::atomic<bool> m_initialized;
        std::atomic<bool> m_capturing;
        std::atomic<bool> m_stopRequested;

        AImageReader* m_imageReader;
        ANativeWindow* m_nativeWindow;

        std::unique_ptr<std::thread> m_captureThread;

        FrameCallback m_frameCallback;
        ErrorCallback m_errorCallback;

        mutable std::mutex m_statsMutex;
        uint64_t m_totalFramesCaptured;
        uint64_t m_droppedFrames;
        std::chrono::steady_clock::time_point m_startTime;
        std::chrono::steady_clock::time_point m_lastFrameTime;

        Result createImageReader();
        Result setupVirtualDisplay();
        void destroyCapture();

        void captureThreadFunc();
        static void onImageAvailable(void* context, AImageReader* reader);
        void handleImageAvailable();

        Result processImage(AImage* image);
        byte_vector convertImageToBuffer(AImage* image);

        uint32_t getImageFormat(AImage* image) const;
        Resolution getImageResolution(AImage* image) const;

        void updateStats();

        bool checkPermissions() const;
        Result requestMediaProjection();
    };
}

#endif // ANDROID_SERVER_SCREEN_CAPTURE_MEDIA_PROJECTION_CAPTURE_H
