#ifndef NATIVE_STREAMING_SERVICE_H
#define NATIVE_STREAMING_SERVICE_H

#include <memory>
#include <map>
#include "tcp_client.h"
#include "h264_decoder.h"
#include "streaming_widget.h"

namespace infrastructure::streaming
{
    class NativeStreamingService final : public QObject
    {
        Q_OBJECT

    public:
        explicit NativeStreamingService(QObject* parent = nullptr);
        ~NativeStreamingService() override;

        enum class StreamingMode
        {
            SingleDevice,
            MultiDevice
        };

        Q_ENUM(StreamingMode)

        bool startStreaming(const QString& deviceId, const QString& deviceIp, quint16 port = 27183);
        bool stopStreaming(const QString& deviceId);
        bool isStreaming(const QString& deviceId) const;
        QList<QString> getStreamingDevices() const;

        bool startMultiDeviceStreaming(const QList<QString>& deviceIds, const QMap<QString, QString>& deviceIps);
        void stopAllStreaming();

        StreamingWidget* getStreamingWidget(const QString& deviceId) const;
        StreamingWidget* createStreamingWidget(const QString& deviceId, QWidget* parent = nullptr);
        void removeStreamingWidget(const QString& deviceId);

        void setStreamingMode(StreamingMode mode);
        StreamingMode getStreamingMode() const;
        void setAutoReconnect(bool enabled);
        bool isAutoReconnectEnabled() const;
        void setMaxReconnectAttempts(int attempts);
        int getMaxReconnectAttempts() const;

        double getAverageFPS(const QString& deviceId) const;
        int getTotalFrameCount() const;
        int getTotalErrorCount() const;
        QMap<QString, double> getAllDeviceFPS() const;

        bool deployServerToDevice(const QString& deviceId);
        bool startServerOnDevice(const QString& deviceId);
        bool stopServerOnDevice(const QString& deviceId);
        static bool isServerRunning(const QString& deviceId);

    signals:
        void streamingStarted(const QString& deviceId);
        void streamingStopped(const QString& deviceId);
        void streamingError(const QString& deviceId, const QString& error);
        void frameReceived(const QString& deviceId, const QImage& frame);
        void statisticsUpdated(const QString& deviceId, double fps, int frameCount, int errorCount);
        void deviceConnected(const QString& deviceId);
        void deviceDisconnected(const QString& deviceId);
        void serverDeploymentFinished(const QString& deviceId, bool success);

    private slots:
        void onStreamingWidgetDestroyed(QObject* obj);
        void onFrameReceived(const QImage& frame);
        void onStatisticsUpdated(double fps, int frameCount, int errorCount);
        void onDeviceConnected(const QString& deviceId);
        void onDeviceDisconnected(const QString& deviceId);
        void onConnectionError(const QString& error);
        void createWidgetSafely(const QString& deviceId);

    private:
        struct StreamingSession
        {
            QString deviceId;
            QString deviceIp;
            quint16 port;
            std::unique_ptr<StreamingWidget> widget;
            bool isActive;
            int reconnectAttempts;
            QTimer* reconnectTimer;
            bool widgetCreationPending;

            StreamingSession(const QString& id, const QString& ip, const quint16 p)
                : deviceId(id), deviceIp(ip), port(p), isActive(false), reconnectAttempts(0), reconnectTimer(nullptr),
                  widgetCreationPending(false)
            {
            }
        };

        std::map<QString, std::unique_ptr<StreamingSession>> m_sessions;
        StreamingMode m_streamingMode;
        bool m_autoReconnect;
        int m_maxReconnectAttempts;

        int m_totalFrameCount;
        int m_totalErrorCount;

        void initializeSession(const QString& deviceId, const QString& deviceIp, quint16 port);
        void cleanupSession(const QString& deviceId);
        StreamingWidget* ensureWidget(const QString& deviceId);
        bool continueStreamingStartup(const QString& deviceId, const QString& deviceIp, quint16 port);
        bool finishStreamingStartup(
            const QString& deviceId,
            const QString& deviceIp,
            quint16 port,
            StreamingWidget* widget
        );
        void setupSessionConnections(StreamingSession* session);
        void handleReconnection(const QString& deviceId);

        QString getServerBinaryPath(const QString& architecture = QString()) const;
        QString getLibcppSharedPath(const QString& architecture) const;
        bool pushServerToDevice(
            const QString& deviceId,
            const QString& localPath,
            const QString& remotePath = QString()
        );
        bool executeServerCommand(const QString& deviceId, const QString& command);
        QString getDeviceArchitecture(const QString& deviceId) const;

        static constexpr int DEFAULT_STREAMING_PORT = 27183;
        static constexpr int DEFAULT_MAX_RECONNECT_ATTEMPTS = 5;
        static constexpr int RECONNECT_DELAY_MS = 3000;
        static constexpr auto SERVER_BINARY_NAME = "android_server";
        static constexpr auto SERVER_REMOTE_PATH = "/data/local/tmp/android_server";
        static constexpr auto SERVER_REMOTE_DIR = "/data/local/tmp";
        static constexpr auto LIBCPP_SHARED_NAME = "libc++_shared.so";
    };
}

#endif
