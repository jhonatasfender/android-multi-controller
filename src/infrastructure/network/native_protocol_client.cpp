#include "native_protocol_client.h"
#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QtEndian>

namespace infrastructure::network
{
    NativeProtocolClient::NativeProtocolClient(QObject* parent)
        : QObject(parent)
        , m_connectionTimeout(DEFAULT_CONNECTION_TIMEOUT)
        , m_heartbeatInterval(DEFAULT_HEARTBEAT_INTERVAL)
        , m_autoReconnect(false)
    {
        qDebug() << "NativeProtocolClient: Initialized";
    }

    NativeProtocolClient::~NativeProtocolClient()
    {
        stopAllProcesses();
        qDebug() << "NativeProtocolClient: Destroyed";
    }

    bool NativeProtocolClient::connectToDevice(const QString& deviceId, const QHostAddress& address, quint16 port)
    {
        if (isConnected(deviceId))
        {
            return true;
        }

        auto connection = std::make_unique<DeviceConnection>();
        connection->deviceId = deviceId;
        connection->address = address;
        connection->port = port;
        connection->socket = std::make_unique<QTcpSocket>(this);
        connection->connected = false;
        connection->videoStreamActive = false;
        connection->lastHeartbeat = 0;
        connection->sequenceNumber = 0;
        connection->heartbeatTimer = new QTimer(this);
        connection->connectionTimer = new QTimer(this);

        setupConnection(connection.get());

        connection->socket->connectToHost(address, port);
        connection->connectionTimer->start(m_connectionTimeout);

        m_connections[deviceId] = std::move(connection);

        qDebug() << "NativeProtocolClient: Connecting to device" << deviceId << "at" << address.toString() << ":" << port;
        return true;
    }

    bool NativeProtocolClient::disconnectFromDevice(const QString& deviceId)
    {
        auto connection = getConnection(deviceId);
        if (!connection)
        {
            return false;
        }

        connection->socket->disconnectFromHost();
        cleanupConnection(deviceId);

        qDebug() << "NativeProtocolClient: Disconnected from device" << deviceId;
        return true;
    }

    bool NativeProtocolClient::isConnected(const QString& deviceId) const
    {
        auto it = m_connections.find(deviceId);
        return it != m_connections.end() && it->second->connected;
    }

    bool NativeProtocolClient::executeCommand(const QString& deviceId, const QString& command)
    {
        auto connection = getConnection(deviceId);
        if (!connection || !connection->connected)
        {
            return false;
        }

        QByteArray packet = createCommandPacket(command);
        qint64 written = connection->socket->write(packet);
        
        return written == packet.size();
    }

    bool NativeProtocolClient::executeCommandAsync(
        const QString& deviceId,
        const QString& command,
        std::function<void(bool, const QString&)> callback)
    {
        auto connection = getConnection(deviceId);
        if (!connection || !connection->connected)
        {
            if (callback)
            {
                callback(false, "Device not connected");
            }
            return false;
        }

        connection->commandCallback = callback;
        return executeCommand(deviceId, command);
    }

    bool NativeProtocolClient::sendTouchEvent(const QString& deviceId, int x, int y, int action)
    {
        QJsonObject eventData;
        eventData["x"] = x;
        eventData["y"] = y;
        eventData["action"] = action;

        ControlEventType eventType = (action == 0) ? ControlEventType::TouchDown :
                                   (action == 1) ? ControlEventType::TouchUp :
                                   ControlEventType::TouchMove;

        return sendControlEvent(deviceId, eventType, eventData);
    }

    bool NativeProtocolClient::sendKeyEvent(const QString& deviceId, int keyCode, int action)
    {
        QJsonObject eventData;
        eventData["keyCode"] = keyCode;
        eventData["action"] = action;

        ControlEventType eventType = (action == 0) ? ControlEventType::KeyDown : ControlEventType::KeyUp;
        return sendControlEvent(deviceId, eventType, eventData);
    }

    bool NativeProtocolClient::pushFile(const QString& deviceId, const QString& localPath, const QString& remotePath)
    {
        Q_UNUSED(deviceId)
        Q_UNUSED(localPath)
        Q_UNUSED(remotePath)
        return false;
    }

