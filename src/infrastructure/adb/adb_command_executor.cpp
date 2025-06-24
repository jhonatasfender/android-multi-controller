#include "adb_command_executor.h"
#include <QProcess>
#include <QDir>
#include <QThread>

namespace infrastructure::adb
{
    AdbCommandExecutor::AdbCommandExecutor(
        QObject* parent
    ) : QObject(parent), m_adbPath("adb")
    {
        QProcess process;
        process.start("which", QStringList() << "adb");
        process.waitForFinished();

        if (process.exitCode() == 0)
        {
            m_adbPath = process.readAllStandardOutput().trimmed();
        }
    }

    AdbCommandExecutor::~AdbCommandExecutor()
    {
        for (const auto process : m_processes.values())
        {
            if (process->state() != QProcess::NotRunning)
            {
                process->terminate();

                if (!process->waitForFinished(2000))
                {
                    process->kill();
                    process->waitForFinished(1000);
                }
            }
            process->deleteLater();
        }
        m_processes.clear();

        m_callbacks.clear();
        m_binaryCallbacks.clear();
        m_commands.clear();
        m_deviceIds.clear();
    }

    bool AdbCommandExecutor::executeCommand(const QString& deviceId, const QString& command)
    {
        const QString output = executeAdbCommandWithDevice(deviceId, QStringList() << "shell" << command);
        return !output.isEmpty();
    }

    bool AdbCommandExecutor::executeCommandAsync(
        const QString& deviceId,
        const QString& command,
        const std::function<void(bool, const QString&)> callback
    )
    {
        const auto process = new QProcess(this);

        connect(process, &QProcess::finished, this, &AdbCommandExecutor::onProcessFinished);
        connect(process, &QProcess::errorOccurred, this, &AdbCommandExecutor::onProcessError);

        m_callbacks[process] = callback;
        m_commands[process] = command;
        m_deviceIds[process] = deviceId;

        if (m_processes.contains(deviceId))
        {
            cleanupProcess(m_processes[deviceId]);
        }
        m_processes[deviceId] = process;

        QStringList arguments;
        arguments << "-s" << deviceId << "shell" << command;

        process->start(m_adbPath, arguments);
        return process->state() != QProcess::NotRunning;
    }

    bool AdbCommandExecutor::executeScreenCaptureAsync(
        const QString& deviceId,
        const std::function<void(bool, const QByteArray&)>& callback
    )
    {
        QProcess process;
        QStringList arguments;
        arguments << "-s" << deviceId << "exec-out" << "screencap -p";

        process.start(m_adbPath, arguments);

        if (!process.waitForStarted())
        {
            callback(false, QByteArray());
            return false;
        }

        if (!process.waitForFinished(30000))
        {
            process.kill();
            callback(false, QByteArray());
            return false;
        }

        const QByteArray output = process.readAllStandardOutput();
        const QString error = process.readAllStandardError();
        const int exitCode = process.exitCode();

        const bool success = exitCode == 0 && !output.isEmpty();

        if (callback)
        {
            callback(success, success ? output : QByteArray());
        }

        emit commandExecuted(deviceId, "screencap -p", success, success ? "Image captured" : error);

        return success;
    }

    bool AdbCommandExecutor::startScreenCapture(const QString& deviceId)
    {
        const QString command = "screencap -p /sdcard/screenshot.png";
        return executeCommand(deviceId, command);
    }

    bool AdbCommandExecutor::stopScreenCapture(const QString& deviceId)
    {
        return true;
    }

    bool AdbCommandExecutor::sendTouchEvent(const QString& deviceId, const int x, const int y, int action)
    {
        const QString command = QString("input tap %1 %2").arg(x).arg(y);
        return executeCommand(deviceId, command);
    }

    bool AdbCommandExecutor::sendKeyEvent(const QString& deviceId, const int keyCode, int action)
    {
        const QString command = QString("input keyevent %1").arg(keyCode);
        return executeCommand(deviceId, command);
    }

    bool AdbCommandExecutor::pushFile(const QString& deviceId, const QString& localPath, const QString& remotePath)
    {
        QStringList arguments;
        arguments << "-s" << deviceId << "push" << localPath << remotePath;
        const QString output = executeAdbCommand(arguments);
        return !output.contains("error");
    }

