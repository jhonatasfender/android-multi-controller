#include "device_control_view_model.h"

namespace presentation::view_models
{
    DeviceControlViewModel::DeviceControlViewModel(
        const std::shared_ptr<use_case::DeviceControlUseCase>& useCase,
        QObject* parent
    )
        : QObject(parent)
          , m_useCase(useCase)
    {
        if (m_useCase)
        {
            connect(
                m_useCase.get(),
                &use_case::DeviceControlUseCase::commandExecuted,
                this, &DeviceControlViewModel::onCommandExecuted
            );
            connect(
                m_useCase.get(),
                &use_case::DeviceControlUseCase::errorOccurred,
                this,
                &DeviceControlViewModel::onErrorOccurred
            );
        }
    }

    bool DeviceControlViewModel::executeCommand(const QString& deviceId, const QString& command) const
    {
        if (m_useCase)
        {
            return m_useCase->executeCommand(deviceId, command);
        }
        return false;
    }

    bool DeviceControlViewModel::sendTouchEvent(
        const QString& deviceId,
        const int x,
        const int y,
        const int action
    ) const
    {
        if (m_useCase)
        {
            return m_useCase->sendTouchEvent(deviceId, x, y, action);
        }
        return false;
    }

    bool DeviceControlViewModel::sendKeyEvent(const QString& deviceId, const int keyCode, const int action) const
    {
        if (m_useCase)
        {
            return m_useCase->sendKeyEvent(deviceId, keyCode, action);
        }
        return false;
    }

    void DeviceControlViewModel::onCommandExecuted(const QString& deviceId, bool success, const QString& output)
    {
        emit commandExecuted(deviceId, success, output);
    }

    void DeviceControlViewModel::onErrorOccurred(const QString& error)
    {
        emit errorOccurred(error);
    }
}
