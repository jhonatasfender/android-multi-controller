#include "native_streaming_service.h"
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QApplication>
#include <QThread>
#include <QTimer>
#include <QMetaObject>

namespace infrastructure::streaming {
    NativeStreamingService::NativeStreamingService(QObject* parent)
        : QObject(parent)
        , m_streamingMode(StreamingMode::SingleDevice)
        , m_autoReconnect(true)
        , m_maxReconnectAttempts(DEFAULT_MAX_RECONNECT_ATTEMPTS)
        , m_totalFrameCount(0)
        , m_totalErrorCount(0)
    {
        qDebug() << "NativeStreamingService: Initialized";
    }

    NativeStreamingService::~NativeStreamingService()
    {
        stopAllStreaming();
        qDebug() << "NativeStreamingService: Destroyed";
    }

    bool NativeStreamingService::startStreaming(const QString& deviceId, const QString& deviceIp, quint16 port)
    {
        if (isStreaming(deviceId)) {
            qDebug() << "NativeStreamingService: Device already streaming:" << deviceId;
            return true;
        }

        if (!deployServerToDevice(deviceId)) {
            qWarning() << "NativeStreamingService: Failed to deploy server to device:" << deviceId;
            return false;
        }

        if (!startServerOnDevice(deviceId)) {
            qWarning() << "NativeStreamingService: Failed to start server on device:" << deviceId;
            return false;
        }

        initializeSession(deviceId, deviceIp, port);

        return finishStreamingStartup(deviceId, deviceIp, port, nullptr);
    }

    bool NativeStreamingService::continueStreamingStartup(const QString& deviceId, const QString& deviceIp,
        quint16 port)
    {
        return finishStreamingStartup(deviceId, deviceIp, port, nullptr);
    }

    bool NativeStreamingService::finishStreamingStartup(
        const QString& deviceId,
        const QString& deviceIp,
        const quint16 port,
        StreamingWidget* widget
    )
    {
        const auto& session = m_sessions[deviceId];
        session->isActive = true;
        emit streamingStarted(deviceId);

        qDebug() << "NativeStreamingService: Started streaming for a device:" << deviceId;
        return true;
    }

    auto NativeStreamingService::stopStreaming(const QString& deviceId) -> bool
    {
        if (!isStreaming(deviceId)) {
            return true;
        }

        auto& session = m_sessions[deviceId];
        bool success = true;

        if (session->widget) {
            if (!session->widget->stopStreaming()) {
                success = false;
            }
            session->widget->disconnectFromDevice();
        }

        if (!stopServerOnDevice(deviceId)) {
            success = false;
        }

        session->isActive = false;
        emit streamingStopped(deviceId);

        return success;
    }

    bool NativeStreamingService::isStreaming(const QString& deviceId) const
    {
        const auto it = m_sessions.find(deviceId);
        return it != m_sessions.end() && it->second->isActive;
    }

    QList<QString> NativeStreamingService::getStreamingDevices() const
    {
        QList<QString> devices;
        for (const auto& pair : m_sessions) {
            if (pair.second->isActive) {
                devices.append(pair.first);
            }
        }
        return devices;
    }

    bool NativeStreamingService::startMultiDeviceStreaming(const QList<QString>& deviceIds,
        const QMap<QString, QString>& deviceIps)
    {
        if (m_streamingMode != StreamingMode::MultiDevice) {
            setStreamingMode(StreamingMode::MultiDevice);
        }

        bool allSuccess = true;
        for (const QString& deviceId : deviceIds) {
            const QString deviceIp = deviceIps.value(deviceId);
            if (deviceIp.isEmpty()) {
                qWarning() << "NativeStreamingService: No IP address for device:" << deviceId;
                allSuccess = false;
                continue;
            }

            if (!startStreaming(deviceId, deviceIp)) {
                allSuccess = false;
            }
        }

        return allSuccess;
    }

    void NativeStreamingService::stopAllStreaming()
    {
        const QList<QString> devices = getStreamingDevices();
        for (const QString& deviceId : devices) {
            stopStreaming(deviceId);
        }

        m_sessions.clear();
    }

    StreamingWidget* NativeStreamingService::getStreamingWidget(const QString& deviceId) const
    {
        const auto it = m_sessions.find(deviceId);
        return it != m_sessions.end() && it->second->widget ? it->second->widget.get() : nullptr;
    }

