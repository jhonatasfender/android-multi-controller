#include "adb_service.h"
#include "adb_command_executor.h"
#include <QProcess>
#include <QThread>
#include <QRegularExpression>

namespace infrastructure::adb
{
    AdbService::AdbService(
        QObject* parent
    ) : QObject(parent), m_commandExecutor(std::make_unique<AdbCommandExecutor>(this))
    {
        QProcess process;
        process.start("which", QStringList() << "adb");
        process.waitForFinished();
        if (process.exitCode() == 0)
        {
            m_adbPath = process.readAllStandardOutput().trimmed();
        }
        else
        {
            m_adbPath = "adb";
        }
    }

    AdbService::~AdbService() = default;

    QList<std::shared_ptr<core::entities::Device>> AdbService::getConnectedDevices()
    {
        const QString output = executeAdbCommand(QStringList() << "devices" << "-l");
        parseDeviceList(output);
        return m_devices;
    }

    bool AdbService::isDeviceConnected(const QString& deviceId)
    {
        for (const auto& device : m_devices)
        {
            if (device->id() == deviceId)
            {
                return device->status() == core::entities::DeviceStatus::Authorized;
            }
        }
        return false;
    }

    bool AdbService::executeCommand(const QString& deviceId, const QString& command) const
    {
        return m_commandExecutor->executeCommand(deviceId, command);
    }

    bool AdbService::startServer() const
    {
        const QString output = executeAdbCommand(QStringList() << "start-server");
        return !output.contains("error");
    }

    bool AdbService::stopServer() const
    {
        const QString output = executeAdbCommand(QStringList() << "kill-server");
        return !output.contains("error");
    }

    AdbService::ServerStatus AdbService::getServerStatus() const
    {
        QProcess process;
        process.start(m_adbPath, QStringList() << "get-state");
        process.waitForFinished(1000);

        if (process.state() == QProcess::NotRunning)
        {
            if (const QString output = process.readAllStandardOutput().trimmed(); output == "device")
            {
                return ServerStatus::Running;
            }
        }

        return ServerStatus::Stopped;
    }


    QString AdbService::getAdbVersion() const
    {
        return executeAdbCommand(QStringList() << "version");
    }

    QString AdbService::getWirelessDebuggingInstructions() const
    {
        return QString(
            "To enable wireless debugging:\n"
            "1. Enable Developer Options on your Android device\n"
            "2. Enable 'Wireless debugging' in Developer Options\n"
            "3. Tap on 'Wireless debugging' to see pairing options\n"
            "4. Use 'Pair device with pairing code' or 'Pair device with QR code'\n"
            "5. Follow the on-screen instructions to pair your computer\n"
            "6. Once paired, use the IP address and port shown on the device"
        );
    }

    bool AdbService::connectToDevice(const QString& deviceId)
    {
        const DeviceConnectionType connectionType = getDeviceConnectionType(deviceId);
        
        switch (connectionType)
        {
            case DeviceConnectionType::WiFi:
            {
                const QString output = executeAdbCommand(QStringList() << "connect" << deviceId);
                const bool success = output.contains("connected to") || output.contains("already connected");
                if (!success)
                {
                    const QString errorMsg = QString("Failed to connect to Wi-Fi device '%1': %2").arg(deviceId, output.trimmed());
                    emit errorOccurred(errorMsg);
                }
                return success;
            }
            
            case DeviceConnectionType::WirelessDebugging:
            {
                QString connectionString = deviceId;
                
                if (deviceId.contains("._adb-tls-connect._tcp"))
                {
                    const QString output = executeAdbCommand(QStringList() << "connect" << deviceId);
                    
                    const bool success = output.contains("connected to") || output.contains("already connected");
                    if (!success)
                    {
                        const QString altOutput = executeAdbCommand(QStringList() << "connect" << deviceId << "--timeout" << "5000");
                        
                        const bool altSuccess = altOutput.contains("connected to") || altOutput.contains("already connected");
                        if (!altSuccess)
                        {
                            const QString errorMsg = QString("Failed to connect to wireless device '%1'. Make sure wireless debugging is enabled and the device is discoverable. Try using the IP address and port format (e.g., 192.168.1.100:5555).").arg(deviceId);
                            emit errorOccurred(errorMsg);
                        }
                        return altSuccess;
                    }
                    return success;
                }
                else if (deviceId.contains("._adb._tcp"))
                {
                    const QString output = executeAdbCommand(QStringList() << "connect" << deviceId);
                    
                    const bool success = output.contains("connected to") || output.contains("already connected");
                    if (!success)
                    {
                        const QString errorMsg = QString("Failed to connect to wireless device '%1': %2").arg(deviceId, output.trimmed());
                        emit errorOccurred(errorMsg);
                    }
                    return success;
                }
                else
                {
                    const QString output = executeAdbCommand(QStringList() << "connect" << deviceId);
                    
                    const bool success = output.contains("connected to") || output.contains("already connected");
                    if (!success)
                    {
                        const QString errorMsg = QString("Failed to connect to wireless device '%1': %2").arg(deviceId, output.trimmed());
                        emit errorOccurred(errorMsg);
                    }
                    return success;
                }
            }
            
            case DeviceConnectionType::USB:
            {
                if (isDeviceConnected(deviceId))
                {
                    return true;
                }
                else
                {
                    const QString errorMsg = QString("USB device '%1' is not currently connected").arg(deviceId);
                    emit errorOccurred(errorMsg);
                    return false;
                }
            }
            
            case DeviceConnectionType::Unknown:
            default:
            {
                const QString output = executeAdbCommand(QStringList() << "connect" << deviceId);
                
                const bool success = output.contains("connected to") || output.contains("already connected");
                if (!success)
                {
                    const QString errorMsg = QString("Failed to connect to device '%1' (unknown type): %2").arg(deviceId, output.trimmed());
                    emit errorOccurred(errorMsg);
                }
                return success;
            }
        }
    }

