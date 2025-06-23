#ifndef DEVICE_LIST_VIEW_MODEL_H
#define DEVICE_LIST_VIEW_MODEL_H

#include <QQmlListProperty>
#include <memory>
#include "../../use_case/device_management_use_case.h"
#include "../../core/entities/device.h"

namespace presentation::view_models
{
    class DeviceListViewModel final : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QQmlListProperty<core::entities::Device> devices READ devices NOTIFY devicesChanged)
        Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
        Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
        Q_PROPERTY(int connectedDeviceCount READ connectedDeviceCount NOTIFY connectedDeviceCountChanged)

    public:
        explicit DeviceListViewModel(
            const std::shared_ptr<use_case::DeviceManagementUseCase>& useCase,
            QObject* parent = nullptr
        );

        QQmlListProperty<core::entities::Device> devices();
        bool isLoading() const { return m_isLoading; }
        QString errorMessage() const { return m_errorMessage; }
        int connectedDeviceCount() const { return m_connectedDeviceCount; }

        Q_INVOKABLE void refreshDevices();
        Q_INVOKABLE void connectToDevice(const QString& deviceId) const;
        Q_INVOKABLE void disconnectFromDevice(const QString& deviceId) const;
        Q_INVOKABLE void connectToAllDevices() const;
        Q_INVOKABLE void disconnectFromAllDevices() const;
        Q_INVOKABLE void updateDeviceName(const QString& deviceId, const QString& name) const;

    signals:
        void devicesChanged();
        void isLoadingChanged();
        void errorMessageChanged();
        void connectedDeviceCountChanged();
        void deviceConnected(const QString& deviceId);
        void deviceDisconnected(const QString& deviceId);

    private slots:
        void onDeviceDiscovered(const std::shared_ptr<core::entities::Device>& device);
        void onDeviceConnected(const QString& deviceId);
        void onDeviceDisconnected(const QString& deviceId);
        void onDeviceStatusChanged(const QString& deviceId, core::entities::DeviceStatus status);
        void onErrorOccurred(const QString& error);

    private:
        std::shared_ptr<use_case::DeviceManagementUseCase> m_useCase;
        QList<std::shared_ptr<core::entities::Device>> m_device_store;
        QList<core::entities::Device*> m_devices;
        bool m_isLoading;
        QString m_errorMessage;
        int m_connectedDeviceCount;

        void updateConnectedDeviceCount();
        void setLoading(bool loading);
        void setErrorMessage(const QString& error);
    };
}

#endif
