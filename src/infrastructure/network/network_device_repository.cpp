#include "network_device_repository.h"
#include <QDebug>
#include <QDateTime>
#include <QTcpSocket>
#include <QHostAddress>

namespace infrastructure::network
{
    NetworkDeviceRepository::NetworkDeviceRepository(
        std::shared_ptr<DeviceDiscoveryService> discoveryService,
        QObject* parent
    )
        : QObject(parent)
        , m_discoveryService(discoveryService)
        , m_statusCheckTimer(std::make_unique<QTimer>(this))
        , m_autoDiscoveryEnabled(false)
        , m_discoveryInterval(DEFAULT_DISCOVERY_INTERVAL)
        , m_connectionTimeout(DEFAULT_CONNECTION_TIMEOUT)
        , m_statusCheckInterval(DEFAULT_STATUS_CHECK_INTERVAL)
    {
        connect(m_discoveryService.get(), &DeviceDiscoveryService::deviceDiscovered,
                this, &NetworkDeviceRepository::onDeviceDiscovered);
        connect(m_discoveryService.get(), &DeviceDiscoveryService::deviceUpdated,
                this, &NetworkDeviceRepository::onDeviceUpdated);
        connect(m_discoveryService.get(), &DeviceDiscoveryService::deviceLost,
                this, &NetworkDeviceRepository::onDeviceLost);
        connect(m_discoveryService.get(), &DeviceDiscoveryService::discoveryError,
                this, &NetworkDeviceRepository::onDiscoveryError);

        m_statusCheckTimer->setSingleShot(false);
        m_statusCheckTimer->setInterval(m_statusCheckInterval);
        connect(m_statusCheckTimer.get(), &QTimer::timeout, this, &NetworkDeviceRepository::onStatusCheckTimer);

        qDebug() << "NetworkDeviceRepository: Initialized";
    }

    NetworkDeviceRepository::~NetworkDeviceRepository()
    {
        stopAutoDiscovery();
        qDebug() << "NetworkDeviceRepository: Destroyed";
    }

    QList<std::shared_ptr<core::entities::Device>> NetworkDeviceRepository::discoverDevices()
    {
        if (!m_discoveryService->isDiscovering())
        {
            m_discoveryService->startDiscovery();
        }

        const auto networkDevices = m_discoveryService->getDiscoveredDevices();
        
        for (const auto& networkDevice : networkDevices)
        {
            m_networkDevices[networkDevice.deviceId] = networkDevice;
            
            auto device = findDevice(networkDevice.deviceId);
            if (!device)
            {
                device = createDeviceFromNetworkDevice(networkDevice);
                m_devices.append(device);
            }
            else
            {
                syncDeviceWithNetworkDevice(device, networkDevice);
            }
        }

        return m_devices;
    }

    QList<std::shared_ptr<core::entities::Device>> NetworkDeviceRepository::getAllDevices()
    {
        return m_devices;
    }

    std::shared_ptr<core::entities::Device> NetworkDeviceRepository::getDeviceById(const QString& deviceId)
    {
        return findDevice(deviceId);
    }

    bool NetworkDeviceRepository::connectToDevice(const QString& deviceId)
    {
        auto device = findDevice(deviceId);
        if (!device)
        {
            qWarning() << "NetworkDeviceRepository: Device not found:" << deviceId;
            return false;
        }

        if (device->connected())
        {
            return true;
        }

        if (!testDeviceConnection(deviceId))
        {
            qWarning() << "NetworkDeviceRepository: Failed to connect to device:" << deviceId;
            return false;
        }

        device->setConnected(true);
        device->setStatus(core::entities::DeviceStatus::Authorized);
        
        emit deviceStatusChanged(deviceId, true);
        
        qDebug() << "NetworkDeviceRepository: Connected to device:" << deviceId;
        return true;
    }

    bool NetworkDeviceRepository::disconnectFromDevice(const QString& deviceId)
    {
        auto device = findDevice(deviceId);
        if (!device)
        {
            return false;
        }

        device->setConnected(false);
        device->setStatus(core::entities::DeviceStatus::Offline);
        
        emit deviceStatusChanged(deviceId, false);
        
        qDebug() << "NetworkDeviceRepository: Disconnected from device:" << deviceId;
        return true;
    }

