#ifndef MULTI_DEVICE_MIRROR_VIEW_MODEL_H
#define MULTI_DEVICE_MIRROR_VIEW_MODEL_H

#include <QObject>
#include <QImage>
#include <memory>
#include "../../use_case/device_management_use_case.h"
#include "../../infrastructure/adb/screen_capture_service.h"
#include "device_image_provider.h"

namespace presentation::view_models
{
    class MultiDeviceMirrorViewModel final : public QObject
    {
        Q_OBJECT

    public:
        explicit MultiDeviceMirrorViewModel(
            const std::shared_ptr<use_case::DeviceManagementUseCase>& deviceUseCase,
            presentation::DeviceImageProvider* imageProvider,
            QObject* parent = nullptr
        );

        Q_INVOKABLE void startMirroring();
        Q_INVOKABLE void stopMirroring();
        Q_INVOKABLE void toggleMirroring();
        Q_INVOKABLE void setGridLayout(int columns, int rows);
        Q_INVOKABLE void setGridColumns(int columns);
        Q_INVOKABLE void setGridRows(int rows);
        Q_INVOKABLE void setCaptureInterval(int intervalMs);
        Q_INVOKABLE void setCaptureQuality(int quality);

        Q_PROPERTY(QList<QObject*> devices READ devices NOTIFY devicesChanged)
        Q_PROPERTY(bool isMirroring READ isMirroring NOTIFY isMirroringChanged)
        Q_PROPERTY(int gridColumns READ gridColumns WRITE setGridColumns NOTIFY gridColumnsChanged)
        Q_PROPERTY(int gridRows READ gridRows WRITE setGridRows NOTIFY gridRowsChanged)
        Q_PROPERTY(int captureInterval READ captureInterval WRITE setCaptureInterval NOTIFY captureIntervalChanged)
        Q_PROPERTY(int captureQuality READ captureQuality WRITE setCaptureQuality NOTIFY captureQualityChanged)
        Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

        QList<QObject*> devices() const;
        bool isMirroring() const;
        int gridColumns() const;
        int gridRows() const;
        int captureInterval() const;
        int captureQuality() const;
        QString errorMessage() const;

    signals:
        void devicesChanged();
        void isMirroringChanged();
        void gridColumnsChanged();
        void gridRowsChanged();
        void captureIntervalChanged();
        void captureQualityChanged();
        void errorMessageChanged();
        void screenCaptured(const QString& deviceId);
        void mirroringStarted();
        void mirroringStopped();

    public slots:
        void onDevicesUpdated();

    private slots:
        void onScreenCaptured(const QString& deviceId, const QImage& image);
        void onCaptureStarted(const QString& deviceId);
        void onCaptureStopped(const QString& deviceId);
        void onCaptureError(const QString& deviceId, const QString& error);
        void onMultiDeviceCaptureStarted(const QList<QString>& deviceIds);
        void onMultiDeviceCaptureStopped();

    private:
        std::shared_ptr<use_case::DeviceManagementUseCase> m_deviceUseCase;
        std::unique_ptr<infrastructure::adb::ScreenCaptureService> m_screenCaptureService;
        
        QList<QObject*> m_devices;
        bool m_isMirroring;
        int m_gridColumns;
        int m_gridRows;
        int m_captureInterval;
        int m_captureQuality;
        QString m_errorMessage;
        presentation::DeviceImageProvider* m_imageProvider;

        void updateDevices();
        void startScreenCaptureForConnectedDevices();
        void stopScreenCaptureForAllDevices();
        void updateErrorMessage(const QString& error);
    };
}

#endif 