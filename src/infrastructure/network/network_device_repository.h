#ifndef NETWORK_DEVICE_REPOSITORY_H
#define NETWORK_DEVICE_REPOSITORY_H

#include "../../core/interfaces/device_repository.h"
#include "device_discovery_service.h"
#include <QObject>
#include <QTimer>
#include <memory>

namespace infrastructure::network
{
    class NetworkDeviceRepository : public QObject, public core::interfaces::DeviceRepository
    {
        Q_OBJECT

    public:
        explicit NetworkDeviceRepository(
            std::shared_ptr<DeviceDiscoveryService> discoveryService,
            QObject* parent = nullptr
        );
        ~NetworkDeviceRepository() override;

        QList<std::shared_ptr<core::entities::Device>> discoverDevices() override;
        QList<std::shared_ptr<core::entities::Device>> getAllDevices() override;
        std::shared_ptr<core::entities::Device> getDeviceById(const QString& deviceId) override;
        bool connectToDevice(const QString& deviceId) override;
        bool disconnectFromDevice(const QString& deviceId) override;
        bool refreshDeviceStatus(const QString& deviceId) override;
        bool updateDevice(std::shared_ptr<core::entities::Device> device) override;
        bool removeDevice(const QString& deviceId) override;

        bool addDeviceManually(const QHostAddress& address, quint16 port = 8080);
        bool startAutoDiscovery();
        bool stopAutoDiscovery();
        bool isAutoDiscoveryEnabled() const;
        
        void setDiscoveryInterval(int intervalMs);
        void setConnectionTimeout(int timeoutMs);
        int getDiscoveryInterval() const;
        int getConnectionTimeout() const;

        int getOnlineDeviceCount() const;
        int getTotalDeviceCount() const;
        QStringList getDeviceAddresses() const;

    signals:
        void deviceDiscovered(std::shared_ptr<core::entities::Device> device);
        void deviceStatusChanged(const QString& deviceId, bool connected);
        void discoveryError(const QString& error);

    private slots:
        void onDeviceDiscovered(const NetworkDevice& networkDevice);
        void onDeviceUpdated(const NetworkDevice& networkDevice);
        void onDeviceLost(const QString& deviceId);
        void onDiscoveryError(const QString& error);
        void onStatusCheckTimer();

    private:
        std::shared_ptr<DeviceDiscoveryService> m_discoveryService;
        std::unique_ptr<QTimer> m_statusCheckTimer;
        
        QList<std::shared_ptr<core::entities::Device>> m_devices;
        QMap<QString, NetworkDevice> m_networkDevices;
        
        bool m_autoDiscoveryEnabled;
        int m_discoveryInterval;
        int m_connectionTimeout;
        int m_statusCheckInterval;

        std::shared_ptr<core::entities::Device> findDevice(const QString& deviceId);
        void updateDeviceStatus(const QString& deviceId, bool connected);
        void syncDeviceWithNetworkDevice(std::shared_ptr<core::entities::Device> device, const NetworkDevice& networkDevice);
        
        bool testDeviceConnection(const QString& deviceId);
        void performStatusCheck();
        
        std::shared_ptr<core::entities::Device> createDeviceFromNetworkDevice(const NetworkDevice& networkDevice) const;
        void removeDeviceInternal(const QString& deviceId);

        static constexpr int DEFAULT_DISCOVERY_INTERVAL = 5000;
        static constexpr int DEFAULT_CONNECTION_TIMEOUT = 10000;
        static constexpr int DEFAULT_STATUS_CHECK_INTERVAL = 15000;
    };
}

#endif 