#ifndef ADB_SERVICE_H
#define ADB_SERVICE_H

#include <memory>
#include "adb_command_executor.h"
#include "../../core/entities/device.h"

namespace infrastructure::adb
{
    class AdbService final : public QObject
    {
        Q_OBJECT

    public:
        explicit AdbService(QObject* parent = nullptr);
        ~AdbService() override;

        enum class ServerStatus
        {
            Running,
            Stopped,
            Error
        };

        Q_ENUM(ServerStatus)


        QList<std::shared_ptr<core::entities::Device>> getConnectedDevices();
        bool isDeviceConnected(const QString& deviceId);
        bool executeCommand(const QString& deviceId, const QString& command) const;

        bool startServer() const;
        bool stopServer() const;
        ServerStatus getServerStatus() const;
        QString getAdbVersion() const;

        static QString getWirelessDebuggingInstructions();

        bool connectToDevice(const QString& deviceId);
        bool disconnectFromDevice(const QString& deviceId) const;

        enum class DeviceConnectionType
        {
            USB,
            WiFi,
            WirelessDebugging,
            Unknown
        };

        static DeviceConnectionType getDeviceConnectionType(const QString& deviceId);

    signals:
        void devicesUpdated(const QList<std::shared_ptr<core::entities::Device>>& devices);
        void deviceConnected(const std::shared_ptr<core::entities::Device>& device);
        void deviceDisconnected(const QString& deviceId);
        void errorOccurred(const QString& error);

    private slots:
        static void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
        void onProcessError(QProcess::ProcessError error);

    private:
        std::unique_ptr<AdbCommandExecutor> m_commandExecutor;
        QList<std::shared_ptr<core::entities::Device>> m_devices;
        QString m_adbPath;

        QString executeAdbCommand(const QStringList& arguments) const;
        void parseDeviceList(const QString& output);
        static core::entities::DeviceStatus parseDeviceStatus(const QString& status);
        QString getDeviceProperty(const QString& deviceId, const QString& property) const;
    };
}

#endif