    bool NetworkDeviceRepository::refreshDeviceStatus(const QString& deviceId)
    {
        auto device = findDevice(deviceId);
        if (!device)
        {
            return false;
        }

        bool isConnected = testDeviceConnection(deviceId);
        bool statusChanged = device->connected() != isConnected;

        device->setConnected(isConnected);
        device->setStatus(isConnected ? 
                         core::entities::DeviceStatus::Authorized : 
                         core::entities::DeviceStatus::Offline);

        if (statusChanged)
        {
            emit deviceStatusChanged(deviceId, isConnected);
        }

        return true;
    }

    bool NetworkDeviceRepository::updateDevice(std::shared_ptr<core::entities::Device> device)
    {
        if (!device)
        {
            return false;
        }

        auto existingDevice = findDevice(device->id());
        if (!existingDevice)
        {
            m_devices.append(device);
            return true;
        }

        existingDevice->setName(device->name());
        existingDevice->setModel(device->model());
        existingDevice->setManufacturer(device->manufacturer());
        existingDevice->setConnected(device->connected());
        existingDevice->setStatus(device->status());

        return true;
    }

    bool NetworkDeviceRepository::removeDevice(const QString& deviceId)
    {
        removeDeviceInternal(deviceId);
        
        m_discoveryService->removeDevice(deviceId);
        
        return true;
    }

    bool NetworkDeviceRepository::addDeviceManually(const QHostAddress& address, quint16 port)
    {
        return m_discoveryService->addDevice(address, port);
    }

    bool NetworkDeviceRepository::startAutoDiscovery()
    {
        if (m_autoDiscoveryEnabled)
        {
            return true;
        }

        bool success = m_discoveryService->startDiscovery();
        if (success)
        {
            m_autoDiscoveryEnabled = true;
            m_statusCheckTimer->start();
            qDebug() << "NetworkDeviceRepository: Auto-discovery started";
        }

        return success;
    }

    bool NetworkDeviceRepository::stopAutoDiscovery()
    {
        if (!m_autoDiscoveryEnabled)
        {
            return true;
        }

        m_autoDiscoveryEnabled = false;
        m_statusCheckTimer->stop();
        m_discoveryService->stopDiscovery();
        
        qDebug() << "NetworkDeviceRepository: Auto-discovery stopped";
        return true;
    }

    bool NetworkDeviceRepository::isAutoDiscoveryEnabled() const
    {
        return m_autoDiscoveryEnabled;
    }

    void NetworkDeviceRepository::setDiscoveryInterval(int intervalMs)
    {
        m_discoveryInterval = intervalMs;
        m_discoveryService->setDiscoveryInterval(intervalMs);
    }

    void NetworkDeviceRepository::setConnectionTimeout(int timeoutMs)
    {
        m_connectionTimeout = timeoutMs;
        m_discoveryService->setDiscoveryTimeout(timeoutMs);
    }

    int NetworkDeviceRepository::getDiscoveryInterval() const
    {
        return m_discoveryInterval;
    }

    int NetworkDeviceRepository::getConnectionTimeout() const
    {
        return m_connectionTimeout;
    }

    int NetworkDeviceRepository::getOnlineDeviceCount() const
    {
        int count = 0;
        for (const auto& device : m_devices)
        {
            if (device->connected())
            {
                count++;
            }
        }
        return count;
    }

    int NetworkDeviceRepository::getTotalDeviceCount() const
    {
        return m_devices.size();
    }

    QStringList NetworkDeviceRepository::getDeviceAddresses() const
    {
        QStringList addresses;
        for (const auto& networkDevice : m_networkDevices)
        {
            addresses.append(QString("%1:%2").arg(networkDevice.address.toString()).arg(networkDevice.port));
        }
        return addresses;
    }

    void NetworkDeviceRepository::onDeviceDiscovered(const NetworkDevice& networkDevice)
    {
        m_networkDevices[networkDevice.deviceId] = networkDevice;
        
        auto device = findDevice(networkDevice.deviceId);
        if (!device)
        {
            device = createDeviceFromNetworkDevice(networkDevice);
            m_devices.append(device);
            emit deviceDiscovered(device);
        }
        else
        {
            syncDeviceWithNetworkDevice(device, networkDevice);
        }

        qDebug() << "NetworkDeviceRepository: Device discovered:" << networkDevice.deviceId;
    }

