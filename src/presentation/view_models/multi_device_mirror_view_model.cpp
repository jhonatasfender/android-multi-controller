#include "multi_device_mirror_view_model.h"
#include "../../core/entities/device.h"
#include <QTimer>
#include <QNetworkInterface>
#include <QDebug>
#include <QMap>
#include <QSet>
#include <QMutex>
#include <QMutexLocker>

namespace presentation::view_models
{
    MultiDeviceMirrorViewModel::MultiDeviceMirrorViewModel(
        const std::shared_ptr<use_case::DeviceManagementUseCase>& deviceUseCase,
        DeviceImageProvider* imageProvider,
        QObject* parent
    ) : QObject(parent)
        , m_deviceUseCase(deviceUseCase)
        , m_isMirroring(false)
        , m_gridColumns(2)
        , m_gridRows(2)
        , m_imageProvider(imageProvider)
    {
        initializePortPool();
        updateDevices();
    }
    
    void MultiDeviceMirrorViewModel::ensureStreamingService()
    {
        if (m_streamingService)
        {
            return;
        }

        qDebug() << "MultiDeviceMirrorViewModel: Creating streaming service";
        m_streamingService = std::make_unique<infrastructure::streaming::NativeStreamingService>(this);

        connect(
            m_streamingService.get(),
            &infrastructure::streaming::NativeStreamingService::streamingStarted,
            this,
            &MultiDeviceMirrorViewModel::onStreamingStarted
        );
        connect(
            m_streamingService.get(),
            &infrastructure::streaming::NativeStreamingService::streamingStopped,
            this,
            &MultiDeviceMirrorViewModel::onStreamingStopped
        );
        connect(
            m_streamingService.get(),
            &infrastructure::streaming::NativeStreamingService::streamingError,
            this,
            &MultiDeviceMirrorViewModel::onStreamingError
        );
        connect(
            m_streamingService.get(),
            &infrastructure::streaming::NativeStreamingService::frameReceived,
            this,
            &MultiDeviceMirrorViewModel::onFrameReceived
        );
        connect(
            m_streamingService.get(),
            &infrastructure::streaming::NativeStreamingService::statisticsUpdated,
            this,
            &MultiDeviceMirrorViewModel::onStatisticsUpdated
        );

        m_streamingService->setStreamingMode(infrastructure::streaming::NativeStreamingService::StreamingMode::MultiDevice);
        m_streamingService->setAutoReconnect(true);
    }

    void MultiDeviceMirrorViewModel::startMirroring()
    {
        if (m_isMirroring)
        {
            qWarning() << "Mirroring is already active";
            return;
        }

        ensureStreamingService();
        startStreamingForConnectedDevices();
    }

