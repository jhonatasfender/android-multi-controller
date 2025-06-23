#ifndef DEVICE_REPOSITORY_IMPL_H
#define DEVICE_REPOSITORY_IMPL_H

#include <memory>
#include "../../core/interfaces/device_repository.h"
#include "../../core/entities/device.h"

namespace infrastructure::persistence
{
    class DeviceRepositoryImpl final : public QObject, public core::interfaces::DeviceRepository
    {
        Q_OBJECT

    public:
        explicit DeviceRepositoryImpl(QObject* parent = nullptr);
        ~DeviceRepositoryImpl() override;

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

    private:
        QList<std::shared_ptr<core::entities::Device>> m_devices;
        void loadDevices();
        void saveDevices();
    };
}

#endif