    bool NativeProtocolClient::pullFile(const QString& deviceId, const QString& remotePath, const QString& localPath)
    {
        Q_UNUSED(deviceId)
        Q_UNUSED(remotePath)
        Q_UNUSED(localPath)
        return false;
    }

    bool NativeProtocolClient::installApp(const QString& deviceId, const QString& apkPath)
    {
        QJsonObject eventData;
        eventData["apkPath"] = apkPath;
        return sendControlEvent(deviceId, ControlEventType::SystemCommand, eventData);
    }

    bool NativeProtocolClient::uninstallApp(const QString& deviceId, const QString& packageName)
    {
        QJsonObject eventData;
        eventData["packageName"] = packageName;
        eventData["action"] = "uninstall";
        return sendControlEvent(deviceId, ControlEventType::SystemCommand, eventData);
    }

    bool NativeProtocolClient::launchApp(const QString& deviceId, const QString& packageName)
    {
        QJsonObject eventData;
        eventData["packageName"] = packageName;
        return sendControlEvent(deviceId, ControlEventType::AppLaunch, eventData);
    }

    void NativeProtocolClient::stopAllProcesses()
    {
        std::vector<QString> deviceIds;
        for (const auto& pair : m_connections)
        {
            deviceIds.push_back(pair.first);
        }
        for (const QString& deviceId : deviceIds)
        {
            disconnectFromDevice(deviceId);
        }
        m_connections.clear();
    }

    bool NativeProtocolClient::sendControlEvent(const QString& deviceId, ControlEventType eventType, const QJsonObject& data)
    {
        auto connection = getConnection(deviceId);
        if (!connection || !connection->connected)
        {
            return false;
        }

        QByteArray packet = createControlEventPacket(eventType, data);
        qint64 written = connection->socket->write(packet);
        
        return written == packet.size();
    }

    bool NativeProtocolClient::requestMetadata(const QString& deviceId)
    {
        auto connection = getConnection(deviceId);
        if (!connection || !connection->connected)
        {
            return false;
        }

        QByteArray packet = createPacket(PacketType::Metadata, QByteArray());
        qint64 written = connection->socket->write(packet);
        
        return written == packet.size();
    }

    bool NativeProtocolClient::requestVideoStream(const QString& deviceId)
    {
        auto connection = getConnection(deviceId);
        if (!connection || !connection->connected)
        {
            return false;
        }

        QJsonObject requestData;
        requestData["action"] = "start_video_stream";
        
        QJsonDocument doc(requestData);
        QByteArray packet = createPacket(PacketType::VideoConfig, doc.toJson(QJsonDocument::Compact));
        qint64 written = connection->socket->write(packet);
        
        return written == packet.size();
    }

    bool NativeProtocolClient::stopVideoStream(const QString& deviceId)
    {
        auto connection = getConnection(deviceId);
        if (!connection || !connection->connected)
        {
            return false;
        }

        QJsonObject requestData;
        requestData["action"] = "stop_video_stream";
        
        QJsonDocument doc(requestData);
        QByteArray packet = createPacket(PacketType::VideoConfig, doc.toJson(QJsonDocument::Compact));
        qint64 written = connection->socket->write(packet);
        
        return written == packet.size();
    }

    bool NativeProtocolClient::sendHeartbeat(const QString& deviceId)
    {
        auto connection = getConnection(deviceId);
        if (!connection || !connection->connected)
        {
            return false;
        }

        QByteArray packet = createHeartbeatPacket();
        qint64 written = connection->socket->write(packet);
        
        return written == packet.size();
    }

    QJsonObject NativeProtocolClient::getDeviceMetadata(const QString& deviceId) const
    {
        auto it = m_connections.find(deviceId);
        return it != m_connections.end() ? it->second->metadata : QJsonObject();
    }

    bool NativeProtocolClient::isVideoStreamActive(const QString& deviceId) const
    {
        auto it = m_connections.find(deviceId);
        return it != m_connections.end() && it->second->videoStreamActive;
    }

