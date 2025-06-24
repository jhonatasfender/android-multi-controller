#include "multi_device_mirror_view_model.h"
#include "../../core/entities/device.h"
#include <QTimer>

namespace presentation::view_models
{
    MultiDeviceMirrorViewModel::MultiDeviceMirrorViewModel(
        const std::shared_ptr<use_case::DeviceManagementUseCase>& deviceUseCase,
        DeviceImageProvider* imageProvider,
        QObject* parent
    ) : QObject(parent)
        , m_deviceUseCase(deviceUseCase)
        , m_screenCaptureService(std::make_unique<infrastructure::adb::ScreenCaptureService>(this))
        , m_isMirroring(false)
        , m_gridColumns(2)
        , m_gridRows(2)
        , m_captureInterval(100)
        , m_captureQuality(80)
        , m_imageProvider(imageProvider)
    {
        connect(
            m_screenCaptureService.get(),
            &infrastructure::adb::ScreenCaptureService::screenCaptured,
            this,
            &MultiDeviceMirrorViewModel::onScreenCaptured
        );
        connect(
            m_screenCaptureService.get(),
            &infrastructure::adb::ScreenCaptureService::captureStarted,
            this,
            &MultiDeviceMirrorViewModel::onCaptureStarted
        );
        connect(
            m_screenCaptureService.get(),
            &infrastructure::adb::ScreenCaptureService::captureStopped,
            this,
            &MultiDeviceMirrorViewModel::onCaptureStopped
        );
        connect(
            m_screenCaptureService.get(),
            &infrastructure::adb::ScreenCaptureService::captureError,
            this,
            &MultiDeviceMirrorViewModel::onCaptureError
        );
        connect(
            m_screenCaptureService.get(),
            &infrastructure::adb::ScreenCaptureService::multiDeviceCaptureStarted,
            this, &MultiDeviceMirrorViewModel::onMultiDeviceCaptureStarted
        );
        connect(
            m_screenCaptureService.get(),
            &infrastructure::adb::ScreenCaptureService::multiDeviceCaptureStopped,
            this,
            &MultiDeviceMirrorViewModel::onMultiDeviceCaptureStopped
        );

        updateDevices();
    }

    void MultiDeviceMirrorViewModel::startMirroring()
    {
        if (m_isMirroring)
        {
            qWarning() << "Mirroring is already active";
            return;
        }

        startScreenCaptureForConnectedDevices();
    }

    void MultiDeviceMirrorViewModel::stopMirroring()
    {
        if (!m_isMirroring)
        {
            qWarning() << "Mirroring is not active";
            return;
        }

        stopScreenCaptureForAllDevices();
    }

    void MultiDeviceMirrorViewModel::toggleMirroring()
    {
        if (m_isMirroring)
        {
            stopMirroring();
        }
        else
        {
            startMirroring();
        }
    }

    void MultiDeviceMirrorViewModel::setGridLayout(const int columns, const int rows)
    {
        setGridColumns(columns);
        setGridRows(rows);
    }

    void MultiDeviceMirrorViewModel::setCaptureInterval(const int intervalMs)
    {
        if (m_captureInterval != intervalMs && intervalMs > 0)
        {
            m_captureInterval = intervalMs;

            for (const auto capturingDevices = m_screenCaptureService->getCapturingDevices(); const QString& deviceId :
                 capturingDevices)
            {
                m_screenCaptureService->setCaptureInterval(deviceId, intervalMs);
            }

            emit captureIntervalChanged();
        }
    }

    void MultiDeviceMirrorViewModel::setCaptureQuality(const int quality)
    {
        if (m_captureQuality != quality && quality >= 1 && quality <= 100)
        {
            m_captureQuality = quality;

            for (const auto capturingDevices = m_screenCaptureService->getCapturingDevices(); const QString& deviceId :
                 capturingDevices)
            {
                m_screenCaptureService->setCaptureQuality(deviceId, quality);
            }

            emit captureQualityChanged();
        }
    }

    QList<QObject*> MultiDeviceMirrorViewModel::devices() const
    {
        return m_devices;
    }

