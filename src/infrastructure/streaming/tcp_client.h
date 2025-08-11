#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QByteArray>
#include <memory>
#include <map>
#include "../../core/entities/device.h"
#include "../adb/adb_service.h"

namespace infrastructure::streaming
{
    class TcpClient final : public QObject
    {
        Q_OBJECT

    public:
        explicit TcpClient(QObject* parent = nullptr);
        ~TcpClient() override;

        enum class ConnectionState
        {
            Disconnected,
            Connecting,
            Connected,
            Error
        };

        Q_ENUM(ConnectionState)

        bool connectToDevice(const QString& deviceId, const QString& deviceIp, quint16 port = 27183);
        bool disconnectFromDevice(const QString& deviceId);
        bool isConnected(const QString& deviceId) const;
        ConnectionState getConnectionState(const QString& deviceId) const;
        
        bool sendCommand(const QString& deviceId, const QByteArray& command);
        bool requestVideoStream(const QString& deviceId);
        bool stopVideoStream(const QString& deviceId);
        
        QList<QString> getConnectedDevices() const;
        QString getDeviceIp(const QString& deviceId) const;
        quint16 getDevicePort(const QString& deviceId) const;

    signals:
        void connected(const QString& deviceId);
        void disconnected(const QString& deviceId);
        void connectionError(const QString& deviceId, const QString& error);
        void videoDataReceived(const QString& deviceId, const QByteArray& h264Data);
        void stateChanged(const QString& deviceId, ConnectionState state);

    private slots:
        void onSocketConnected();
        void onSocketDisconnected();
        void onSocketError(QAbstractSocket::SocketError error);
        void onSocketReadyRead();
        void onReconnectTimer();

    private:
        struct DeviceConnection
        {
            QString deviceId;
            QString deviceIp;
            quint16 port;
            std::unique_ptr<QTcpSocket> socket;
            ConnectionState state;
            QTimer* reconnectTimer;
            QByteArray buffer;
            bool streamingActive;
        };

        std::map<QString, std::unique_ptr<DeviceConnection>> m_connections;
        std::shared_ptr<adb::AdbService> m_adbService;
        static constexpr int RECONNECT_INTERVAL = 5000;
        static constexpr int CONNECTION_TIMEOUT = 10000;

        void setState(const QString& deviceId, ConnectionState state);
        void cleanupConnection(const QString& deviceId);
        void setupSocket(DeviceConnection* connection);
        void processReceivedData(const QString& deviceId, const QByteArray& data);
        void sendVideoStreamRequest(const QString& deviceId);
        QByteArray createStreamRequest();
        QByteArray createStopStreamRequest();
        
        static constexpr quint32 MAGIC_BYTES = 0x12345678;
        static constexpr quint8 PACKET_TYPE_VIDEO_STREAM_REQUEST = 0x01;
        static constexpr quint8 PACKET_TYPE_VIDEO_STREAM_STOP = 0x02;
        static constexpr quint8 PACKET_TYPE_VIDEO_DATA = 0x03;
    };
}

#endif 