    quint64 NativeProtocolClient::getLastHeartbeat(const QString& deviceId) const
    {
        auto it = m_connections.find(deviceId);
        return it != m_connections.end() ? it->second->lastHeartbeat : 0;
    }

    void NativeProtocolClient::setConnectionTimeout(int timeoutMs)
    {
        m_connectionTimeout = timeoutMs;
    }

    void NativeProtocolClient::setHeartbeatInterval(int intervalMs)
    {
        m_heartbeatInterval = intervalMs;
        
        for (const auto& pair : m_connections)
        {
            if (pair.second->heartbeatTimer->isActive())
            {
                pair.second->heartbeatTimer->setInterval(intervalMs);
            }
        }
    }

    void NativeProtocolClient::setAutoReconnect(bool enabled)
    {
        m_autoReconnect = enabled;
    }

    int NativeProtocolClient::getConnectionTimeout() const
    {
        return m_connectionTimeout;
    }

    int NativeProtocolClient::getHeartbeatInterval() const
    {
        return m_heartbeatInterval;
    }

    bool NativeProtocolClient::isAutoReconnectEnabled() const
    {
        return m_autoReconnect;
    }

    void NativeProtocolClient::onSocketConnected()
    {
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
        if (!socket)
        {
            return;
        }

        QString deviceId = getDeviceIdFromSocket(socket);
        auto connection = getConnection(deviceId);
        if (!connection)
        {
            return;
        }

        connection->connected = true;
        connection->connectionTimer->stop();
        connection->heartbeatTimer->start(m_heartbeatInterval);

        emit deviceConnected(deviceId);
        
        requestMetadata(deviceId);

        qDebug() << "NativeProtocolClient: Connected to device" << deviceId;
    }

    void NativeProtocolClient::onSocketDisconnected()
    {
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
        if (!socket)
        {
            return;
        }

        QString deviceId = getDeviceIdFromSocket(socket);
        auto connection = getConnection(deviceId);
        if (connection)
        {
            connection->connected = false;
            connection->videoStreamActive = false;
            connection->heartbeatTimer->stop();
        }

        emit deviceDisconnected(deviceId);
        
        if (!m_autoReconnect)
        {
            cleanupConnection(deviceId);
        }

        qDebug() << "NativeProtocolClient: Disconnected from device" << deviceId;
    }

    void NativeProtocolClient::onSocketError(QAbstractSocket::SocketError error)
    {
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
        if (!socket)
        {
            return;
        }

        QString deviceId = getDeviceIdFromSocket(socket);
        QString errorString = socket->errorString();

        emit errorOccurred(deviceId, errorString);
        
        qWarning() << "NativeProtocolClient: Socket error for device" << deviceId << ":" << errorString;
    }

    void NativeProtocolClient::onSocketReadyRead()
    {
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
        if (!socket)
        {
            return;
        }

        QString deviceId = getDeviceIdFromSocket(socket);
        auto connection = getConnection(deviceId);
        if (!connection)
        {
            return;
        }

        connection->receiveBuffer.append(socket->readAll());
        processReceivedData(connection);
    }

    void NativeProtocolClient::onHeartbeatTimer()
    {
        QTimer* timer = qobject_cast<QTimer*>(sender());
        if (!timer)
        {
            return;
        }

        for (const auto& pair : m_connections)
        {
            if (pair.second->heartbeatTimer == timer)
            {
                sendHeartbeat(pair.second->deviceId);
                break;
            }
        }
    }

    void NativeProtocolClient::onConnectionTimeout()
    {
        QTimer* timer = qobject_cast<QTimer*>(sender());
        if (!timer)
        {
            return;
        }

        for (const auto& pair : m_connections)
        {
            if (pair.second->connectionTimer == timer)
            {
                emit errorOccurred(pair.second->deviceId, "Connection timeout");
                cleanupConnection(pair.second->deviceId);
                break;
            }
        }
    }

    NativeProtocolClient::DeviceConnection* NativeProtocolClient::getConnection(const QString& deviceId)
    {
        auto it = m_connections.find(deviceId);
        return it != m_connections.end() ? it->second.get() : nullptr;
    }

