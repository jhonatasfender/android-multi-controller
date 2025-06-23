#ifndef SCREEN_CAPTURE_SERVICE_H
#define SCREEN_CAPTURE_SERVICE_H

#include <QObject>
#include <QTimer>
#include <QImage>
#include <QMap>
#include <memory>
#include "../../core/entities/device.h"

namespace infrastructure::adb
{
    class AdbCommandExecutor;

    class ScreenCaptureService final : public QObject
    {
        Q_OBJECT

    public:
        explicit ScreenCaptureService(QObject* parent = nullptr);
        ~ScreenCaptureService() override;

        bool startScreenCapture(const QString& deviceId, int intervalMs = 100);
        bool stopScreenCapture(const QString& deviceId);
        bool startMultiDeviceCapture(const QList<QString>& deviceIds, int intervalMs = 100);
        void stopAllCaptures();
        bool isDeviceCapturing(const QString& deviceId) const;
        QList<QString> getCapturingDevices() const;
        QImage getLastCapturedImage(const QString& deviceId) const;
        void setCaptureInterval(const QString& deviceId, int intervalMs);
        void setCaptureQuality(const QString& deviceId, int quality = 80);
        void setCaptureResolution(const QString& deviceId, const QSize& resolution);

    signals:
        void screenCaptured(const QString& deviceId, const QImage& image);
        void captureStarted(const QString& deviceId);
        void captureStopped(const QString& deviceId);
        void captureError(const QString& deviceId, const QString& error);
        void multiDeviceCaptureStarted(const QList<QString>& deviceIds);
        void multiDeviceCaptureStopped();

    private slots:
        void onCaptureTimer();
        void onAdbCommandFinished(const QString& deviceId, const QString& command, bool success, const QString& output);

    private:
        std::unique_ptr<AdbCommandExecutor> m_adbExecutor;
        QMap<QString, QTimer*> m_captureTimers;
        QMap<QString, QImage> m_lastCapturedImages;
        QMap<QString, int> m_captureIntervals;
        QMap<QString, int> m_captureQualities;
        QMap<QString, QSize> m_captureResolutions;

        void executeScreenCapture(const QString& deviceId);
        void processCapturedImage(const QString& deviceId, const QByteArray& imageData);
        void cleanupDevice(const QString& deviceId);
    };
}

#endif