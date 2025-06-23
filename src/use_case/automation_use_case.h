#ifndef AUTOMATION_USE_CASE_H
#define AUTOMATION_USE_CASE_H

#include <QObject>
#include <QString>
#include <QList>
#include <memory>
#include "../core/interfaces/command_executor.h"

namespace use_case
{
    class AutomationUseCase final : public QObject
    {
        Q_OBJECT

    public:
        explicit AutomationUseCase(
            const std::shared_ptr<core::interfaces::CommandExecutor>& executor,
            QObject* parent = nullptr
        );

        bool runScript(const QString& deviceId, const QString& scriptPath) const;
        bool runScriptOnAllDevices(const QList<QString>& deviceIds, const QString& scriptPath) const;
        bool scheduleScript(const QString& deviceId, const QString& scriptPath, const QString& schedule) const;

    signals:
        void scriptCompleted(const QString& deviceId, bool success, const QString& output);
        void errorOccurred(const QString& error);

    private:
        std::shared_ptr<core::interfaces::CommandExecutor> m_executor;
    };
}

#endif
