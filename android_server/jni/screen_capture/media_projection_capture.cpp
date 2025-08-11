#include "media_projection_capture.h"
#include <chrono>
#include <cstring>

#include <media/NdkImageReader.h>
#include <media/NdkImage.h>

namespace android_server::screen_capture
{
    MediaProjectionCapture::MediaProjectionCapture()
        : m_resolution(1280, 720)
          , m_format(AIMAGE_FORMAT_RGBA_8888)
          , m_fps(30)
          , m_initialized(false)
          , m_capturing(false)
          , m_stopRequested(false)
          , m_imageReader(nullptr)
          , m_nativeWindow(nullptr)
          , m_totalFramesCaptured(0)
          , m_droppedFrames(0)
    {
    }

    MediaProjectionCapture::~MediaProjectionCapture()
    {
        MediaProjectionCapture::stop();
        destroyCapture();
    }

    Result MediaProjectionCapture::initialize(const Resolution& resolution)
    {
        if (m_initialized)
        {
            return Result::ERROR_ALREADY_RUNNING;
        }

        if (!checkPermissions())
        {
            return Result::ERROR_PERMISSION_DENIED;
        }

        m_resolution = resolution;

        Result result = createImageReader();
        if (result != Result::SUCCESS)
        {
            return result;
        }

        result = setupVirtualDisplay();
        if (result != Result::SUCCESS)
        {
            destroyCapture();
            return result;
        }

        m_initialized = true;

        return Result::SUCCESS;
    }

    Result MediaProjectionCapture::start()
    {
        if (!m_initialized)
        {
            return Result::ERROR_NOT_INITIALIZED;
        }

        if (m_capturing)
        {
            return Result::ERROR_ALREADY_RUNNING;
        }

        m_stopRequested = false;
        m_capturing = true;
        m_startTime = std::chrono::steady_clock::now();
        m_lastFrameTime = m_startTime;

        m_captureThread = std::make_unique<std::thread>(&MediaProjectionCapture::captureThreadFunc, this);

        return Result::SUCCESS;
    }

    Result MediaProjectionCapture::stop()
    {
        if (!m_capturing)
        {
            return Result::SUCCESS;
        }

        m_stopRequested = true;
        m_capturing = false;

        if (m_captureThread && m_captureThread->joinable())
        {
            m_captureThread->join();
            m_captureThread.reset();
        }

        return Result::SUCCESS;
    }

    bool MediaProjectionCapture::isCapturing() const
    {
        return m_capturing.load();
    }

    void MediaProjectionCapture::setFrameCallback(FrameCallback callback)
    {
        m_frameCallback = callback;
    }

    void MediaProjectionCapture::setErrorCallback(ErrorCallback callback)
    {
        m_errorCallback = callback;
    }

    Resolution MediaProjectionCapture::getCurrentResolution() const
    {
        return m_resolution;
    }

    std::vector<Resolution> MediaProjectionCapture::getSupportedResolutions() const
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

    uint32_t MediaProjectionCapture::getCurrentFormat() const
    {
        return m_format;
    }

    std::vector<uint32_t> MediaProjectionCapture::getSupportedFormats() const
    {
        return {
            AIMAGE_FORMAT_RGBA_8888,
            AIMAGE_FORMAT_RGB_888,
            AIMAGE_FORMAT_YUV_420_888
        };
    }

    Result MediaProjectionCapture::setResolution(const Resolution& resolution)
    {
        if (m_capturing)
        {
            return Result::ERROR_ALREADY_RUNNING;
        }

        m_resolution = resolution;
        return Result::SUCCESS;
    }

    Result MediaProjectionCapture::setFormat(uint32_t format)
    {
        if (m_capturing)
        {
            return Result::ERROR_ALREADY_RUNNING;
        }

        m_format = format;
        return Result::SUCCESS;
    }

    Result MediaProjectionCapture::setFrameRate(uint32_t fps)
    {
        m_fps = fps;
        return Result::SUCCESS;
    }

