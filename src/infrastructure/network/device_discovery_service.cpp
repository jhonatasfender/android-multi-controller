#include "device_discovery_service.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QTcpSocket>
#include <QDebug>
#include <QDateTime>
#include <QtEndian>

namespace infrastructure::network
{
    DeviceDiscoveryService::DeviceDiscoveryService(QObject* parent)
        : QObject(parent)
        , m_discoverySocket(std::make_unique<QUdpSocket>(this))
        , m_discoveryTimer(std::make_unique<QTimer>(this))
        , m_timeoutTimer(std::make_unique<QTimer>(this))
        , m_discoveryPort(DEFAULT_DISCOVERY_PORT)
        , m_discoveryInterval(DEFAULT_DISCOVERY_INTERVAL)
        , m_discoveryTimeout(DEFAULT_DISCOVERY_TIMEOUT)
        , m_isDiscovering(false)
        , m_discoverySequence(0)
    {
        m_discoveryTimer->setSingleShot(false);
        connect(m_discoveryTimer.get(), &QTimer::timeout, this, &DeviceDiscoveryService::onDiscoveryTimer);

        m_timeoutTimer->setSingleShot(false);
        connect(m_timeoutTimer.get(), &QTimer::timeout, this, &DeviceDiscoveryService::onDiscoveryTimeout);

        connect(m_discoverySocket.get(), &QUdpSocket::readyRead, this, &DeviceDiscoveryService::onSocketReadyRead);
        connect(m_discoverySocket.get(), QOverload<QAbstractSocket::SocketError>::of(&QUdpSocket::errorOccurred),
                this, &DeviceDiscoveryService::onSocketError);

        updateBroadcastAddresses();

        qDebug() << "DeviceDiscoveryService: Initialized";
    }

    DeviceDiscoveryService::~DeviceDiscoveryService()
    {
        stopDiscovery();
        qDebug() << "DeviceDiscoveryService: Destroyed";
    }

    bool DeviceDiscoveryService::startDiscovery()
    {
        if (m_isDiscovering)
        {
            return true;
        }

        if (!m_discoverySocket->bind(QHostAddress::Any, m_discoveryPort))
        {
            emit discoveryError(QString("Failed to bind to discovery port %1: %2")
                              .arg(m_discoveryPort)
                              .arg(m_discoverySocket->errorString()));
            return false;
        }

        updateBroadcastAddresses();
        
        m_isDiscovering = true;
        m_discoveryTimer->start(m_discoveryInterval);
        m_timeoutTimer->start(1000);

        sendDiscoveryBroadcast();

        qDebug() << "DeviceDiscoveryService: Discovery started on port" << m_discoveryPort;
        return true;
    }

    bool DeviceDiscoveryService::stopDiscovery()
    {
        if (!m_isDiscovering)
        {
            return true;
        }

        m_isDiscovering = false;
        m_discoveryTimer->stop();
        m_timeoutTimer->stop();
        m_discoverySocket->close();

        qDebug() << "DeviceDiscoveryService: Discovery stopped";
        return true;
    }

    bool DeviceDiscoveryService::isDiscovering() const
    {
        return m_isDiscovering;
    }

    bool DeviceDiscoveryService::addDevice(const QHostAddress& address, quint16 port)
    {
        if (findDeviceByAddress(address, port))
        {
            return true;
        }

        if (!testConnection(address, port))
        {
            emit discoveryError(QString("Cannot connect to device at %1:%2").arg(address.toString()).arg(port));
            return false;
        }

        QString deviceId = QString("%1:%2").arg(address.toString()).arg(port);
        
        NetworkDevice device(deviceId, address, port);
        device.name = QString("Manual Device (%1)").arg(address.toString());
        device.online = true;
        device.lastSeen = QDateTime::currentMSecsSinceEpoch();

        m_devices.append(device);
        emit deviceDiscovered(device);

        qDebug() << "DeviceDiscoveryService: Manually added device" << deviceId;
        return true;
    }

    bool DeviceDiscoveryService::removeDevice(const QString& deviceId)
    {
        for (int i = 0; i < m_devices.size(); ++i)
        {
            if (m_devices[i].deviceId == deviceId)
            {
                m_devices.removeAt(i);
                emit deviceLost(deviceId);
                qDebug() << "DeviceDiscoveryService: Removed device" << deviceId;
                return true;
            }
        }
        return false;
    }

    QList<NetworkDevice> DeviceDiscoveryService::getDiscoveredDevices() const
    {
        return m_devices;
    }