    bool AdbCommandExecutor::pullFile(const QString& deviceId, const QString& remotePath, const QString& localPath)
    {
        QStringList arguments;
        arguments << "-s" << deviceId << "pull" << remotePath << localPath;
        const QString output = executeAdbCommand(arguments);
        return !output.contains("error");
    }

    bool AdbCommandExecutor::installApp(const QString& deviceId, const QString& apkPath)
    {
        QStringList arguments;
        arguments << "-s" << deviceId << "install" << apkPath;
        const QString output = executeAdbCommand(arguments);
        return output.contains("Success");
    }

    bool AdbCommandExecutor::uninstallApp(const QString& deviceId, const QString& packageName)
    {
        QStringList arguments;
        arguments << "-s" << deviceId << "uninstall" << packageName;
        const QString output = executeAdbCommand(arguments);
        return output.contains("Success");
    }

    bool AdbCommandExecutor::launchApp(const QString& deviceId, const QString& packageName)
    {
        const QString command = QString("monkey -p %1 -c android.intent.category.LAUNCHER 1").arg(packageName);
        return executeCommand(deviceId, command);
    }

    void AdbCommandExecutor::onProcessFinished(const int exitCode, QProcess::ExitStatus exitStatus)
    {
        const auto process = qobject_cast<QProcess*>(sender());
        if (!process)
        {
            return;
        }

        const QString output = process->readAllStandardOutput();
        const QString error = process->readAllStandardError();

        const bool success = exitCode == 0;
        const QString result = success ? output : error;

        const QString deviceId = m_deviceIds.value(process);
        const QString command = m_commands.value(process);

        if (const auto callback = m_callbacks.value(process))
        {
            callback(success, result);
        }

        emit commandExecuted(deviceId, command, success, result);

        cleanupProcess(process);
    }

    void AdbCommandExecutor::onProcessError(const QProcess::ProcessError error)
    {
        const auto process = qobject_cast<QProcess*>(sender());
        if (!process)
        {
            return;
        }

        const QString errorMsg = QString("Process error: %1").arg(error);
        const QString deviceId = m_deviceIds.value(process);

        if (const auto callback = m_callbacks.value(process))
        {
            callback(false, errorMsg);
        }

        emit this->errorOccurred(deviceId, errorMsg);

        cleanupProcess(process);
    }

    void AdbCommandExecutor::cleanupProcess(QProcess* process)
    {
        if (!process)
        {
            return;
        }

        if (process->state() != QProcess::NotRunning)
        {
            process->terminate();

            if (constexpr int timeout = 15000; !process->waitForFinished(timeout))
            {
                process->kill();
                process->waitForFinished(5000);
            }
        }

        m_callbacks.remove(process);
        m_binaryCallbacks.remove(process);
        m_commands.remove(process);
        m_deviceIds.remove(process);

        for (auto it = m_processes.begin(); it != m_processes.end(); ++it)
        {
            if (it.value() == process)
            {
                m_processes.erase(it);
                break;
            }
        }

        process->deleteLater();
    }

    QString AdbCommandExecutor::executeAdbCommand(const QStringList& arguments) const
    {
        QProcess process;
        process.start(m_adbPath, arguments);
        process.waitForFinished();
        return process.readAllStandardOutput();
    }

    QString AdbCommandExecutor::executeAdbCommandWithDevice(const QString& deviceId, const QStringList& arguments) const
    {
        QStringList fullArgs;
        fullArgs << "-s" << deviceId << arguments;
        return executeAdbCommand(fullArgs);
    }

    void AdbCommandExecutor::stopAllProcesses()
    {
        for (auto process : m_processes.values())
        {
            if (process->state() != QProcess::NotRunning)
            {
                process->terminate();

                if (!process->waitForFinished(1000))
                {
                    process->kill();
                    process->waitForFinished(500);
                }
            }
        }

        m_callbacks.clear();
        m_binaryCallbacks.clear();
        m_commands.clear();
        m_deviceIds.clear();
        m_processes.clear();
    }
}