    void MultiDeviceMirrorViewModel::stopMirroring()
    {
        if (!m_isMirroring)
        {
            qWarning() << "Mirroring is not active";
            return;
        }

        if (m_streamingService)
        {
            stopStreamingForAllDevices();
        }
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

    void MultiDeviceMirrorViewModel::setStreamingQuality(const int bitrate)
    {
        qDebug() << "Setting streaming bitrate to:" << bitrate;
    }
    
    QObject* MultiDeviceMirrorViewModel::getDeviceById(const QString& deviceId)
    {
        if (!m_deviceUseCase) {
            return nullptr;
        }
        
        const auto connectedDevices = m_deviceUseCase->getConnectedDevices();
        for (const auto& device : connectedDevices) {
            if (device && device->id() == deviceId) {
                return device.get();
            }
        }
        
        return nullptr;
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

    QString MultiDeviceMirrorViewModel::errorMessage() const
    {
        return m_errorMessage;
    }

    double MultiDeviceMirrorViewModel::averageFPS() const
    {
        if (!m_streamingService)
        {
            return 0.0;
        }

        const auto fpsMap = m_streamingService->getAllDeviceFPS();
        if (fpsMap.isEmpty())
        {
            return 0.0;
        }

        double totalFPS = 0.0;
        for (const auto& fps : fpsMap)
        {
            totalFPS += fps;
        }

        return totalFPS / fpsMap.size();
    }

    int MultiDeviceMirrorViewModel::totalFrameCount() const
    {
        if (!m_streamingService)
        {
            return 0;
        }

        return m_streamingService->getTotalFrameCount();
    }

    void MultiDeviceMirrorViewModel::onDevicesUpdated()
    {
        updateDevices();
    }

    void MultiDeviceMirrorViewModel::onStreamingStarted(const QString& deviceId)
    {
        qDebug() << "MultiDeviceMirrorViewModel: Streaming started for device:" << deviceId;
        
        if (!m_isMirroring)
        {
            m_isMirroring = true;
            emit isMirroringChanged();
            emit mirroringStarted();
        }
    }

    void MultiDeviceMirrorViewModel::onStreamingStopped(const QString& deviceId)
    {
        qDebug() << "MultiDeviceMirrorViewModel: Streaming stopped for device:" << deviceId;
        
        if (m_streamingService)
        {
            const auto streamingDevices = m_streamingService->getStreamingDevices();
            if (streamingDevices.isEmpty())
            {
                m_isMirroring = false;
                emit isMirroringChanged();
                emit mirroringStopped();
            }
        }
    }

    void MultiDeviceMirrorViewModel::onStreamingError(const QString& deviceId, const QString& error)
    {
        qWarning() << "MultiDeviceMirrorViewModel: Streaming error for device" << deviceId << ":" << error;
        updateErrorMessage(QString("Streaming error for device %1: %2").arg(deviceId, error));
    }

    void MultiDeviceMirrorViewModel::onFrameReceived(const QString& deviceId, const QImage& frame)
    {
        if (m_imageProvider)
        {
            m_imageProvider->updateImage(deviceId, frame);
        }
        else
        {
            qWarning() << "Image provider is null!";
        }
        emit screenCaptured(deviceId);
    }

    void MultiDeviceMirrorViewModel::onStatisticsUpdated(const QString& deviceId, double fps, int frameCount, int errorCount)
    {
        Q_UNUSED(deviceId)
        Q_UNUSED(fps)
        Q_UNUSED(frameCount)
        Q_UNUSED(errorCount)
        
        emit statisticsChanged();
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

    void MultiDeviceMirrorViewModel::startStreamingForConnectedDevices()
    {
        if (!m_deviceUseCase)
        {
            updateErrorMessage("Device management service not available");
            return;
        }

        ensureStreamingService();
        if (!m_streamingService)
        {
            updateErrorMessage("Failed to initialize streaming service");
            return;
        }

        const auto connectedDevices = getConnectedDeviceIds();
        if (connectedDevices.isEmpty())
        {
            updateErrorMessage("No connected devices are available for mirroring");
            return;
        }

        bool allSuccess = true;
        for (const QString& deviceId : connectedDevices)
        {
            const QString deviceIp = getDeviceIpAddress(deviceId);
            const quint16 devicePort = getDevicePort(deviceId);
            
            if (deviceIp.isEmpty())
            {
                qWarning() << "Could not determine IP address for device:" << deviceId;
                allSuccess = false;
                continue;
            }

            qDebug() << "Starting streaming for device:" << deviceId << "IP:" << deviceIp << "Port:" << devicePort;
            
            if (!m_streamingService->startStreaming(deviceId, deviceIp, devicePort))
            {
                allSuccess = false;
                qWarning() << "Failed to start streaming for device:" << deviceId;
            }
        }

        if (!allSuccess)
        {
            updateErrorMessage("Failed to start streaming for some devices");
        }
        else
        {
            updateErrorMessage("");
        }
    }

    void MultiDeviceMirrorViewModel::stopStreamingForAllDevices()
    {
        if (m_streamingService)
        {
            m_streamingService->stopAllStreaming();
        }
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

    QList<QString> MultiDeviceMirrorViewModel::getConnectedDeviceIds() const
    {
        QList<QString> deviceIds;
        
        if (!m_deviceUseCase)
        {
            return deviceIds;
        }

        const auto connectedDevices = m_deviceUseCase->getConnectedDevices();
        for (const auto& device : connectedDevices)
        {
            if (device && device->connected())
            {
                deviceIds.append(device->id());
            }
        }

        return deviceIds;
    }

    QString MultiDeviceMirrorViewModel::getDeviceIpAddress(const QString& deviceId)
    {
        if (deviceId.contains(":"))
        {
            const QStringList parts = deviceId.split(":");
            if (parts.size() >= 2)
            {
                return parts[0];
            }
        }

        return "127.0.0.1";
    }

    quint16 MultiDeviceMirrorViewModel::getDevicePort(const QString& deviceId)
    {
        QMutexLocker locker(&m_portMutex);
        
        if (m_devicePortMap.contains(deviceId)) {
            return m_devicePortMap[deviceId];
        }
        
        locker.unlock();
        quint16 allocatedPort = allocatePortForDevice(deviceId);
        
        if (allocatedPort == 0) {
            qWarning() << "Failed to allocate port for device" << deviceId << "- using fallback port 8080";
            return 8080;
        }
        
        qDebug() << "Dynamically assigned port" << allocatedPort << "to device" << deviceId;
        return allocatedPort;
    }

    bool MultiDeviceMirrorViewModel::startStreamingForDevice(const QString& deviceId)
    {
        qDebug() << "Starting streaming for device:" << deviceId;
        
        ensureStreamingService();
        if (!m_streamingService) {
            qWarning() << "Failed to initialize streaming service";
            return false;
        }
        
        const QString deviceIp = getDeviceIpAddress(deviceId);
        const quint16 devicePort = getDevicePort(deviceId);
        
        if (deviceIp.isEmpty()) {
            qWarning() << "Could not determine IP address for device:" << deviceId;
            return false;
        }
        
        return m_streamingService->startStreaming(deviceId, deviceIp, devicePort);
    }
    
    bool MultiDeviceMirrorViewModel::stopStreamingForDevice(const QString& deviceId)
    {
        qDebug() << "Stopping streaming for device:" << deviceId;
        
        if (!m_streamingService) {
            qWarning() << "Streaming service not available";
            return false;
        }
        
        bool result = m_streamingService->stopStreaming(deviceId);
        
        // Release the port when streaming is stopped
        if (result) {
            releasePortForDevice(deviceId);
        }
        
        return result;
    }
    
    bool MultiDeviceMirrorViewModel::toggleStreamingForDevice(const QString& deviceId)
    {
        qDebug() << "Toggling streaming for device:" << deviceId;
        
        if (!m_streamingService) {
            qWarning() << "Streaming service not available";
            return false;
        }
        
        if (m_streamingService->isStreaming(deviceId)) {
            return stopStreamingForDevice(deviceId);
        } else {
            return startStreamingForDevice(deviceId);
        }
    }

    // Dynamic port management methods
    void MultiDeviceMirrorViewModel::initializePortPool()
    {
        QMutexLocker locker(&m_portMutex);
        
        // Initialize the pool with available ports
        m_availablePorts.clear();
        m_devicePortMap.clear();
        
        for (quint16 port = PORT_RANGE_START; port <= PORT_RANGE_END; ++port) {
            m_availablePorts.insert(port);
        }
        
        qDebug() << "Initialized port pool with" << m_availablePorts.size() << "ports";
        qDebug() << "Port range:" << PORT_RANGE_START << "to" << PORT_RANGE_END;
    }

    quint16 MultiDeviceMirrorViewModel::allocatePortForDevice(const QString& deviceId)
    {
        QMutexLocker locker(&m_portMutex);
        
        // Check if device already has a port assigned
        if (m_devicePortMap.contains(deviceId)) {
            qDebug() << "Device" << deviceId << "already has port" << m_devicePortMap[deviceId];
            return m_devicePortMap[deviceId];
        }
        
        // Find the next available port
        if (m_availablePorts.isEmpty()) {
            qWarning() << "No available ports for device" << deviceId;
            return 0; // No port available
        }
        
        // Get the first available port
        quint16 allocatedPort = *m_availablePorts.begin();
        m_availablePorts.remove(allocatedPort);
        m_devicePortMap[deviceId] = allocatedPort;
        
        qDebug() << "Allocated port" << allocatedPort << "for device" << deviceId;
        qDebug() << "Remaining available ports:" << m_availablePorts.size();
        
        return allocatedPort;
    }

    void MultiDeviceMirrorViewModel::releasePortForDevice(const QString& deviceId)
    {
        QMutexLocker locker(&m_portMutex);
        
        if (!m_devicePortMap.contains(deviceId)) {
            qDebug() << "Device" << deviceId << "does not have an allocated port";
            return;
        }
        
        quint16 releasedPort = m_devicePortMap[deviceId];
        m_devicePortMap.remove(deviceId);
        m_availablePorts.insert(releasedPort);
        
        qDebug() << "Released port" << releasedPort << "from device" << deviceId;
        qDebug() << "Available ports:" << m_availablePorts.size();
    }

    bool MultiDeviceMirrorViewModel::isPortAvailable(quint16 port) const
    {
        QMutexLocker locker(const_cast<QMutex*>(&m_portMutex));
        return m_availablePorts.contains(port);
    }
}