    bool AdbService::disconnectFromDevice(const QString& deviceId) const
    {
        const QString output = executeAdbCommand(QStringList() << "disconnect" << deviceId);
        return !output.contains("error") && !output.contains("failed");
    }

    AdbService::DeviceConnectionType AdbService::getDeviceConnectionType(const QString& deviceId)
    {
        const QRegularExpression ipRegex("^(\\d{1,3}\\.){3}\\d{1,3}(:\\d+)?$");
        if (ipRegex.match(deviceId).hasMatch())
        {
            return DeviceConnectionType::WiFi;
        }

        if (deviceId.contains("._adb-tls-connect._tcp") || 
            deviceId.contains("._adb._tcp") ||
            deviceId.contains("._adb-tls-pairing._tcp"))
        {
            return DeviceConnectionType::WirelessDebugging;
        }

        const QRegularExpression usbRegex("^[A-Za-z0-9]+$");
        if (usbRegex.match(deviceId).hasMatch())
        {
            return DeviceConnectionType::USB;
        }

        if (deviceId.contains(":") && !deviceId.contains("."))
        {
            return DeviceConnectionType::WirelessDebugging;
        }

        return DeviceConnectionType::Unknown;
    }

    void AdbService::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
    {
    }

    void AdbService::onProcessError(const QProcess::ProcessError error)
    {
        emit errorOccurred(QString("ADB process error: %1").arg(error));
    }

    QString AdbService::executeAdbCommand(const QStringList& arguments) const
    {
        QProcess process;
        process.start(m_adbPath, arguments);
        process.waitForFinished(-1);

        if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        {
            return "Error: " + process.readAllStandardError();
        }

        return process.readAllStandardOutput();
    }

    void AdbService::parseDeviceList(const QString& output)
    {
        m_devices.clear();

        for (const QStringList lines = output.split('\n', Qt::SkipEmptyParts); const QString& line : lines)
        {
            if (line.startsWith("List of devices") || line.trimmed().isEmpty())
                continue;

            QStringList tokens = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (tokens.size() < 2)
                continue;

            QString id = tokens[0];
            QString status = tokens[1];

            QString product, model, deviceName;
            for (const QString& token : tokens.mid(2))
            {
                if (token.startsWith("product:"))
                    product = token.section(':', 1);
                else if (token.startsWith("model:"))
                    model = token.section(':', 1);
                else if (token.startsWith("device:"))
                    deviceName = token.section(':', 1);
            }

            auto device = std::make_shared<core::entities::Device>(id, deviceName.isEmpty() ? id : deviceName);
            device->setModel(model);
            device->setManufacturer(product);
            device->setStatus(parseDeviceStatus(status));
            device->setConnected(status == "device");

            m_devices.append(device);
        }

        emit devicesUpdated(m_devices);
    }

    core::entities::DeviceStatus AdbService::parseDeviceStatus(const QString& status)
    {
        if (status == "device") return core::entities::DeviceStatus::Authorized;
        if (status == "offline") return core::entities::DeviceStatus::Offline;
        if (status == "unauthorized") return core::entities::DeviceStatus::Unauthorized;
        if (status == "bootloader") return core::entities::DeviceStatus::Bootloader;
        if (status == "recovery") return core::entities::DeviceStatus::Recovery;
        return core::entities::DeviceStatus::Unknown;
    }

    QString AdbService::getDeviceProperty(const QString& deviceId, const QString& property) const
    {
        const QString command = QString("shell getprop %1").arg(property);
        return m_commandExecutor->executeCommand(deviceId, command) ? "Success" : "Failure";
    }
}