    QList<std::shared_ptr<core::entities::Device>> DeviceDiscoveryService::getDevices() const
    {
        QList<std::shared_ptr<core::entities::Device>> devices;
        
        for (const auto& networkDevice : m_devices)
        {
            if (networkDevice.online)
            {
                devices.append(createDeviceEntity(networkDevice));
            }
        }
        
        return devices;
    }

    NetworkDevice* DeviceDiscoveryService::findDevice(const QString& deviceId)
    {
        for (auto& device : m_devices)
        {
            if (device.deviceId == deviceId)
            {
                return &device;
            }
        }
        return nullptr;
    }

    NetworkDevice* DeviceDiscoveryService::findDeviceByAddress(const QHostAddress& address, quint16 port)
    {
        for (auto& device : m_devices)
        {
            if (device.address == address && device.port == port)
            {
                return &device;
            }
        }
        return nullptr;
    }

    void DeviceDiscoveryService::setDiscoveryInterval(int intervalMs)
    {
        m_discoveryInterval = intervalMs;
        if (m_isDiscovering)
        {
            m_discoveryTimer->setInterval(intervalMs);
        }
    }

    void DeviceDiscoveryService::setDiscoveryTimeout(int timeoutMs)
    {
        m_discoveryTimeout = timeoutMs;
    }

    void DeviceDiscoveryService::setDiscoveryPort(quint16 port)
    {
        if (m_isDiscovering)
        {
            qWarning() << "Cannot change discovery port while discovery is running";
            return;
        }
        m_discoveryPort = port;
    }

    void DeviceDiscoveryService::setBroadcastAddresses(const QList<QHostAddress>& addresses)
    {
        m_broadcastAddresses = addresses;
    }

    bool DeviceDiscoveryService::testConnection(const QHostAddress& address, quint16 port)
    {
        QTcpSocket socket;
        socket.connectToHost(address, port);
        
        if (!socket.waitForConnected(3000))
        {
            return false;
        }

        socket.disconnectFromHost();
        return true;
    }

    bool DeviceDiscoveryService::pingDevice(const QString& deviceId)
    {
        NetworkDevice* device = findDevice(deviceId);
        if (!device)
        {
            return false;
        }

        return testConnection(device->address, device->port);
    }

    void DeviceDiscoveryService::onDiscoveryTimer()
    {
        sendDiscoveryBroadcast();
    }

    void DeviceDiscoveryService::onDiscoveryTimeout()
    {
        cleanupOfflineDevices();
    }

    void DeviceDiscoveryService::onSocketReadyRead()
    {
        while (m_discoverySocket->hasPendingDatagrams())
        {
            QByteArray data;
            QHostAddress sender;
            quint16 senderPort;

            data.resize(m_discoverySocket->pendingDatagramSize());
            m_discoverySocket->readDatagram(data.data(), data.size(), &sender, &senderPort);

            processDiscoveryResponse(data, sender, senderPort);
        }
    }

    void DeviceDiscoveryService::onSocketError(QAbstractSocket::SocketError error)
    {
        emit discoveryError(QString("Socket error: %1").arg(m_discoverySocket->errorString()));
    }

    void DeviceDiscoveryService::sendDiscoveryBroadcast()
    {
        if (!m_isDiscovering)
        {
            return;
        }

        QByteArray request = createDiscoveryRequest();

        for (const QHostAddress& address : m_broadcastAddresses)
        {
            qint64 sent = m_discoverySocket->writeDatagram(request, address, DEFAULT_SERVER_PORT);
            if (sent != request.size())
            {
                qWarning() << "Failed to send discovery broadcast to" << address.toString();
            }
        }

        m_discoverySequence++;
    }

    void DeviceDiscoveryService::processDiscoveryResponse(const QByteArray& data, const QHostAddress& sender, quint16 port)
    {
        NetworkDevice device;
        if (!parseDiscoveryResponse(data, device))
        {
            return;
        }

        device.address = sender;
        if (device.port == 0)
        {
            device.port = DEFAULT_SERVER_PORT;
        }

        if (device.deviceId.isEmpty())
        {
            device.deviceId = QString("%1:%2").arg(sender.toString()).arg(device.port);
        }

        device.online = true;
        device.lastSeen = QDateTime::currentMSecsSinceEpoch();

        NetworkDevice* existingDevice = findDevice(device.deviceId);
        if (existingDevice)
        {
            existingDevice->name = device.name;
            existingDevice->model = device.model;
            existingDevice->manufacturer = device.manufacturer;
            existingDevice->androidVersion = device.androidVersion;
            existingDevice->apiLevel = device.apiLevel;
            existingDevice->screenWidth = device.screenWidth;
            existingDevice->screenHeight = device.screenHeight;
            existingDevice->online = true;
            existingDevice->lastSeen = device.lastSeen;

            emit deviceUpdated(*existingDevice);
        }
        else
        {
            m_devices.append(device);
            emit deviceDiscovered(device);
        }

        qDebug() << "DeviceDiscoveryService: Discovered device" << device.deviceId 
                << "at" << sender.toString() << ":" << device.port;
    }