    void NetworkDeviceRepository::onDeviceUpdated(const NetworkDevice& networkDevice)
    {
        m_networkDevices[networkDevice.deviceId] = networkDevice;
        
        auto device = findDevice(networkDevice.deviceId);
        if (device)
        {
            syncDeviceWithNetworkDevice(device, networkDevice);
        }
    }

    void NetworkDeviceRepository::onDeviceLost(const QString& deviceId)
    {
        auto device = findDevice(deviceId);
        if (device)
        {
            device->setConnected(false);
            device->setStatus(core::entities::DeviceStatus::Offline);
            emit deviceStatusChanged(deviceId, false);
        }

        m_networkDevices.remove(deviceId);
        
        qDebug() << "NetworkDeviceRepository: Device lost:" << deviceId;
    }

    void NetworkDeviceRepository::onDiscoveryError(const QString& error)
    {
        emit discoveryError(error);
        qWarning() << "NetworkDeviceRepository: Discovery error:" << error;
    }

    void NetworkDeviceRepository::onStatusCheckTimer()
    {
        performStatusCheck();
    }

    std::shared_ptr<core::entities::Device> NetworkDeviceRepository::findDevice(const QString& deviceId)
    {
        for (const auto& device : m_devices)
        {
            if (device->id() == deviceId)
            {
                return device;
            }
        }
        return nullptr;
    }

    void NetworkDeviceRepository::updateDeviceStatus(const QString& deviceId, bool connected)
    {
        auto device = findDevice(deviceId);
        if (device)
        {
            bool statusChanged = device->connected() != connected;
            device->setConnected(connected);
            device->setStatus(connected ? 
                             core::entities::DeviceStatus::Authorized : 
                             core::entities::DeviceStatus::Offline);

            if (statusChanged)
            {
                emit deviceStatusChanged(deviceId, connected);
            }
        }
    }

    void NetworkDeviceRepository::syncDeviceWithNetworkDevice(
        std::shared_ptr<core::entities::Device> device, 
        const NetworkDevice& networkDevice)
    {
        if (!device)
        {
            return;
        }

        device->setName(networkDevice.name.isEmpty() ? networkDevice.deviceId : networkDevice.name);
        device->setModel(networkDevice.model);
        device->setManufacturer(networkDevice.manufacturer);
        device->setConnected(networkDevice.online);
        device->setStatus(networkDevice.online ? 
                         core::entities::DeviceStatus::Authorized : 
                         core::entities::DeviceStatus::Offline);
    }

    bool NetworkDeviceRepository::testDeviceConnection(const QString& deviceId)
    {
        if (!m_networkDevices.contains(deviceId))
        {
            return false;
        }

        const NetworkDevice& networkDevice = m_networkDevices[deviceId];
        return m_discoveryService->testConnection(networkDevice.address, networkDevice.port);
    }

    void NetworkDeviceRepository::performStatusCheck()
    {
        for (const auto& device : m_devices)
        {
            if (device->connected())
            {
                bool isConnected = testDeviceConnection(device->id());
                if (!isConnected)
                {
                    updateDeviceStatus(device->id(), false);
                }
            }
        }
    }

    std::shared_ptr<core::entities::Device> NetworkDeviceRepository::createDeviceFromNetworkDevice(const NetworkDevice& networkDevice) const
    {
        auto device = std::make_shared<core::entities::Device>(
            networkDevice.deviceId,
            networkDevice.name.isEmpty() ? networkDevice.deviceId : networkDevice.name
        );

        device->setModel(networkDevice.model);
        device->setManufacturer(networkDevice.manufacturer);
        device->setConnected(networkDevice.online);
        device->setStatus(networkDevice.online ? 
                         core::entities::DeviceStatus::Authorized : 
                         core::entities::DeviceStatus::Offline);

        return device;
    }

    void NetworkDeviceRepository::removeDeviceInternal(const QString& deviceId)
    {
        for (int i = 0; i < m_devices.size(); ++i)
        {
            if (m_devices[i]->id() == deviceId)
            {
                m_devices.removeAt(i);
                break;
            }
        }
        
        m_networkDevices.remove(deviceId);
    }
} 