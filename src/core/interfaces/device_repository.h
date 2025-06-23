#ifndef DEVICE_REPOSITORY_H
#define DEVICE_REPOSITORY_H

#include <memory>
#include "../entities/device.h"

namespace core::interfaces
{
    class DeviceRepository
    {
    public:
        virtual ~DeviceRepository() = default;

        virtual QList<std::shared_ptr<entities::Device>> getAllDevices() = 0;
        virtual std::shared_ptr<entities::Device> getDeviceById(const QString& id) = 0;
        virtual bool addDevice(std::shared_ptr<entities::Device> device) = 0;
        virtual bool removeDevice(const QString& id) = 0;
        virtual bool updateDevice(std::shared_ptr<entities::Device> device) = 0;

        virtual QList<std::shared_ptr<entities::Device>> discoverDevices() = 0;
        virtual bool refreshDeviceStatus(const QString& id) = 0;

        virtual bool connectToDevice(const QString& id) = 0;
        virtual bool disconnectFromDevice(const QString& id) = 0;
        virtual bool isDeviceConnected(const QString& id) = 0;
    };
}

#endif
