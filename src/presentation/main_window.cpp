#include "main_window.h"
#include <QQmlContext>
#include "view_models/device_list_view_model.h"
#include "view_models/multi_device_mirror_view_model.h"
#include "../use_case/device_management_use_case.h"
#include "../infrastructure/adb/adb_service.h"
#include "../infrastructure/persistence/adb_device_repository.h"
#include "../core/logging/logger.h"
#include "../core/entities/device.h"
#include "../core/entities/device_status.h"
#include "view_models/device_image_provider.h"

namespace presentation
{
    MainWindow::MainWindow(QObject* parent) : QObject(parent)
    {
        auto adbService = std::make_shared<infrastructure::adb::AdbService>();
        auto deviceRepository = std::make_shared<infrastructure::persistence::AdbDeviceRepository>(adbService);
        m_deviceUseCase = std::make_shared<use_case::DeviceManagementUseCase>(deviceRepository);

        m_imageProvider = new DeviceImageProvider();
        m_deviceListViewModel = new view_models::DeviceListViewModel(m_deviceUseCase, this);
        m_multiDeviceMirrorViewModel = new view_models::MultiDeviceMirrorViewModel(
            m_deviceUseCase,
            m_imageProvider,
            this
        );

        connect(
            m_deviceListViewModel,
            &view_models::DeviceListViewModel::devicesChanged,
            m_multiDeviceMirrorViewModel,
            &view_models::MultiDeviceMirrorViewModel::onDevicesUpdated
        );
    }

    void MainWindow::initialize(QQmlApplicationEngine& engine) const
    {
        engine.addImageProvider("deviceimages", m_imageProvider);
        qmlRegisterType<view_models::DeviceListViewModel>(
            "DeviceController",
            1,
            0,
            "DeviceListViewModel"
        );
        qmlRegisterType<view_models::MultiDeviceMirrorViewModel>(
            "DeviceController",
            1,
            0,
            "MultiDeviceMirrorViewModel"
        );
        qmlRegisterUncreatableType<core::entities::Device>(
            "DeviceController",
            1,
            0,
            "Device",
            "Device objects are written in C++ and passed to QML"
        );
        qmlRegisterUncreatableMetaObject(
            core::entities::staticMetaObject,
            "DeviceController",
            1,
            0,
            "DeviceStatus",
            "DeviceStatus is an enum and cannot be instantiated"
        );

        engine.rootContext()->setContextProperty("deviceListViewModel", m_deviceListViewModel);
        engine.rootContext()->setContextProperty("multiDeviceMirrorViewModel", m_multiDeviceMirrorViewModel);
    }
}