    void NativeProtocolClient::setupConnection(DeviceConnection* connection)
    {
        if (!connection || !connection->socket)
        {
            return;
        }

        connect(connection->socket.get(), &QTcpSocket::connected, this, &NativeProtocolClient::onSocketConnected);
        connect(connection->socket.get(), &QTcpSocket::disconnected, this, &NativeProtocolClient::onSocketDisconnected);
        connect(connection->socket.get(), QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
                this, &NativeProtocolClient::onSocketError);
        connect(connection->socket.get(), &QTcpSocket::readyRead, this, &NativeProtocolClient::onSocketReadyRead);

        connect(connection->heartbeatTimer, &QTimer::timeout, this, &NativeProtocolClient::onHeartbeatTimer);
        connect(connection->connectionTimer, &QTimer::timeout, this, &NativeProtocolClient::onConnectionTimeout);

        connection->heartbeatTimer->setSingleShot(false);
        connection->connectionTimer->setSingleShot(true);
    }

    void NativeProtocolClient::cleanupConnection(const QString& deviceId)
    {
        auto it = m_connections.find(deviceId);
        if (it != m_connections.end())
        {
            auto& connection = it->second;
            
            if (connection->heartbeatTimer)
            {
                connection->heartbeatTimer->stop();
                connection->heartbeatTimer->deleteLater();
            }
            
            if (connection->connectionTimer)
            {
                connection->connectionTimer->stop();
                connection->connectionTimer->deleteLater();
            }

            m_connections.erase(it);
        }
    }

    void NativeProtocolClient::processReceivedData(DeviceConnection* connection)
    {
        if (!connection)
        {
            return;
        }

        while (connection->receiveBuffer.size() >= PACKET_HEADER_SIZE)
        {
            AndroidServerPacket packet;
            if (!parsePacket(connection->receiveBuffer, packet))
            {
                connection->receiveBuffer.clear();
                break;
            }

            if (connection->receiveBuffer.size() < packet.length)
            {
                break;
            }

            packet.payload = connection->receiveBuffer.mid(PACKET_HEADER_SIZE, packet.length - PACKET_HEADER_SIZE);
            
            connection->receiveBuffer.remove(0, packet.length);

            handlePacket(connection, packet);
        }
    }

    bool NativeProtocolClient::parsePacket(const QByteArray& data, AndroidServerPacket& packet)
    {
        if (data.size() < PACKET_HEADER_SIZE)
        {
            return false;
        }

        QDataStream stream(data);
        stream.setByteOrder(QDataStream::BigEndian);

        stream >> packet.magic;
        stream >> packet.version;
        stream >> packet.type;
        stream >> packet.flags;
        stream >> packet.length;
        stream >> packet.timestamp;
        stream >> packet.sequence;
        stream >> packet.crc32;

        return packet.magic == PACKET_MAGIC && packet.version == PROTOCOL_VERSION;
    }

    void NativeProtocolClient::handlePacket(DeviceConnection* connection, const AndroidServerPacket& packet)
    {
        PacketType type = static_cast<PacketType>(packet.type);

        switch (type)
        {
        case PacketType::Metadata:
            handleMetadataPacket(connection, packet);
            break;
        case PacketType::VideoConfig:
            handleVideoConfigPacket(connection, packet);
            break;
        case PacketType::CommandResponse:
            handleCommandResponsePacket(connection, packet);
            break;
        case PacketType::ErrorMessage:
            handleErrorPacket(connection, packet);
            break;
        case PacketType::Heartbeat:
            handleHeartbeatPacket(connection, packet);
            break;
        default:
            qDebug() << "NativeProtocolClient: Unknown packet type:" << packet.type;
            break;
        }
    }

    void NativeProtocolClient::handleMetadataPacket(DeviceConnection* connection, const AndroidServerPacket& packet)
    {
        if (!connection)
        {
            return;
        }

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(packet.payload, &error);
        
        if (error.error == QJsonParseError::NoError)
        {
            connection->metadata = doc.object();
            emit metadataReceived(connection->deviceId, connection->metadata);
        }
    }

    void NativeProtocolClient::handleVideoConfigPacket(DeviceConnection* connection, const AndroidServerPacket& packet)
    {
        Q_UNUSED(packet)
        
        if (!connection)
        {
            return;
        }

        connection->videoStreamActive = true;
        emit videoStreamStarted(connection->deviceId);
    }

