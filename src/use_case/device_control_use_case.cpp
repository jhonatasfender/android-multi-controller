#include "device_control_use_case.h"

namespace use_case
{
    DeviceControlUseCase::DeviceControlUseCase(
        const std::shared_ptr<core::interfaces::CommandExecutor>& executor,
        QObject* parent
    ) : QObject(parent), m_executor(executor)
    {
    }

    bool DeviceControlUseCase::executeCommand(const QString& deviceId, const QString& command) const
    {
        if (m_executor)
        {
            return m_executor->executeCommand(deviceId, command);
        }
        return false;
    }

    bool DeviceControlUseCase::startScreenCapture(const QString& deviceId) const
    {
        if (m_executor)
        {
            return m_executor->startScreenCapture(deviceId);
        }
        return false;
    }

    bool DeviceControlUseCase::stopScreenCapture(const QString& deviceId) const
    {
        if (m_executor)
        {
            return m_executor->stopScreenCapture(deviceId);
        }
        return false;
    }

    bool DeviceControlUseCase::sendTouchEvent(const QString& deviceId, const int x, const int y, const int action) const
    {
        if (m_executor)
        {
            return m_executor->sendTouchEvent(deviceId, x, y, action);
        }
        return false;
    }

    bool DeviceControlUseCase::sendKeyEvent(const QString& deviceId, const int keyCode, const int action) const
    {
        if (m_executor)
        {
            return m_executor->sendKeyEvent(deviceId, keyCode, action);
        }
        return false;
    }
}