    StreamingWidget* NativeStreamingService::createStreamingWidget(const QString& deviceId, QWidget* parent)
    {
        if (!m_sessions.contains(deviceId)) {
            initializeSession(deviceId, "", DEFAULT_STREAMING_PORT);
        }

        StreamingWidget* widget = ensureWidget(deviceId);
        if (widget && parent) {
            widget->setParent(parent);
        }

        return widget;
    }

    void NativeStreamingService::removeStreamingWidget(const QString& deviceId)
    {
        if (isStreaming(deviceId)) {
            stopStreaming(deviceId);
        }

        cleanupSession(deviceId);
    }

    void NativeStreamingService::setStreamingMode(StreamingMode mode)
    {
        m_streamingMode = mode;
    }

    NativeStreamingService::StreamingMode NativeStreamingService::getStreamingMode() const
    {
        return m_streamingMode;
    }

    void NativeStreamingService::setAutoReconnect(bool enabled)
    {
        m_autoReconnect = enabled;
    }

    bool NativeStreamingService::isAutoReconnectEnabled() const
    {
        return m_autoReconnect;
    }

    void NativeStreamingService::setMaxReconnectAttempts(int attempts)
    {
        m_maxReconnectAttempts = attempts;
    }

    int NativeStreamingService::getMaxReconnectAttempts() const
    {
        return m_maxReconnectAttempts;
    }

    double NativeStreamingService::getAverageFPS(const QString& deviceId) const
    {
        const auto it = m_sessions.find(deviceId);
        return it != m_sessions.end() && it->second->widget
            ? it->second->widget->getCurrentFPS()
            : 0.0;
    }

    int NativeStreamingService::getTotalFrameCount() const
    {
        return m_totalFrameCount;
    }

    int NativeStreamingService::getTotalErrorCount() const
    {
        return m_totalErrorCount;
    }

    QMap<QString, double> NativeStreamingService::getAllDeviceFPS() const
    {
        QMap<QString, double> fpsMap;
        for (const auto& pair : m_sessions) {
            if (pair.second->widget) {
                fpsMap[pair.first] = pair.second->widget->getCurrentFPS();
            }
        }
        return fpsMap;
    }

    bool NativeStreamingService::deployServerToDevice(const QString& deviceId)
    {
        const QString architecture = getDeviceArchitecture(deviceId);
        if (architecture.isEmpty()) {
            qWarning() << "NativeStreamingService: Could not determine device architecture:" << deviceId;
            return false;
        }

        const QString binaryPath = getServerBinaryPath(architecture);
        if (binaryPath.isEmpty()) {
            qWarning() << "NativeStreamingService: Server binary not found for architecture:" << architecture;
            return false;
        }

        if (!pushServerToDevice(deviceId, binaryPath)) {
            return false;
        }

        const QString libPath = getLibcppSharedPath(architecture);
        if (!libPath.isEmpty()) {
            const QString remoteLibPath = QString("%1/%2").arg(SERVER_REMOTE_DIR, LIBCPP_SHARED_NAME);
            if (!pushServerToDevice(deviceId, libPath, remoteLibPath)) {
                qWarning() << "NativeStreamingService: Failed to push shared library, server may not work";
            }
        }

        const QString chmodCommand = QString("chmod 755 %1").arg(SERVER_REMOTE_PATH);
        if (!executeServerCommand(deviceId, chmodCommand)) {
            qWarning() << "NativeStreamingService: Failed to make server binary executable";
            return false;
        }

        emit serverDeploymentFinished(deviceId, true);
        return true;
    }

    bool NativeStreamingService::startServerOnDevice(const QString& deviceId)
    {
        if (isServerRunning(deviceId)) {
            qDebug() << "NativeStreamingService: Server already running on device:" << deviceId;
            return true;
        }

        const QString startCommand = QString("cd %1 && setsid sh -c 'LD_LIBRARY_PATH=. %2 --port 8080 --verbose > server.log 2>&1' &").arg(
            SERVER_REMOTE_DIR, SERVER_REMOTE_PATH);

        QProcess process;
        process.start("adb", QStringList() << "-s" << deviceId << "shell" << startCommand);

        if (!process.waitForFinished(10000)) {
            qWarning() << "NativeStreamingService: Failed to execute server command on device:" << deviceId;
            qWarning() << "Command:" << startCommand;
            qWarning() << "Error:" << process.readAllStandardError();
            return false;
        }

        if (process.exitCode() != 0) {
            qWarning() << "NativeStreamingService: Server start command failed on device:" << deviceId;
            qWarning() << "Exit code:" << process.exitCode();
            qWarning() << "Error:" << process.readAllStandardError();
            return false;
        }

        QThread::msleep(3000);

        const bool running = isServerRunning(deviceId);
        if (!running) {
            qWarning() << "NativeStreamingService: Server failed to start on device:" << deviceId;

            QProcess logProcess;
            logProcess.start("adb", QStringList() << "-s" << deviceId << "shell" << "cat /data/local/tmp/server.log");
            logProcess.waitForFinished(5000);
            const QString serverLog = logProcess.readAllStandardOutput();
            if (!serverLog.isEmpty()) {
                qWarning() << "Server log:" << serverLog;
            }
        }
        else {
            qDebug() << "NativeStreamingService: Server started successfully on device:" << deviceId;
        }

        return running;
    }

