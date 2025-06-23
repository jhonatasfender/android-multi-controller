#include "device_list_view_model.h"
#include <QQmlListProperty>

namespace presentation::view_models
{
    DeviceListViewModel::DeviceListViewModel(
        const std::shared_ptr<use_case::DeviceManagementUseCase>& useCase,
        QObject* parent
    )
        : QObject(parent)
          , m_useCase(useCase)
          , m_isLoading(false)
          , m_connectedDeviceCount(0)
    {
        if (m_useCase)
        {
            connect(
                m_useCase.get(),
                &use_case::DeviceManagementUseCase::deviceDiscovered,
                this,
                &DeviceListViewModel::onDeviceDiscovered
            );
            connect(
                m_useCase.get(),
                &use_case::DeviceManagementUseCase::deviceConnected,
                this,
                &DeviceListViewModel::onDeviceConnected
            );
            connect(
                m_useCase.get(),
                &use_case::DeviceManagementUseCase::deviceDisconnected,
                this,
                &DeviceListViewModel::onDeviceDisconnected
            );
            connect(
                m_useCase.get(),
                &use_case::DeviceManagementUseCase::deviceStatusChanged,
                this,
                &DeviceListViewModel::onDeviceStatusChanged
            );
            connect(
                m_useCase.get(),
                &use_case::DeviceManagementUseCase::errorOccurred,
                this,
                &DeviceListViewModel::onErrorOccurred
            );
        }
    }

    QQmlListProperty<core::entities::Device> DeviceListViewModel::devices()
    {
        return QQmlListProperty(this, &m_devices);
    }

    void DeviceListViewModel::refreshDevices()
    {
        setLoading(true);

        if (m_useCase)
        {
            m_device_store = m_useCase->discoverAndConnectDevices();
            m_devices.clear();

            for (const auto& device : m_device_store)
            {
                m_devices.append(device.get());
            }

            updateConnectedDeviceCount();
            emit devicesChanged();
        }

        setLoading(false);
    }

    void DeviceListViewModel::connectToDevice(const QString& deviceId) const
    {
        if (m_useCase)
        {
            m_useCase->connectToDevice(deviceId);
        }
    }

    void DeviceListViewModel::disconnectFromDevice(const QString& deviceId) const
    {
        if (m_useCase)
        {
            m_useCase->disconnectFromDevice(deviceId);
        }
    }

    void DeviceListViewModel::connectToAllDevices() const
    {
        if (m_useCase)
        {
            m_useCase->connectToAllDevices();
        }
    }

    void DeviceListViewModel::disconnectFromAllDevices() const
    {
        if (m_useCase)
        {
            m_useCase->disconnectFromAllDevices();
        }
    }

    void DeviceListViewModel::updateDeviceName(const QString& deviceId, const QString& name) const
    {
        if (m_useCase)
        {
            m_useCase->updateDeviceName(deviceId, name);
        }
    }

    void DeviceListViewModel::onDeviceDiscovered(const std::shared_ptr<core::entities::Device>& device)
    {
        m_device_store.append(device);
        m_devices.append(device.get());
        emit devicesChanged();
    }

    void DeviceListViewModel::onDeviceConnected(const QString& deviceId)
    {
        updateConnectedDeviceCount();
        emit deviceConnected(deviceId);
    }

    void DeviceListViewModel::onDeviceDisconnected(const QString& deviceId)
    {
        updateConnectedDeviceCount();
        emit deviceDisconnected(deviceId);
    }

    void DeviceListViewModel::onDeviceStatusChanged(const QString& deviceId, const core::entities::DeviceStatus status)
    {
        for (const auto device : m_devices)
        {
            if (device && device->id() == deviceId)
            {
                device->setStatus(status);
                break;
            }
        }
    }

    void DeviceListViewModel::onErrorOccurred(const QString& error)
    {
        setErrorMessage(error);
    }

    void DeviceListViewModel::updateConnectedDeviceCount()
    {
        int count = 0;
        for (const auto device : m_devices)
        {
            if (device && device->connected())
            {
                count++;
            }
        }

        if (m_connectedDeviceCount != count)
        {
            m_connectedDeviceCount = count;
            emit connectedDeviceCountChanged();
        }
    }

    void DeviceListViewModel::setLoading(const bool loading)
    {
        if (m_isLoading != loading)
        {
            m_isLoading = loading;
            emit isLoadingChanged();
        }
    }

    void DeviceListViewModel::setErrorMessage(const QString& error)
    {
        if (m_errorMessage != error)
        {
            m_errorMessage = error;
            emit errorMessageChanged();
        }
    }
}
