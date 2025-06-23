#ifndef ADB_DEVICE_REPOSITORY_H
#define ADB_DEVICE_REPOSITORY_H

#include <memory>
#include "../../core/interfaces/device_repository.h"
#include "../../core/entities/device.h"
#include "../adb/adb_service.h"

namespace infrastructure::persistence
{
    class AdbDeviceRepository final : public QObject, public core::interfaces::DeviceRepository
    {
        Q_OBJECT

    public:
        explicit AdbDeviceRepository(const std::shared_ptr<adb::AdbService>& adbService, QObject* parent = nullptr);
        ~AdbDeviceRepository() override;

        QList<std::shared_ptr<core::entities::Device>> getAllDevices() override;
        std::shared_ptr<core::entities::Device> getDeviceById(const QString& id) override;
        bool addDevice(std::shared_ptr<core::entities::Device> device) override;
        bool removeDevice(const QString& id) override;
        bool updateDevice(std::shared_ptr<core::entities::Device> device) override;
        QList<std::shared_ptr<core::entities::Device>> discoverDevices() override;
        bool refreshDeviceStatus(const QString& id) override;
        bool connectToDevice(const QString& id) override;
        bool disconnectFromDevice(const QString& id) override;
        bool isDeviceConnected(const QString& id) override;

    private slots:
        void onDevicesUpdated(const QList<std::shared_ptr<core::entities::Device>>& devices);
        void onDeviceConnected(const std::shared_ptr<core::entities::Device>& device);
        void onDeviceDisconnected(const QString& deviceId);
        static void onErrorOccurred(const QString& error);

    private:
        std::shared_ptr<adb::AdbService> m_adbService;
        QList<std::shared_ptr<core::entities::Device>> m_devices;
    };
}

#endif