    QByteArray DeviceDiscoveryService::createDiscoveryRequest()
    {
        QJsonObject request;
        request["type"] = "discovery_request";
        request["magic"] = static_cast<qint64>(DISCOVERY_MAGIC);
        request["sequence"] = static_cast<qint64>(m_discoverySequence);
        request["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        request["message"] = DISCOVERY_MESSAGE;

        QJsonDocument doc(request);
        return doc.toJson(QJsonDocument::Compact);
    }

    bool DeviceDiscoveryService::parseDiscoveryResponse(const QByteArray& data, NetworkDevice& device)
    {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);

        if (error.error != QJsonParseError::NoError)
        {
            return false;
        }

        QJsonObject response = doc.object();
        
        if (response["type"].toString() != "discovery_response")
        {
            return false;
        }

        quint32 magic = static_cast<quint32>(response["magic"].toInteger());
        if (magic != DISCOVERY_MAGIC)
        {
            return false;
        }

        device.deviceId = response["device_id"].toString();
        device.name = response["device_name"].toString();
        device.model = response["device_model"].toString();
        device.manufacturer = response["device_manufacturer"].toString();
        device.androidVersion = response["android_version"].toString();
        device.apiLevel = static_cast<quint32>(response["api_level"].toInteger());
        device.screenWidth = static_cast<quint32>(response["screen_width"].toInteger());
        device.screenHeight = static_cast<quint32>(response["screen_height"].toInteger());
        device.port = static_cast<quint16>(response["server_port"].toInteger());

        return true;
    }

    void DeviceDiscoveryService::updateBroadcastAddresses()
    {
        m_broadcastAddresses = getNetworkBroadcastAddresses();
        
        m_broadcastAddresses.append(QHostAddress::Broadcast);
        m_broadcastAddresses.append(QHostAddress("255.255.255.255"));

        qDebug() << "DeviceDiscoveryService: Updated broadcast addresses:" << m_broadcastAddresses.size();
    }

    QList<QHostAddress> DeviceDiscoveryService::getNetworkBroadcastAddresses()
    {
        QList<QHostAddress> addresses;

        const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
        for (const QNetworkInterface& interface : interfaces)
        {
            if (!(interface.flags() & QNetworkInterface::IsRunning) ||
                !(interface.flags() & QNetworkInterface::CanBroadcast) ||
                (interface.flags() & QNetworkInterface::IsLoopBack))
            {
                continue;
            }

            const QList<QNetworkAddressEntry> entries = interface.addressEntries();
            for (const QNetworkAddressEntry& entry : entries)
            {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    addresses.append(entry.broadcast());
                }
            }
        }

        return addresses;
    }

    void DeviceDiscoveryService::cleanupOfflineDevices()
    {
        const quint64 now = QDateTime::currentMSecsSinceEpoch();
        
        for (int i = m_devices.size() - 1; i >= 0; --i)
        {
            NetworkDevice& device = m_devices[i];
            
            if (device.online && (now - device.lastSeen) > m_discoveryTimeout)
            {
                device.online = false;
                emit deviceLost(device.deviceId);
                qDebug() << "DeviceDiscoveryService: Device" << device.deviceId << "went offline";
            }
        }
    }

    void DeviceDiscoveryService::updateDeviceLastSeen(const QString& deviceId)
    {
        NetworkDevice* device = findDevice(deviceId);
        if (device)
        {
            device->lastSeen = QDateTime::currentMSecsSinceEpoch();
            device->online = true;
        }
    }

    std::shared_ptr<core::entities::Device> DeviceDiscoveryService::createDeviceEntity(const NetworkDevice& networkDevice) const
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

    void DeviceDiscoveryService::updateDeviceFromResponse(NetworkDevice& device, const QByteArray& response)
    {
        Q_UNUSED(device)
        Q_UNUSED(response)
    }
} 