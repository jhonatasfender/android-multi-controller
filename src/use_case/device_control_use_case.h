#ifndef DEVICE_CONTROL_USE_CASE_H
#define DEVICE_CONTROL_USE_CASE_H

#include <QObject>
#include <memory>
#include "../core/interfaces/command_executor.h"

namespace use_case
{
    class DeviceControlUseCase final : public QObject
    {
        Q_OBJECT

    public:
        explicit DeviceControlUseCase(
            const std::shared_ptr<core::interfaces::CommandExecutor>& executor,
            QObject* parent = nullptr
        );

        bool executeCommand(const QString& deviceId, const QString& command) const;

        bool sendTouchEvent(const QString& deviceId, int x, int y, int action) const;
        bool sendKeyEvent(const QString& deviceId, int keyCode, int action) const;

    signals:
        void commandExecuted(const QString& deviceId, bool success, const QString& output);
        void errorOccurred(const QString& error);

    private:
        std::shared_ptr<core::interfaces::CommandExecutor> m_executor;
    };
}

#endif