    bool MultiDeviceMirrorViewModel::isMirroring() const
    {
        return m_isMirroring;
    }

    int MultiDeviceMirrorViewModel::gridColumns() const
    {
        return m_gridColumns;
    }

    int MultiDeviceMirrorViewModel::gridRows() const
    {
        return m_gridRows;
    }

    int MultiDeviceMirrorViewModel::captureInterval() const
    {
        return m_captureInterval;
    }

    int MultiDeviceMirrorViewModel::captureQuality() const
    {
        return m_captureQuality;
    }

    QString MultiDeviceMirrorViewModel::errorMessage() const
    {
        return m_errorMessage;
    }

    void MultiDeviceMirrorViewModel::onDevicesUpdated()
    {
        updateDevices();
    }

    void MultiDeviceMirrorViewModel::onScreenCaptured(const QString& deviceId, const QImage& image)
    {
        if (m_imageProvider)
        {
            m_imageProvider->updateImage(deviceId, image);
        }
        else
        {
            qWarning() << "Image provider is null!";
        }
        emit screenCaptured(deviceId);
    }

    void MultiDeviceMirrorViewModel::onCaptureStarted(const QString& deviceId)
    {
    }

    void MultiDeviceMirrorViewModel::onCaptureStopped(const QString& deviceId)
    {
    }

    void MultiDeviceMirrorViewModel::onCaptureError(const QString& deviceId, const QString& error)
    {
        updateErrorMessage(QString("Error capturing device %1: %2").arg(deviceId, error));
    }

    void MultiDeviceMirrorViewModel::onMultiDeviceCaptureStarted(const QList<QString>& deviceIds)
    {
        m_isMirroring = true;
        emit isMirroringChanged();
        emit mirroringStarted();
    }

    void MultiDeviceMirrorViewModel::onMultiDeviceCaptureStopped()
    {
        m_isMirroring = false;
        emit isMirroringChanged();
        emit mirroringStopped();
    }

    void MultiDeviceMirrorViewModel::updateDevices()
    {
        if (!m_deviceUseCase)
        {
            return;
        }

        const auto deviceList = m_deviceUseCase->getConnectedDevices();
        m_devices.clear();

        for (const auto& device : deviceList)
        {
            if (device)
            {
                m_devices.append(device.get());
            }
        }

        emit devicesChanged();
    }

    void MultiDeviceMirrorViewModel::startScreenCaptureForConnectedDevices()
    {
        if (!m_deviceUseCase)
        {
            updateErrorMessage("Device management service not available");
            return;
        }

        const auto connectedDevices = m_deviceUseCase->getConnectedDevices();
        QList<QString> deviceIds;

        for (const auto& device : connectedDevices)
        {
            if (device && device->connected())
            {
                deviceIds.append(device->id());
            }
        }

        if (deviceIds.isEmpty())
        {
            updateErrorMessage("No connected devices are available for mirroring");
            return;
        }

        if (m_screenCaptureService->startMultiDeviceCapture(deviceIds, m_captureInterval))
        {
            updateErrorMessage("");
        }
        else
        {
            updateErrorMessage("Didn't start screen capture for devices");
        }
    }

    void MultiDeviceMirrorViewModel::stopScreenCaptureForAllDevices()
    {
        m_screenCaptureService->stopAllCaptures();
    }

    void MultiDeviceMirrorViewModel::updateErrorMessage(const QString& error)
    {
        if (m_errorMessage != error)
        {
            m_errorMessage = error;
            emit errorMessageChanged();
        }
    }

    void MultiDeviceMirrorViewModel::setGridColumns(int columns)
    {
        if (m_gridColumns != columns && columns > 0 && columns <= 4)
        {
            m_gridColumns = columns;
            emit gridColumnsChanged();
        }
    }

    void MultiDeviceMirrorViewModel::setGridRows(int rows)
    {
        if (m_gridRows != rows && rows > 0 && rows <= 4)
        {
            m_gridRows = rows;
            emit gridRowsChanged();
        }
    }
}
