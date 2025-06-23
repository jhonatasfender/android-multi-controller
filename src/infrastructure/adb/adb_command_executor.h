#ifndef ADB_COMMAND_EXECUTOR_H
#define ADB_COMMAND_EXECUTOR_H

#include <QProcess>
#include <QString>
#include <QMap>
#include <functional>
#include "../../core/interfaces/command_executor.h"

namespace infrastructure::adb
{
    class AdbCommandExecutor final : public QObject, public core::interfaces::CommandExecutor
    {
        Q_OBJECT

    public:
        explicit AdbCommandExecutor(QObject* parent = nullptr);
        ~AdbCommandExecutor() override;

        bool executeCommand(const QString& deviceId, const QString& command) override;
        bool executeCommandAsync(
            const QString& deviceId,
            const QString& command,
            const std::function<void(bool, const QString&)> callback
        ) override;
        bool executeScreenCaptureAsync(
            const QString& deviceId,
            const std::function<void(bool, const QByteArray&)> callback
        );
        bool startScreenCapture(const QString& deviceId) override;
        bool stopScreenCapture(const QString& deviceId) override;
        bool sendTouchEvent(const QString& deviceId, int x, int y, int action) override;
        bool sendKeyEvent(const QString& deviceId, int keyCode, int action) override;
        bool pushFile(const QString& deviceId, const QString& localPath, const QString& remotePath) override;
        bool pullFile(const QString& deviceId, const QString& remotePath, const QString& localPath) override;
        bool installApp(const QString& deviceId, const QString& apkPath) override;
        bool uninstallApp(const QString& deviceId, const QString& packageName) override;
        bool launchApp(const QString& deviceId, const QString& packageName) override;
        void stopAllProcesses();

    signals:
        void commandExecuted(const QString& deviceId, const QString& command, bool success, const QString& output);
        void errorOccurred(const QString& deviceId, const QString& error);

    private slots:
        void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
        void onProcessError(QProcess::ProcessError error);

    private:
        QString m_adbPath;
        QMap<QProcess*, std::function<void(bool, const QString&)>> m_callbacks;
        QMap<QProcess*, std::function<void(bool, const QByteArray&)>> m_binaryCallbacks;
        QMap<QProcess*, QString> m_commands;
        QMap<QProcess*, QString> m_deviceIds;
        QMap<QString, QProcess*> m_processes;

        QString executeAdbCommand(const QStringList& arguments) const;
        QString executeAdbCommandWithDevice(const QString& deviceId, const QStringList& arguments) const;
        void cleanupProcess(QProcess* process);
    };
}

#endif
