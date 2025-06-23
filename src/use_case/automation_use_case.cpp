#include "automation_use_case.h"

namespace use_case
{
    AutomationUseCase::AutomationUseCase(
        const std::shared_ptr<core::interfaces::CommandExecutor>& executor,
        QObject* parent
    ) : QObject(parent), m_executor(executor)
    {
    }

    bool AutomationUseCase::runScript(const QString& deviceId, const QString& scriptPath) const
    {
        if (m_executor)
        {
            return m_executor->executeCommand(deviceId, "sh " + scriptPath);
        }
        return false;
    }

    bool AutomationUseCase::runScriptOnAllDevices(const QList<QString>& deviceIds, const QString& scriptPath) const
    {
        bool allSuccess = true;
        for (const QString& deviceId : deviceIds)
        {
            if (!runScript(deviceId, scriptPath))
            {
                allSuccess = false;
            }
        }
        return allSuccess;
    }

    bool AutomationUseCase::scheduleScript(
        const QString& deviceId,
        const QString& scriptPath,
        const QString& schedule
    ) const
    {
        if (m_executor)
        {
            const QString command = QString("echo '%1 %2' | crontab -").arg(schedule, scriptPath);
            return m_executor->executeCommand(deviceId, command);
        }
        return false;
    }
}
