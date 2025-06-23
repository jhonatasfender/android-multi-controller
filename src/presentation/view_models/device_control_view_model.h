#ifndef DEVICE_CONTROL_VIEW_MODEL_H
#define DEVICE_CONTROL_VIEW_MODEL_H

#include <memory>
#include "../../use_case/device_control_use_case.h"

namespace presentation::view_models
{
    class DeviceControlViewModel final : public QObject
    {
        Q_OBJECT

    public:
        explicit DeviceControlViewModel(
            const std::shared_ptr<use_case::DeviceControlUseCase>& useCase,
            QObject* parent = nullptr
        );

        Q_INVOKABLE bool executeCommand(const QString& deviceId, const QString& command) const;
        Q_INVOKABLE bool startScreenCapture(const QString& deviceId) const;
        Q_INVOKABLE bool stopScreenCapture(const QString& deviceId) const;
        Q_INVOKABLE bool sendTouchEvent(const QString& deviceId, int x, int y, int action) const;
        Q_INVOKABLE bool sendKeyEvent(const QString& deviceId, int keyCode, int action) const;

    signals:
        void commandExecuted(const QString& deviceId, bool success, const QString& output);
        void errorOccurred(const QString& error);

    private slots:
        void onCommandExecuted(const QString& deviceId, bool success, const QString& output);
        void onErrorOccurred(const QString& error);

    private:
        std::shared_ptr<use_case::DeviceControlUseCase> m_useCase;
    };
}

#endif
