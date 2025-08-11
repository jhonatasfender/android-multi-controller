#ifndef NATIVE_PROTOCOL_CLIENT_H
#define NATIVE_PROTOCOL_CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostAddress>
#include <unordered_map>
#include "../../core/interfaces/command_executor.h"

namespace infrastructure::network
{
    struct AndroidServerPacket
    {
        quint32 magic;
        quint16 version;
        quint8 type;
        quint8 flags;
        quint32 length;
        quint64 timestamp;
        quint32 sequence;
        quint32 crc32;
        QByteArray payload;

        AndroidServerPacket() 
            : magic(0x53435250), version(1), type(0), flags(0), length(0), timestamp(0), sequence(0), crc32(0) {}
    };

    enum class PacketType : quint8
    {
        Metadata = 0x01,
        VideoConfig = 0x02,
        VideoData = 0x03,
        AudioConfig = 0x04,
        AudioData = 0x05,
        ControlEvent = 0x06,
        Heartbeat = 0x07,
        ErrorMessage = 0x08,
        ConnectionAck = 0x09,
        CommandRequest = 0x10,
        CommandResponse = 0x11
    };

    enum class ControlEventType : quint8
    {
        TouchDown = 0x01,
        TouchUp = 0x02,
        TouchMove = 0x03,
        KeyDown = 0x04,
        KeyUp = 0x05,
        AppLaunch = 0x06,
        AppClose = 0x07,
        SystemCommand = 0x08
    };

    class NativeProtocolClient : public QObject, public core::interfaces::CommandExecutor
    {
        Q_OBJECT

    public:
        explicit NativeProtocolClient(QObject* parent = nullptr);
        ~NativeProtocolClient() override;

        bool connectToDevice(const QString& deviceId, const QHostAddress& address, quint16 port = 8080);
        bool disconnectFromDevice(const QString& deviceId);
        bool isConnected(const QString& deviceId) const;

        bool executeCommand(const QString& deviceId, const QString& command) override;
        bool executeCommandAsync(
            const QString& deviceId,
            const QString& command,
            std::function<void(bool, const QString&)> callback
        ) override;
        bool sendTouchEvent(const QString& deviceId, int x, int y, int action) override;
        bool sendKeyEvent(const QString& deviceId, int keyCode, int action) override;
        bool pushFile(const QString& deviceId, const QString& localPath, const QString& remotePath) override;
        bool pullFile(const QString& deviceId, const QString& remotePath, const QString& localPath) override;
        bool installApp(const QString& deviceId, const QString& apkPath) override;
        bool uninstallApp(const QString& deviceId, const QString& packageName) override;
        bool launchApp(const QString& deviceId, const QString& packageName) override;
        void stopAllProcesses() override;

        bool sendControlEvent(const QString& deviceId, ControlEventType eventType, const QJsonObject& data);
        bool requestMetadata(const QString& deviceId);
        bool requestVideoStream(const QString& deviceId);
        bool stopVideoStream(const QString& deviceId);
        bool sendHeartbeat(const QString& deviceId);

        QJsonObject getDeviceMetadata(const QString& deviceId) const;
        bool isVideoStreamActive(const QString& deviceId) const;
        quint64 getLastHeartbeat(const QString& deviceId) const;

        void setConnectionTimeout(int timeoutMs);
        void setHeartbeatInterval(int intervalMs);
        void setAutoReconnect(bool enabled);
        int getConnectionTimeout() const;
        int getHeartbeatInterval() const;
        bool isAutoReconnectEnabled() const;

    signals:
        void deviceConnected(const QString& deviceId);
        void deviceDisconnected(const QString& deviceId);
        void metadataReceived(const QString& deviceId, const QJsonObject& metadata);
        void videoStreamStarted(const QString& deviceId);
        void videoStreamStopped(const QString& deviceId);
        void heartbeatReceived(const QString& deviceId);
        void commandExecuted(const QString& deviceId, const QString& command, bool success, const QString& output);
        void errorOccurred(const QString& deviceId, const QString& error);

    private slots:
        void onSocketConnected();
        void onSocketDisconnected();
        void onSocketError(QAbstractSocket::SocketError error);
        void onSocketReadyRead();
        void onHeartbeatTimer();
        void onConnectionTimeout();

    private:
        struct DeviceConnection
        {
            QString deviceId;
            QHostAddress address;
            quint16 port;
            std::unique_ptr<QTcpSocket> socket;
            QByteArray receiveBuffer;
            bool connected;
            bool videoStreamActive;
            quint64 lastHeartbeat;
            quint32 sequenceNumber;
            QJsonObject metadata;
            QTimer* heartbeatTimer;
            QTimer* connectionTimer;
            std::function<void(bool, const QString&)> commandCallback;
        };

        std::unordered_map<QString, std::unique_ptr<DeviceConnection>> m_connections;
        int m_connectionTimeout;
        int m_heartbeatInterval;
        bool m_autoReconnect;

        DeviceConnection* getConnection(const QString& deviceId);
        void setupConnection(DeviceConnection* connection);
        void cleanupConnection(const QString& deviceId);

        void processReceivedData(DeviceConnection* connection);
        bool parsePacket(const QByteArray& data, AndroidServerPacket& packet);
        void handlePacket(DeviceConnection* connection, const AndroidServerPacket& packet);
        void handleMetadataPacket(DeviceConnection* connection, const AndroidServerPacket& packet);
        void handleVideoConfigPacket(DeviceConnection* connection, const AndroidServerPacket& packet);
        void handleCommandResponsePacket(DeviceConnection* connection, const AndroidServerPacket& packet);
        void handleErrorPacket(DeviceConnection* connection, const AndroidServerPacket& packet);
        void handleHeartbeatPacket(DeviceConnection* connection, const AndroidServerPacket& packet);

        QByteArray createPacket(PacketType type, const QByteArray& payload);
        QByteArray createControlEventPacket(ControlEventType eventType, const QJsonObject& data);
        QByteArray createCommandPacket(const QString& command);
        QByteArray createHeartbeatPacket();
        
        quint32 calculateCRC32(const QByteArray& data);
        AndroidServerPacket createPacketHeader(PacketType type, quint32 payloadLength);

        QString getDeviceIdFromSocket(QTcpSocket* socket);

        static constexpr quint32 PACKET_MAGIC = 0x53435250;
        static constexpr quint16 PROTOCOL_VERSION = 1;
        static constexpr int DEFAULT_CONNECTION_TIMEOUT = 10000;
        static constexpr int DEFAULT_HEARTBEAT_INTERVAL = 5000;
        static constexpr int PACKET_HEADER_SIZE = 32;
    };
}

#endif 