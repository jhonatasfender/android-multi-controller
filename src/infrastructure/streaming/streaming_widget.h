#ifndef STREAMING_WIDGET_H
#define STREAMING_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <memory>
#include "tcp_client.h"
#include "h264_decoder.h"
#include "../../core/entities/device.h"

namespace infrastructure::streaming
{
    class StreamingWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit StreamingWidget(QWidget* parent = nullptr);
        ~StreamingWidget() override;

        enum class StreamingState
        {
            Disconnected,
            Connecting,
            Connected,
            Streaming,
            Error
        };

        Q_ENUM(StreamingState)

        bool connectToDevice(const QString& deviceId, const QString& deviceIp, quint16 port = 27183);
        bool disconnectFromDevice();
        bool isConnected() const;
        bool isStreaming() const;
        
        bool startStreaming();
        bool stopStreaming();
        StreamingState getStreamingState() const;
        
        void setAutoResize(bool autoResize);
        void setAspectRatioMode(Qt::AspectRatioMode mode);
        void setOutputFormat(QImage::Format format);
        void setScalingMode(Qt::TransformationMode mode);
        
        QString getDeviceId() const;
        QString getDeviceIp() const;
        quint16 getDevicePort() const;
        
        double getCurrentFPS() const;
        int getFrameCount() const;
        int getErrorCount() const;
        QSize getVideoSize() const;
        
        void setInputEnabled(bool enabled);
        bool isInputEnabled() const;

    signals:
        void stateChanged(StreamingState state);
        void frameReceived(const QImage& frame);
        void statisticsUpdated(double fps, int frameCount, int errorCount);
        void connectionError(const QString& error);
        void streamingStarted();
        void streamingStopped();
        void deviceConnected(const QString& deviceId);
        void deviceDisconnected(const QString& deviceId);
        
        void touchEvent(int x, int y, int action);
        void keyEvent(int keyCode, int action);

    protected:
        void paintEvent(QPaintEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;
        void keyReleaseEvent(QKeyEvent* event) override;

    private slots:
        void onTcpConnected(const QString& deviceId);
        void onTcpDisconnected(const QString& deviceId);
        void onTcpError(const QString& deviceId, const QString& error);
        void onVideoDataReceived(const QString& deviceId, const QByteArray& h264Data);
        void onFrameDecoded(const QImage& frame);
        void onDecoderError(const QString& error);
        void onDecoderStateChanged(H264Decoder::DecoderState state);
        void onStatisticsTimer();
        void updateStatistics();

    private:
        std::unique_ptr<TcpClient> m_tcpClient;
        std::unique_ptr<H264Decoder> m_decoder;
        
        StreamingState m_state;
        QString m_deviceId;
        QString m_deviceIp;
        quint16 m_devicePort;
        
        QPixmap m_currentFrame;
        QPixmap m_scaledFrame;
        QRect m_displayRect;
        Qt::AspectRatioMode m_aspectRatioMode;
        Qt::TransformationMode m_scalingMode;
        bool m_autoResize;
        
        QTimer* m_statisticsTimer;
        double m_currentFPS;
        int m_frameCount;
        int m_errorCount;
        QSize m_videoSize;
        
        bool m_inputEnabled;
        QPoint m_lastMousePos;
        bool m_mousePressed;
        
        void initializeComponents();
        void setupConnections();
        void setState(StreamingState state);
        void updateDisplayRect();
        void scaleFrame();
        void drawFrame(QPainter& painter);
        void drawOverlay(QPainter& painter);
        void drawStatistics(QPainter& painter);
        void drawConnectionStatus(QPainter& painter);
        
        QPoint screenToDevice(const QPoint& screenPos) const;
        QPoint deviceToScreen(const QPoint& devicePos) const;
        void sendTouchEvent(const QPoint& pos, int action);
        void sendKeyEvent(int keyCode, int action);
        
        QString getStateString() const;
        QString formatStatistics() const;
        QColor getStateColor() const;
        
        static constexpr int STATISTICS_UPDATE_INTERVAL = 1000;
        static constexpr int DEFAULT_STREAMING_PORT = 27183;
        static constexpr int OVERLAY_MARGIN = 10;
        static constexpr int OVERLAY_PADDING = 5;
        
        static constexpr int ACTION_DOWN = 0;
        static constexpr int ACTION_UP = 1;
        static constexpr int ACTION_MOVE = 2;
    };
}

#endif 