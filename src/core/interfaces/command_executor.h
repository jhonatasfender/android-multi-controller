#ifndef COMMAND_EXECUTOR_H
#define COMMAND_EXECUTOR_H

#include <QString>
#include <functional>

namespace core::interfaces
{
    class CommandExecutor
    {
    public:
        virtual ~CommandExecutor() = default;

        virtual bool executeCommand(const QString& deviceId, const QString& command) = 0;
        virtual bool executeCommandAsync(
            const QString& deviceId,
            const QString& command,
            std::function<void(bool, const QString&)> callback
        ) = 0;

        virtual bool startScreenCapture(const QString& deviceId) = 0;
        virtual bool stopScreenCapture(const QString& deviceId) = 0;
        virtual bool sendTouchEvent(const QString& deviceId, int x, int y, int action) = 0;
        virtual bool sendKeyEvent(const QString& deviceId, int keyCode, int action) = 0;

        virtual bool pushFile(const QString& deviceId, const QString& localPath, const QString& remotePath) = 0;
        virtual bool pullFile(const QString& deviceId, const QString& remotePath, const QString& localPath) = 0;

        virtual bool installApp(const QString& deviceId, const QString& apkPath) = 0;
        virtual bool uninstallApp(const QString& deviceId, const QString& packageName) = 0;
        virtual bool launchApp(const QString& deviceId, const QString& packageName) = 0;
        virtual void stopAllProcesses() = 0;
    };
}

#endif
