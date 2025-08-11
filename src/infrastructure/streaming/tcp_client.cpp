#include "tcp_client.h"
#include <QDataStream>
#include <QNetworkInterface>
#include <QDebug>

namespace infrastructure::streaming
{
    TcpClient::TcpClient(QObject* parent)
        : QObject(parent)
        , m_adbService(nullptr)
    {
    }

    TcpClient::~TcpClient()
    {
        for (auto& pair : m_connections)
        {
            cleanupConnection(pair.first);
        }
    }

    bool TcpClient::connectToDevice(const QString& deviceId, const QString& deviceIp, quint16 port)
    {
        if (m_connections.find(deviceId) != m_connections.end())
        {
            qDebug() << "Already connected to device:" << deviceId;
            return true;
        }

        auto connection = std::make_unique<DeviceConnection>();
        connection->deviceId = deviceId;
        connection->deviceIp = deviceIp;
        connection->port = port;
        connection->socket = std::make_unique<QTcpSocket>(this);
        connection->state = ConnectionState::Disconnected;
        connection->reconnectTimer = new QTimer(this);
        connection->streamingActive = false;

        setupSocket(connection.get());

        m_connections[deviceId] = std::move(connection);

        setState(deviceId, ConnectionState::Connecting);
        m_connections[deviceId]->socket->connectToHost(QHostAddress(deviceIp), port);

        return true;
    }

    bool TcpClient::disconnectFromDevice(const QString& deviceId)
    {
        auto it = m_connections.find(deviceId);
        if (it == m_connections.end())
        {
            return false;
        }

        auto& connection = it->second;
        
        if (connection->streamingActive)
        {
            stopVideoStream(deviceId);
        }

        setState(deviceId, ConnectionState::Disconnected);
        
        if (connection->socket->state() == QAbstractSocket::ConnectedState)
        {
            connection->socket->disconnectFromHost();
        }

        cleanupConnection(deviceId);
        return true;
    }

    bool TcpClient::isConnected(const QString& deviceId) const
    {
        auto it = m_connections.find(deviceId);
        if (it == m_connections.end())
        {
            return false;
        }

        return it->second->state == ConnectionState::Connected;
    }

    TcpClient::ConnectionState TcpClient::getConnectionState(const QString& deviceId) const
    {
        auto it = m_connections.find(deviceId);
        if (it == m_connections.end())
        {
            return ConnectionState::Disconnected;
        }

        return it->second->state;
    }

    bool TcpClient::sendCommand(const QString& deviceId, const QByteArray& command)
    {
        if (!isConnected(deviceId))
        {
            return false;
        }

        auto it = m_connections.find(deviceId);
        if (it == m_connections.end()) return false;
        const auto& connection = it->second;
        const qint64 bytesWritten = connection->socket->write(command);
        
        return bytesWritten > 0 && connection->socket->waitForBytesWritten(CONNECTION_TIMEOUT);
    }

    bool TcpClient::requestVideoStream(const QString& deviceId)
    {
        if (!isConnected(deviceId))
        {
            qWarning() << "Cannot request video stream: device not connected:" << deviceId;
            return false;
        }

        const QByteArray request = createStreamRequest();
        if (sendCommand(deviceId, request))
        {
            auto it = m_connections.find(deviceId);
            if (it != m_connections.end())
            {
                it->second->streamingActive = true;
            }
            return true;
        }

        return false;
    }

    bool TcpClient::stopVideoStream(const QString& deviceId)
    {
        if (!isConnected(deviceId))
        {
            return false;
        }

        const QByteArray request = createStopStreamRequest();
        if (sendCommand(deviceId, request))
        {
            auto it = m_connections.find(deviceId);
            if (it != m_connections.end())
            {
                it->second->streamingActive = false;
            }
            return true;
        }

        return false;
    }

    QList<QString> TcpClient::getConnectedDevices() const
    {
        QList<QString> connectedDevices;
        
        for (const auto& pair : m_connections)
        {
            if (pair.second->state == ConnectionState::Connected)
            {
                connectedDevices.append(pair.second->deviceId);
            }
        }

        return connectedDevices;
    }

    QString TcpClient::getDeviceIp(const QString& deviceId) const
    {
        auto it = m_connections.find(deviceId);
        if (it == m_connections.end())
        {
            return QString();
        }

        return it->second->deviceIp;
    }

    quint16 TcpClient::getDevicePort(const QString& deviceId) const
    {
        auto it = m_connections.find(deviceId);
        if (it == m_connections.end())
        {
            return 0;
        }

        return it->second->port;
    }

    void TcpClient::onSocketConnected()
    {
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
        if (!socket) return;

        QString deviceId;
        for (const auto& pair : m_connections)
        {
            if (pair.second->socket.get() == socket)
            {
                deviceId = pair.second->deviceId;
                break;
            }
        }

        if (deviceId.isEmpty()) return;

        setState(deviceId, ConnectionState::Connected);
        emit connected(deviceId);
        
        qDebug() << "Connected to device:" << deviceId;
    }

    void TcpClient::onSocketDisconnected()
    {
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
        if (!socket) return;

        QString deviceId;
        for (const auto& pair : m_connections)
        {
            if (pair.second->socket.get() == socket)
            {
                deviceId = pair.second->deviceId;
                break;
            }
        }

        if (deviceId.isEmpty()) return;

        setState(deviceId, ConnectionState::Disconnected);
        emit disconnected(deviceId);
        
        qDebug() << "Disconnected from device:" << deviceId;

        auto it = m_connections.find(deviceId);
        if (it != m_connections.end())
        {
            it->second->reconnectTimer->start(RECONNECT_INTERVAL);
        }
    }