    void NativeProtocolClient::handleCommandResponsePacket(DeviceConnection* connection, const AndroidServerPacket& packet)
    {
        if (!connection)
        {
            return;
        }

        QString response = QString::fromUtf8(packet.payload);
        bool success = !response.contains("error", Qt::CaseInsensitive);

        if (connection->commandCallback)
        {
            connection->commandCallback(success, response);
            connection->commandCallback = nullptr;
        }

        emit commandExecuted(connection->deviceId, "", success, response);
    }

    void NativeProtocolClient::handleErrorPacket(DeviceConnection* connection, const AndroidServerPacket& packet)
    {
        if (!connection)
        {
            return;
        }

        QString error = QString::fromUtf8(packet.payload);
        emit errorOccurred(connection->deviceId, error);
    }

    void NativeProtocolClient::handleHeartbeatPacket(DeviceConnection* connection, const AndroidServerPacket& packet)
    {
        Q_UNUSED(packet)
        
        if (!connection)
        {
            return;
        }

        connection->lastHeartbeat = QDateTime::currentMSecsSinceEpoch();
        emit heartbeatReceived(connection->deviceId);
    }

    QByteArray NativeProtocolClient::createPacket(PacketType type, const QByteArray& payload)
    {
        AndroidServerPacket header = createPacketHeader(type, payload.size());
        
        QByteArray packet;
        QDataStream stream(&packet, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);

        stream << header.magic;
        stream << header.version;
        stream << header.type;
        stream << header.flags;
        stream << header.length;
        stream << header.timestamp;
        stream << header.sequence;
        stream << header.crc32;

        packet.append(payload);
        return packet;
    }

    QByteArray NativeProtocolClient::createControlEventPacket(ControlEventType eventType, const QJsonObject& data)
    {
        QJsonObject eventObject;
        eventObject["eventType"] = static_cast<int>(eventType);
        eventObject["data"] = data;

        QJsonDocument doc(eventObject);
        return createPacket(PacketType::ControlEvent, doc.toJson(QJsonDocument::Compact));
    }

    QByteArray NativeProtocolClient::createCommandPacket(const QString& command)
    {
        QJsonObject commandObject;
        commandObject["command"] = command;

        QJsonDocument doc(commandObject);
        return createPacket(PacketType::CommandRequest, doc.toJson(QJsonDocument::Compact));
    }

    QByteArray NativeProtocolClient::createHeartbeatPacket()
    {
        QJsonObject heartbeatObject;
        heartbeatObject["timestamp"] = QDateTime::currentMSecsSinceEpoch();

        QJsonDocument doc(heartbeatObject);
        return createPacket(PacketType::Heartbeat, doc.toJson(QJsonDocument::Compact));
    }

    quint32 NativeProtocolClient::calculateCRC32(const QByteArray& data)
    {
        quint32 crc = 0xFFFFFFFF;
        for (char byte : data)
        {
            crc ^= static_cast<quint8>(byte);
            for (int i = 0; i < 8; i++)
            {
                if (crc & 1)
                {
                    crc = (crc >> 1) ^ 0xEDB88320;
                }
                else
                {
                    crc >>= 1;
                }
            }
        }
        return ~crc;
    }

    AndroidServerPacket NativeProtocolClient::createPacketHeader(PacketType type, quint32 payloadLength)
    {
        AndroidServerPacket header;
        header.magic = PACKET_MAGIC;
        header.version = PROTOCOL_VERSION;
        header.type = static_cast<quint8>(type);
        header.flags = 0;
        header.length = PACKET_HEADER_SIZE + payloadLength;
        header.timestamp = QDateTime::currentMSecsSinceEpoch();
        header.sequence = 0;
        header.crc32 = 0;

        return header;
    }

    QString NativeProtocolClient::getDeviceIdFromSocket(QTcpSocket* socket)
    {
        for (const auto& pair : m_connections)
        {
            if (pair.second->socket.get() == socket)
            {
                return pair.second->deviceId;
            }
        }
        return QString();
    }
} 