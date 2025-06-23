#include "screen_capture_service.h"
#include "adb_command_executor.h"
#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QTimer>
#include <QThread>
#include <QImage>

namespace infrastructure::adb
{
    ScreenCaptureService::ScreenCaptureService(QObject* parent)
        : QObject(parent)
        , m_adbExecutor(std::make_unique<AdbCommandExecutor>(this))
    {
        connect(m_adbExecutor.get(), &AdbCommandExecutor::commandExecuted,
                this, &ScreenCaptureService::onAdbCommandFinished);
        connect(m_adbExecutor.get(), &AdbCommandExecutor::errorOccurred,
                this, [this](const QString& deviceId, const QString& error) {
                    emit captureError(deviceId, error);
                });
    }

    ScreenCaptureService::~ScreenCaptureService()
    {
        stopAllCaptures();
    }

    bool ScreenCaptureService::startScreenCapture(const QString& deviceId, int intervalMs)
    {
        if (isDeviceCapturing(deviceId))
        {
            qWarning() << "Device" << deviceId << "is already being captured";
            return false;
        }

        if (intervalMs <= 0)
        {
            intervalMs = 100;
        }

        m_captureIntervals[deviceId] = intervalMs;
        m_captureQualities[deviceId] = 80;
        m_captureResolutions[deviceId] = QSize(1080, 1920);

        auto timer = new QTimer(this);
        timer->setInterval(intervalMs);
        timer->setSingleShot(false);
        
        connect(timer, &QTimer::timeout, this, [this, deviceId]() {
            executeScreenCapture(deviceId);
        });

        m_captureTimers[deviceId] = timer;
        timer->start();

        emit captureStarted(deviceId);
        
        return true;
    }

    bool ScreenCaptureService::stopScreenCapture(const QString& deviceId)
    {
        if (!isDeviceCapturing(deviceId))
        {
            return false;
        }

        cleanupDevice(deviceId);
        emit captureStopped(deviceId);
        
        return true;
    }

    bool ScreenCaptureService::startMultiDeviceCapture(const QList<QString>& deviceIds, int intervalMs)
    {
        bool allStarted = true;
        
        for (const QString& deviceId : deviceIds)
        {
            if (!startScreenCapture(deviceId, intervalMs))
            {
                allStarted = false;
            }
        }

        if (allStarted)
        {
            emit multiDeviceCaptureStarted(deviceIds);
        }

        return allStarted;
    }

    void ScreenCaptureService::stopAllCaptures()
    {
        const auto deviceIds = m_captureTimers.keys();
        
        for (const QString& deviceId : deviceIds)
        {
            stopScreenCapture(deviceId);
        }

        if (m_adbExecutor)
        {
            m_adbExecutor->stopAllProcesses();
        }

        emit multiDeviceCaptureStopped();
    }

    bool ScreenCaptureService::isDeviceCapturing(const QString& deviceId) const
    {
        return m_captureTimers.contains(deviceId) && m_captureTimers[deviceId]->isActive();
    }

    QList<QString> ScreenCaptureService::getCapturingDevices() const
    {
        QList<QString> devices;
        
        for (auto it = m_captureTimers.begin(); it != m_captureTimers.end(); ++it)
        {
            if (it.value()->isActive())
            {
                devices.append(it.key());
            }
        }
        
        return devices;
    }

    QImage ScreenCaptureService::getLastCapturedImage(const QString& deviceId) const
    {
        return m_lastCapturedImages.value(deviceId);
    }

    void ScreenCaptureService::setCaptureInterval(const QString& deviceId, int intervalMs)
    {
        if (intervalMs <= 0)
        {
            return;
        }

        m_captureIntervals[deviceId] = intervalMs;
        
        if (auto timer = m_captureTimers.value(deviceId))
        {
            timer->setInterval(intervalMs);
        }
    }

    void ScreenCaptureService::setCaptureQuality(const QString& deviceId, int quality)
    {
        if (quality < 1 || quality > 100)
        {
            return;
        }
        
        m_captureQualities[deviceId] = quality;
    }

    void ScreenCaptureService::setCaptureResolution(const QString& deviceId, const QSize& resolution)
    {
        if (resolution.width() <= 0 || resolution.height() <= 0)
        {
            return;
        }
        
        m_captureResolutions[deviceId] = resolution;
    }

    void ScreenCaptureService::onCaptureTimer()
    {
    }

    void ScreenCaptureService::onAdbCommandFinished(const QString& deviceId, const QString& command, bool success, const QString& output)
    {
        if (!success)
        {
            emit captureError(deviceId, output);
            return;
        }

        if (output.startsWith("PNG") || output.contains("\x89PNG"))
        {
            processCapturedImage(deviceId, output.toUtf8());
        }
        else
        {
            emit captureError(deviceId, "Invalid image data received");
        }
    }

    void ScreenCaptureService::executeScreenCapture(const QString& deviceId)
    {
        m_adbExecutor->executeScreenCaptureAsync(deviceId, 
            [this, deviceId](bool success, const QByteArray& output) {
                if (success)
                {
                    processCapturedImage(deviceId, output);
                }
                else
                {
                    emit captureError(deviceId, "Failed to capture screen");
                }
            });
    }

    void ScreenCaptureService::processCapturedImage(const QString& deviceId, const QByteArray& imageData)
    {
        QImage image;
        if (image.loadFromData(imageData))
        {
            const QSize targetResolution = m_captureResolutions.value(deviceId, QSize(1080, 1920));
            if (image.size() != targetResolution)
            {
                image = image.scaled(targetResolution, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }

            m_lastCapturedImages[deviceId] = image;
            emit screenCaptured(deviceId, image);
        }
        else
        {
            qWarning() << "Failed to load image data for device:" << deviceId << "data size:" << imageData.size();
            emit captureError(deviceId, "Failed to load captured image data");
        }
    }

    void ScreenCaptureService::cleanupDevice(const QString& deviceId)
    {
        if (auto timer = m_captureTimers.take(deviceId))
        {
            timer->stop();
            timer->deleteLater();
        }

        m_lastCapturedImages.remove(deviceId);
        m_captureIntervals.remove(deviceId);
        m_captureQualities.remove(deviceId);
        m_captureResolutions.remove(deviceId);
    }
} 