    bool NativeStreamingService::stopServerOnDevice(const QString& deviceId)
    {
        const QString killCommand = QString("pkill -f %1").arg(SERVER_BINARY_NAME);
        return executeServerCommand(deviceId, killCommand);
    }

    bool NativeStreamingService::isServerRunning(const QString& deviceId)
    {
        const QString checkCommand = QString("pgrep -f %1").arg(SERVER_BINARY_NAME);
        QProcess process;
        process.start("adb", QStringList() << "-s" << deviceId << "shell" << checkCommand);
        process.waitForFinished(5000);

        const QString output = process.readAllStandardOutput().trimmed();
        return !output.isEmpty();
    }

    void NativeStreamingService::onStreamingWidgetDestroyed(QObject* obj)
    {
        for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
            if (it->second->widget.get() == obj) {
                cleanupSession(it->first);
                break;
            }
        }
    }

    void NativeStreamingService::onFrameReceived(const QImage& frame)
    {
        StreamingWidget* widget = qobject_cast<StreamingWidget*>(sender());
        if (!widget) {
            return;
        }

        const QString deviceId = widget->getDeviceId();
        if (!deviceId.isEmpty()) {
            m_totalFrameCount++;
            emit frameReceived(deviceId, frame);
        }
    }

    void NativeStreamingService::onStatisticsUpdated(double fps, int frameCount, int errorCount)
    {
        StreamingWidget* widget = qobject_cast<StreamingWidget*>(sender());
        if (!widget) {
            return;
        }

        const QString deviceId = widget->getDeviceId();
        if (!deviceId.isEmpty()) {
            m_totalErrorCount += errorCount;
            emit statisticsUpdated(deviceId, fps, frameCount, errorCount);
        }
    }

    void NativeStreamingService::onDeviceConnected(const QString& deviceId)
    {
        emit deviceConnected(deviceId);
    }

    void NativeStreamingService::onDeviceDisconnected(const QString& deviceId)
    {
        emit deviceDisconnected(deviceId);

        if (m_autoReconnect && m_sessions.contains(deviceId)) {
            handleReconnection(deviceId);
        }
    }

    void NativeStreamingService::onConnectionError(const QString& error)
    {
        StreamingWidget* widget = qobject_cast<StreamingWidget*>(sender());
        if (!widget) {
            return;
        }

        const QString deviceId = widget->getDeviceId();
        if (!deviceId.isEmpty()) {
            emit streamingError(deviceId, error);

            if (m_autoReconnect && m_sessions.contains(deviceId)) {
                handleReconnection(deviceId);
            }
        }
    }

    void NativeStreamingService::initializeSession(const QString& deviceId, const QString& deviceIp, quint16 port)
    {
        if (m_sessions.contains(deviceId)) {
            return;
        }

        auto session = std::make_unique<StreamingSession>(deviceId, deviceIp, port);
        session->reconnectTimer = new QTimer(this);

        m_sessions[deviceId] = std::move(session);
    }

    void NativeStreamingService::cleanupSession(const QString& deviceId)
    {
        const auto it = m_sessions.find(deviceId);
        if (it != m_sessions.end()) {
            if (it->second->reconnectTimer) {
                it->second->reconnectTimer->stop();
                it->second->reconnectTimer->deleteLater();
            }

            m_sessions.erase(it);
        }
    }

    StreamingWidget* NativeStreamingService::ensureWidget(const QString& deviceId)
    {
        const auto it = m_sessions.find(deviceId);
        if (it == m_sessions.end()) {
            return nullptr;
        }

        auto& session = it->second;

        if (session->widget) {
            return session->widget.get();
        }

        if (!session->widgetCreationPending) {
            session->widgetCreationPending = true;
            QTimer::singleShot(0, this, [this, deviceId]()
                {
                    createWidgetSafely(deviceId);
                });
        }

        return nullptr;
    }

    void NativeStreamingService::createWidgetSafely(const QString& deviceId)
    {
        if (!QApplication::instance()) {
            qWarning() << "NativeStreamingService: QApplication not available, retrying widget creation for device:" <<
                deviceId;
            QTimer::singleShot(500, this, [this, deviceId]()
                {
                    createWidgetSafely(deviceId);
                });
            return;
        }

        if (QThread::currentThread() != QApplication::instance()->thread()) {
            qWarning() << "NativeStreamingService: Not on main thread, retrying widget creation for device:" <<
                deviceId;
            QMetaObject::invokeMethod(this, [this, deviceId]()
                {
                    createWidgetSafely(deviceId);
                }, Qt::QueuedConnection);
            return;
        }

        if (!QApplication::instance()->thread()->isRunning()) {
            qWarning() << "NativeStreamingService: Application thread not running, retrying widget creation for device:"
                << deviceId;
            QTimer::singleShot(500, this, [this, deviceId]()
                {
                    createWidgetSafely(deviceId);
                });
            return;
        }

        const auto it = m_sessions.find(deviceId);
        if (it == m_sessions.end()) {
            return;
        }

        auto& session = it->second;
        session->widgetCreationPending = false;

        if (!session->widget) {
            try {
                if (QApplication::instance() && QApplication::instance()->thread()->isRunning()) {
                    session->widget = std::make_unique<StreamingWidget>();
                    setupSessionConnections(session.get());
                    qDebug() << "NativeStreamingService: Created streaming widget for device:" << deviceId;
                }
                else {
                    qDebug() << "NativeStreamingService: Skipping widget creation (no GUI context) for device:" << deviceId;
                }
            }
            catch (const std::exception& e) {
                qCritical() << "NativeStreamingService: Failed to create widget:" << e.what();
                session->widget.reset();
                session->widgetCreationPending = true;
                QTimer::singleShot(1000, this, [this, deviceId]()
                    {
                        createWidgetSafely(deviceId);
                    });
            }
            catch (...) {
                qCritical() << "NativeStreamingService: Failed to create widget: Unknown exception";
                session->widget.reset();
                session->widgetCreationPending = true;
                QTimer::singleShot(1000, this, [this, deviceId]()
                    {
                        createWidgetSafely(deviceId);
                    });
            }
        }
    }

    void NativeStreamingService::setupSessionConnections(StreamingSession* session)
    {
        if (!session || !session->widget) {
            return;
        }

        connect(
            session->widget.get(), &StreamingWidget::frameReceived, this, &NativeStreamingService::onFrameReceived);
        connect(
            session->widget.get(), &StreamingWidget::statisticsUpdated, this,
            &NativeStreamingService::onStatisticsUpdated);
        connect(
            session->widget.get(), &StreamingWidget::deviceConnected, this,
            &NativeStreamingService::onDeviceConnected);
        connect(
            session->widget.get(), &StreamingWidget::deviceDisconnected, this,
            &NativeStreamingService::onDeviceDisconnected);
        connect(
            session->widget.get(), &StreamingWidget::connectionError, this,
            &NativeStreamingService::onConnectionError);
        connect(
            session->widget.get(), &QObject::destroyed, this, &NativeStreamingService::onStreamingWidgetDestroyed);

        session->reconnectTimer->setSingleShot(true);
        connect(session->reconnectTimer, &QTimer::timeout, this, [this, deviceId = session->deviceId]()
            {
                handleReconnection(deviceId);
            });
    }

    void NativeStreamingService::handleReconnection(const QString& deviceId)
    {
        const auto it = m_sessions.find(deviceId);
        if (it == m_sessions.end()) {
            return;
        }

        auto& session = it->second;
        if (session->reconnectAttempts >= m_maxReconnectAttempts) {
            qWarning() << "NativeStreamingService: Max reconnect attempts reached for device:" << deviceId;
            emit streamingError(deviceId, "Max reconnect attempts reached");
            return;
        }

        session->reconnectAttempts++;
        qDebug() << "NativeStreamingService: Attempting to reconnect to device:" << deviceId
            << "Attempt:" << session->reconnectAttempts;

        if (session->widget) {
            session->widget->connectToDevice(deviceId, session->deviceIp, session->port);
        }

        session->reconnectTimer->start(RECONNECT_DELAY_MS);
    }

    QString NativeStreamingService::getServerBinaryPath(const QString& architecture) const
    {
        if (!QApplication::instance()) {
            qWarning() << "NativeStreamingService: QApplication not available, cannot determine binary path";
            return QString();
        }

        const QString appDir = QApplication::applicationDirPath();

        const QString projectRoot = QDir(appDir).filePath("../../");
        const QString normalizedRoot = QDir(projectRoot).canonicalPath();

        if (!architecture.isEmpty()) {
            const QString archPath = QDir(normalizedRoot).filePath(
                QString("android_server/libs/%1/%2").arg(architecture, SERVER_BINARY_NAME));
            if (QFile::exists(archPath)) {
                qDebug() << "NativeStreamingService: Found server binary for architecture" << architecture << "at:" <<
                    archPath;
                return archPath;
            }
        }

        const QString binaryPath = QDir(appDir).filePath(SERVER_BINARY_NAME);
        if (QFile::exists(binaryPath)) {
            return binaryPath;
        }

        const QString androidServerPath = QDir(appDir).filePath("android_server/" + QString(SERVER_BINARY_NAME));
        if (QFile::exists(androidServerPath)) {
            return androidServerPath;
        }

        if (architecture.isEmpty()) {
            const QStringList commonArchs = { "arm64-v8a", "armeabi-v7a", "x86_64", "x86" };
            for (const QString& arch : commonArchs) {
                const QString archPath = QDir(appDir).filePath(
                    QString("android_server/libs/%1/%2").arg(arch, SERVER_BINARY_NAME));
                if (QFile::exists(archPath)) {
                    qDebug() << "NativeStreamingService: Found server binary for fallback architecture" << arch << "at:"
                        << archPath;
                    return archPath;
                }
            }
        }

        qWarning() << "NativeStreamingService: Server binary not found in expected locations for architecture:" <<
            architecture;
        return QString();
    }

    QString NativeStreamingService::getLibcppSharedPath(const QString& architecture) const
    {
        if (architecture.isEmpty()) {
            return QString();
        }

        if (!QApplication::instance()) {
            qWarning() << "NativeStreamingService: QApplication not available, cannot determine library path";
            return QString();
        }

        const QString appDir = QApplication::applicationDirPath();
        const QString projectRoot = QDir(appDir).filePath("../../");
        const QString normalizedRoot = QDir(projectRoot).canonicalPath();
        const QString libPath = QDir(normalizedRoot).filePath(
            QString("android_server/libs/%1/%2").arg(architecture, LIBCPP_SHARED_NAME));

        if (QFile::exists(libPath)) {
            qDebug() << "NativeStreamingService: Found libc++_shared.so for architecture" << architecture << "at:" <<
                libPath;
            return libPath;
        }

        qDebug() << "NativeStreamingService: libc++_shared.so not found for architecture:" << architecture;
        return QString();
    }

    bool NativeStreamingService::pushServerToDevice(const QString& deviceId, const QString& localPath,
        const QString& remotePath)
    {
        const QString actualRemotePath = remotePath.isEmpty() ? SERVER_REMOTE_PATH : remotePath;

        QProcess process;
        process.start("adb", QStringList() << "-s" << deviceId << "push" << localPath << actualRemotePath);
        process.waitForFinished(30000);

        if (process.exitCode() != 0) {
            qWarning() << "NativeStreamingService: Failed to push" << localPath << "to device:" << deviceId;
            qWarning() << "Error:" << process.readAllStandardError();
            return false;
        }

        qDebug() << "NativeStreamingService: Successfully pushed" << localPath << "to" << actualRemotePath <<
            "on device:" << deviceId;
        return true;
    }

    bool NativeStreamingService::executeServerCommand(const QString& deviceId, const QString& command)
    {
        QProcess process;
        process.start("adb", QStringList() << "-s" << deviceId << "shell" << command);
        process.waitForFinished(10000);

        if (process.exitCode() != 0) {
            qWarning() << "NativeStreamingService: Command failed:" << command;
            qWarning() << "Error:" << process.readAllStandardError();
            return false;
        }

        return true;
    }

    QString NativeStreamingService::getDeviceArchitecture(const QString& deviceId) const
    {
        QProcess process;
        process.start("adb", QStringList() << "-s" << deviceId << "shell" << "getprop" << "ro.product.cpu.abi");
        process.waitForFinished(5000);

        if (process.exitCode() != 0) {
            return QString();
        }

        return process.readAllStandardOutput().trimmed();
    }
}