    void TcpClient::onSocketError(QAbstractSocket::SocketError error)
    {
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
        if (!socket) return;

        QString deviceId;
        for (const auto& pair : m_connections)
        {
            if (pair.second->socket.get() == socket)
            {
                deviceId = pair.second->deviceId;
                break;
            }
        }

        if (deviceId.isEmpty()) return;

        setState(deviceId, ConnectionState::Error);
        const QString errorString = socket->errorString();
        emit connectionError(deviceId, errorString);
        
        qWarning() << "Socket error for device" << deviceId << ":" << errorString;

        auto it = m_connections.find(deviceId);
        if (it != m_connections.end())
        {
            it->second->reconnectTimer->start(RECONNECT_INTERVAL);
        }
    }

    void TcpClient::onSocketReadyRead()
    {
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
        if (!socket) return;

        QString deviceId;
        for (const auto& pair : m_connections)
        {
            if (pair.second->socket.get() == socket)
            {
                deviceId = pair.second->deviceId;
                break;
            }
        }

        if (deviceId.isEmpty()) return;

        const QByteArray data = socket->readAll();
        processReceivedData(deviceId, data);
    }

    void TcpClient::onReconnectTimer()
    {
        QTimer* timer = qobject_cast<QTimer*>(sender());
        if (!timer) return;

        QString deviceId;
        for (const auto& pair : m_connections)
        {
            if (pair.second->reconnectTimer == timer)
            {
                deviceId = pair.second->deviceId;
                break;
            }
        }

        if (deviceId.isEmpty()) return;

        timer->stop();

        auto it = m_connections.find(deviceId);
        if (it != m_connections.end())
        {
            const auto& connection = it->second;
            setState(deviceId, ConnectionState::Connecting);
            connection->socket->connectToHost(QHostAddress(connection->deviceIp), connection->port);
            
            qDebug() << "Attempting to reconnect to device:" << deviceId;
        }
    }

    void TcpClient::setState(const QString& deviceId, ConnectionState state)
    {
        auto it = m_connections.find(deviceId);
        if (it == m_connections.end()) return;

        it->second->state = state;
        emit stateChanged(deviceId, state);
    }

    void TcpClient::cleanupConnection(const QString& deviceId)
    {
        auto it = m_connections.find(deviceId);
        if (it == m_connections.end()) return;

        const auto& connection = it->second;
        
        if (connection->reconnectTimer)
        {
            connection->reconnectTimer->stop();
            connection->reconnectTimer->deleteLater();
        }

        if (connection->socket)
        {
            connection->socket->disconnectFromHost();
            connection->socket->deleteLater();
        }

        m_connections.erase(it);
    }

    void TcpClient::setupSocket(DeviceConnection* connection)
    {
        if (!connection || !connection->socket) return;

        QTcpSocket* socket = connection->socket.get();
        
        connect(socket, &QTcpSocket::connected, this, &TcpClient::onSocketConnected);
        connect(socket, &QTcpSocket::disconnected, this, &TcpClient::onSocketDisconnected);
        connect(socket, &QTcpSocket::errorOccurred, this, &TcpClient::onSocketError);
        connect(socket, &QTcpSocket::readyRead, this, &TcpClient::onSocketReadyRead);

        connection->reconnectTimer->setSingleShot(true);
        connect(connection->reconnectTimer, &QTimer::timeout, this, &TcpClient::onReconnectTimer);
    }

    void TcpClient::processReceivedData(const QString& deviceId, const QByteArray& data)
    {
        auto it = m_connections.find(deviceId);
        if (it == m_connections.end()) return;

        auto& connection = it->second;
        connection->buffer.append(data);

        while (connection->buffer.size() >= sizeof(quint32) + sizeof(quint8) + sizeof(quint32))
        {
            QDataStream stream(connection->buffer);
            stream.setByteOrder(QDataStream::BigEndian);

            quint32 magic;
            stream >> magic;
            
            if (magic != MAGIC_BYTES)
            {
                qWarning() << "Invalid magic bytes received from device:" << deviceId;
                connection->buffer.clear();
                return;
            }

            quint8 packetType;
            stream >> packetType;

            quint32 payloadSize;
            stream >> payloadSize;

            const quint32 packetSize = sizeof(quint32) + sizeof(quint8) + sizeof(quint32) + payloadSize;
            if (connection->buffer.size() < packetSize)
            {
                return;
            }

            QByteArray payload = connection->buffer.mid(sizeof(quint32) + sizeof(quint8) + sizeof(quint32), payloadSize);
            
            connection->buffer.remove(0, packetSize);

            if (packetType == PACKET_TYPE_VIDEO_DATA)
            {
                emit videoDataReceived(deviceId, payload);
            }
            else
            {
                qWarning() << "Unknown packet type received:" << packetType;
            }
        }
    }

    QByteArray TcpClient::createStreamRequest()
    {
        QByteArray packet;
        QDataStream stream(&packet, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);

        stream << MAGIC_BYTES;
        stream << PACKET_TYPE_VIDEO_STREAM_REQUEST;
        stream << static_cast<quint32>(0);

        return packet;
    }

    QByteArray TcpClient::createStopStreamRequest()
    {
        QByteArray packet;
        QDataStream stream(&packet, QIODevice::WriteOnly);
        stream.setByteOrder(QDataStream::BigEndian);

        stream << MAGIC_BYTES;
        stream << PACKET_TYPE_VIDEO_STREAM_STOP;
        stream << static_cast<quint32>(0);

        return packet;
    }
} 