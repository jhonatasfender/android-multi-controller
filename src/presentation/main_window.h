#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <memory>

namespace use_case { class DeviceManagementUseCase; }
namespace presentation::view_models {
    class DeviceListViewModel;
    class MultiDeviceMirrorViewModel;
}
namespace presentation {
    class DeviceImageProvider;
}

namespace presentation
{
    class MainWindow : public QObject
    {
        Q_OBJECT
    public:
        explicit MainWindow(QObject *parent = nullptr);
        void initialize(QQmlApplicationEngine& engine);

    private:
        std::shared_ptr<use_case::DeviceManagementUseCase> m_deviceUseCase;
        view_models::DeviceListViewModel* m_deviceListViewModel;
        view_models::MultiDeviceMirrorViewModel* m_multiDeviceMirrorViewModel;
        DeviceImageProvider* m_imageProvider;
    };
}

#endif