    uint64_t MediaProjectionCapture::getTotalFramesCaptured() const
    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        return m_totalFramesCaptured;
    }

    uint64_t MediaProjectionCapture::getDroppedFrames() const
    {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        return m_droppedFrames;
    }

    double MediaProjectionCapture::getAverageFrameRate() const
    {
        std::lock_guard<std::mutex> lock(m_statsMutex);

        if (m_totalFramesCaptured == 0)
        {
            return 0.0;
        }

        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime);

        if (duration.count() > 0)
        {
            return static_cast<double>(m_totalFramesCaptured) / (duration.count() / 1000.0);
        }

        return 0.0;
    }

    std::string MediaProjectionCapture::getCaptureName() const
    {
        return "MediaProjection";
    }

    std::string MediaProjectionCapture::getCaptureVersion() const
    {
        return "Android NDK";
    }

    bool MediaProjectionCapture::requiresRoot() const
    {
        return false;
    }

    bool MediaProjectionCapture::supportsHardwareAcceleration() const
    {
        return true;
    }

    Result MediaProjectionCapture::createImageReader()
    {
        media_status_t status = AImageReader_new(
            m_resolution.width,
            m_resolution.height,
            m_format,
            2,
            &m_imageReader
        );

        if (status != AMEDIA_OK || !m_imageReader)
        {
            return Result::ERROR_CAPTURE_FAILED;
        }

        AImageReader_ImageListener listener;
        listener.context = this;
        listener.onImageAvailable = onImageAvailable;

        status = AImageReader_setImageListener(m_imageReader, &listener);
        if (status != AMEDIA_OK)
        {
            return Result::ERROR_CAPTURE_FAILED;
        }

        status = AImageReader_getWindow(m_imageReader, &m_nativeWindow);
        if (status != AMEDIA_OK || !m_nativeWindow)
        {
            return Result::ERROR_CAPTURE_FAILED;
        }

        return Result::SUCCESS;
    }

    Result MediaProjectionCapture::setupVirtualDisplay()
    {
        return Result::SUCCESS;
    }

    void MediaProjectionCapture::destroyCapture()
    {
        if (m_imageReader)
        {
            AImageReader_delete(m_imageReader);
            m_imageReader = nullptr;
        }

        m_nativeWindow = nullptr;
        m_initialized = false;
    }

    void MediaProjectionCapture::captureThreadFunc()
    {
        while (!m_stopRequested)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / m_fps));

            if (m_stopRequested) break;

            updateStats();
        }
    }

    void MediaProjectionCapture::onImageAvailable(void* context, AImageReader* reader)
    {
        auto* capture = static_cast<MediaProjectionCapture*>(context);
        capture->handleImageAvailable();
    }

    void MediaProjectionCapture::handleImageAvailable()
    {
        if (!m_imageReader || !m_capturing)
        {
            return;
        }

        AImage* image = nullptr;
        media_status_t status = AImageReader_acquireLatestImage(m_imageReader, &image);

        if (status != AMEDIA_OK || !image)
        {
            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_droppedFrames++;
            return;
        }

        Result result = processImage(image);
        if (result != Result::SUCCESS)
        {
            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_droppedFrames++;

            if (m_errorCallback)
            {
                m_errorCallback(result, "Failed to process captured image");
            }
        }

        AImage_delete(image);
    }

    Result MediaProjectionCapture::processImage(AImage* image)
    {
        if (!image || !m_frameCallback)
        {
            return Result::ERROR_CAPTURE_FAILED;
        }

        byte_vector frameData = convertImageToBuffer(image);
        if (frameData.empty())
        {
            return Result::ERROR_CAPTURE_FAILED;
        }

        Resolution imageRes = getImageResolution(image);
        uint64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

        m_frameCallback(frameData, timestamp, imageRes);

        {
            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_totalFramesCaptured++;
        }

        return Result::SUCCESS;
    }

    byte_vector MediaProjectionCapture::convertImageToBuffer(AImage* image)
    {
        if (!image)
        {
            return {};
        }

        uint8_t* pixelData = nullptr;
        int dataLength = 0;
        media_status_t status = AImage_getPlaneData(image, 0, &pixelData, &dataLength);
        if (status != AMEDIA_OK)
        {
            return {};
        }

        int32_t width, height;
        AImage_getWidth(image, &width);
        AImage_getHeight(image, &height);

        size_t dataSize = static_cast<size_t>(dataLength);
        byte_vector buffer(dataSize);

        std::memcpy(buffer.data(), pixelData, dataSize);

        return buffer;
    }

    uint32_t MediaProjectionCapture::getImageFormat(AImage* image) const
    {
        int32_t format;
        AImage_getFormat(image, &format);
        return static_cast<uint32_t>(format);
    }

    Resolution MediaProjectionCapture::getImageResolution(AImage* image) const
    {
        int32_t width, height;
        AImage_getWidth(image, &width);
        AImage_getHeight(image, &height);
        return Resolution(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    }

    void MediaProjectionCapture::updateStats()
    {
        auto now = std::chrono::steady_clock::now();

        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_lastFrameTime = now;
    }

    bool MediaProjectionCapture::checkPermissions() const
    {
        return true;
    }

    Result MediaProjectionCapture::requestMediaProjection()
    {
        return Result::SUCCESS;
    }
}
