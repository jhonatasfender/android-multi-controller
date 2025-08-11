#ifndef DEVICE_DISCOVERY_SERVICE_H
#define DEVICE_DISCOVERY_SERVICE_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QString>
#include <QList>
#include <QAbstractSocket>
#include <memory>
#include "../../core/entities/device.h"

namespace infrastructure::network
{
    struct NetworkDevice
    {
        QString deviceId;
        QString name;
        QString model;
        QString manufacturer;
        QHostAddress address;
        quint16 port;
        quint64 lastSeen;
        bool online;
        QString androidVersion;
        quint32 apiLevel;
        quint32 screenWidth;
        quint32 screenHeight;

        NetworkDevice() 
            : port(8080), lastSeen(0), online(false), apiLevel(0), screenWidth(0), screenHeight(0) {}
        
        NetworkDevice(const QString& id, const QHostAddress& addr, quint16 p)
            : deviceId(id), address(addr), port(p), lastSeen(0), online(false), apiLevel(0), screenWidth(0), screenHeight(0) {}
    };

    class DeviceDiscoveryService : public QObject
    {
        Q_OBJECT

    public:
        explicit DeviceDiscoveryService(QObject* parent = nullptr);
        ~DeviceDiscoveryService() override;

        bool startDiscovery();
        bool stopDiscovery();
        bool isDiscovering() const;

        bool addDevice(const QHostAddress& address, quint16 port = 8080);
        bool removeDevice(const QString& deviceId);

        QList<NetworkDevice> getDiscoveredDevices() const;
        QList<std::shared_ptr<core::entities::Device>> getDevices() const;
        NetworkDevice* findDevice(const QString& deviceId);
        NetworkDevice* findDeviceByAddress(const QHostAddress& address, quint16 port);

        void setDiscoveryInterval(int intervalMs);
        void setDiscoveryTimeout(int timeoutMs);
        void setDiscoveryPort(quint16 port);
        void setBroadcastAddresses(const QList<QHostAddress>& addresses);

        bool testConnection(const QHostAddress& address, quint16 port);
        bool pingDevice(const QString& deviceId);

    signals:
        void deviceDiscovered(const NetworkDevice& device);
        void deviceUpdated(const NetworkDevice& device);
        void deviceLost(const QString& deviceId);
        void discoveryError(const QString& error);

    private slots:
        void onDiscoveryTimer();
        void onDiscoveryTimeout();
        void onSocketReadyRead();
        void onSocketError(QAbstractSocket::SocketError error);

    private:
        std::unique_ptr<QUdpSocket> m_discoverySocket;
        std::unique_ptr<QTimer> m_discoveryTimer;
        std::unique_ptr<QTimer> m_timeoutTimer;

        QList<NetworkDevice> m_devices;
        QList<QHostAddress> m_broadcastAddresses;
        
        quint16 m_discoveryPort;
        int m_discoveryInterval;
        int m_discoveryTimeout;
        bool m_isDiscovering;
        quint32 m_discoverySequence;

        void sendDiscoveryBroadcast();
        void processDiscoveryResponse(const QByteArray& data, const QHostAddress& sender, quint16 port);
        QByteArray createDiscoveryRequest();
        bool parseDiscoveryResponse(const QByteArray& data, NetworkDevice& device);

        void updateBroadcastAddresses();
        QList<QHostAddress> getNetworkBroadcastAddresses();
        void cleanupOfflineDevices();
        void updateDeviceLastSeen(const QString& deviceId);

        std::shared_ptr<core::entities::Device> createDeviceEntity(const NetworkDevice& networkDevice) const;
        void updateDeviceFromResponse(NetworkDevice& device, const QByteArray& response);

        static constexpr quint16 DEFAULT_DISCOVERY_PORT = 8081;
        static constexpr quint16 DEFAULT_SERVER_PORT = 8080;
        static constexpr int DEFAULT_DISCOVERY_INTERVAL = 5000;
        static constexpr int DEFAULT_DISCOVERY_TIMEOUT = 30000;
        static constexpr quint32 DISCOVERY_MAGIC = 0x41445343;
        static constexpr const char* DISCOVERY_MESSAGE = "ANDROID_SERVER_DISCOVERY";
    };
}

#endif 