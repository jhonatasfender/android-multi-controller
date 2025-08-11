#include "streaming_widget.h"
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QFont>
#include <QFontMetrics>

namespace infrastructure::streaming
{
    StreamingWidget::StreamingWidget(QWidget* parent)
        : QWidget(parent)
          , m_state(StreamingState::Disconnected)
          , m_devicePort(DEFAULT_STREAMING_PORT)
          , m_aspectRatioMode(Qt::KeepAspectRatio)
          , m_scalingMode(Qt::SmoothTransformation)
          , m_autoResize(true)
          , m_statisticsTimer(new QTimer(this))
          , m_currentFPS(0.0)
          , m_frameCount(0)
          , m_errorCount(0)
          , m_inputEnabled(true)
          , m_mousePressed(false)
    {
        initializeComponents();
        setupConnections();

        setFocusPolicy(Qt::StrongFocus);
        setMouseTracking(true);
        setAttribute(Qt::WA_OpaquePaintEvent, true);

        m_statisticsTimer->start(STATISTICS_UPDATE_INTERVAL);

        qDebug() << "StreamingWidget: Initialized";
    }

    StreamingWidget::~StreamingWidget()
    {
        if (isStreaming())
        {
            stopStreaming();
        }

        if (isConnected())
        {
            disconnectFromDevice();
        }

        qDebug() << "StreamingWidget: Destroyed";
    }

    bool StreamingWidget::connectToDevice(const QString& deviceId, const QString& deviceIp, quint16 port)
    {
        if (m_state != StreamingState::Disconnected)
        {
            qWarning() << "StreamingWidget: Already connected or connecting";
            return false;
        }

        m_deviceId = deviceId;
        m_deviceIp = deviceIp;
        m_devicePort = port;

        setState(StreamingState::Connecting);

        const bool success = m_tcpClient->connectToDevice(deviceId, deviceIp, port);
        if (!success)
        {
            setState(StreamingState::Error);
            return false;
        }

        return true;
    }

    bool StreamingWidget::disconnectFromDevice()
    {
        if (m_state == StreamingState::Disconnected)
        {
            return true;
        }

        if (isStreaming())
        {
            stopStreaming();
        }

        const bool success = m_tcpClient->disconnectFromDevice(m_deviceId);
        setState(StreamingState::Disconnected);

        m_deviceId.clear();
        m_deviceIp.clear();
        m_devicePort = DEFAULT_STREAMING_PORT;

        return success;
    }

    bool StreamingWidget::isConnected() const
    {
        return m_state == StreamingState::Connected || m_state == StreamingState::Streaming;
    }

    bool StreamingWidget::isStreaming() const
    {
        return m_state == StreamingState::Streaming;
    }

    bool StreamingWidget::startStreaming()
    {
        if (m_state != StreamingState::Connected)
        {
            qWarning() << "StreamingWidget: Not connected to device";
            return false;
        }

        if (!m_decoder->initialize())
        {
            qWarning() << "StreamingWidget: Failed to initialize decoder";
            setState(StreamingState::Error);
            return false;
        }

        if (!m_tcpClient->requestVideoStream(m_deviceId))
        {
            qWarning() << "StreamingWidget: Failed to request video stream";
            setState(StreamingState::Error);
            return false;
        }

        setState(StreamingState::Streaming);
        emit streamingStarted();

        qDebug() << "StreamingWidget: Started streaming for device:" << m_deviceId;
        return true;
    }

    bool StreamingWidget::stopStreaming()
    {
        if (m_state != StreamingState::Streaming)
        {
            return true;
        }

        bool success = true;
        
        if (!m_tcpClient->stopVideoStream(m_deviceId))
        {
            success = false;
        }
        
        m_decoder->cleanup();

        setState(StreamingState::Connected);
        emit streamingStopped();

        return success;
    }

    StreamingWidget::StreamingState StreamingWidget::getStreamingState() const
    {
        return m_state;
    }

    void StreamingWidget::setAutoResize(bool autoResize)
    {
        m_autoResize = autoResize;
    }

    void StreamingWidget::setAspectRatioMode(Qt::AspectRatioMode mode)
    {
        m_aspectRatioMode = mode;
        updateDisplayRect();
        update();
    }

    void StreamingWidget::setOutputFormat(QImage::Format format)
    {
        m_decoder->setOutputFormat(format);
    }

    void StreamingWidget::setScalingMode(Qt::TransformationMode mode)
    {
        m_scalingMode = mode;
        scaleFrame();
        update();
    }

    QString StreamingWidget::getDeviceId() const
    {
        return m_deviceId;
    }

    QString StreamingWidget::getDeviceIp() const
    {
        return m_deviceIp;
    }

    quint16 StreamingWidget::getDevicePort() const
    {
        return m_devicePort;
    }

    double StreamingWidget::getCurrentFPS() const
    {
        return m_currentFPS;
    }

    int StreamingWidget::getFrameCount() const
    {
        return m_frameCount;
    }

    int StreamingWidget::getErrorCount() const
    {
        return m_errorCount;
    }

    QSize StreamingWidget::getVideoSize() const
    {
        return m_videoSize;
    }

    void StreamingWidget::setInputEnabled(bool enabled)
    {
        m_inputEnabled = enabled;
    }

    bool StreamingWidget::isInputEnabled() const
    {
        return m_inputEnabled;
    }

    void StreamingWidget::paintEvent(QPaintEvent* event)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.fillRect(rect(), Qt::black);

        if (!m_scaledFrame.isNull())
        {
            drawFrame(painter);
        }

        drawOverlay(painter);

        QWidget::paintEvent(event);
    }

    void StreamingWidget::resizeEvent(QResizeEvent* event)
    {
        updateDisplayRect();
        scaleFrame();
        QWidget::resizeEvent(event);
    }

    void StreamingWidget::mousePressEvent(QMouseEvent* event)
    {
        if (m_inputEnabled && isStreaming())
        {
            m_lastMousePos = event->pos();
            m_mousePressed = true;
            sendTouchEvent(event->pos(), ACTION_DOWN);
        }

        QWidget::mousePressEvent(event);
    }

    void StreamingWidget::mouseMoveEvent(QMouseEvent* event)
    {
        if (m_inputEnabled && isStreaming() && m_mousePressed)
        {
            sendTouchEvent(event->pos(), ACTION_MOVE);
        }

        m_lastMousePos = event->pos();
        QWidget::mouseMoveEvent(event);
    }

    void StreamingWidget::mouseReleaseEvent(QMouseEvent* event)
    {
        if (m_inputEnabled && isStreaming() && m_mousePressed)
        {
            sendTouchEvent(event->pos(), ACTION_UP);
            m_mousePressed = false;
        }

        QWidget::mouseReleaseEvent(event);
    }

    void StreamingWidget::keyPressEvent(QKeyEvent* event)
    {
        if (m_inputEnabled && isStreaming())
        {
            sendKeyEvent(event->key(), 1);
        }

        QWidget::keyPressEvent(event);
    }

    void StreamingWidget::keyReleaseEvent(QKeyEvent* event)
    {
        if (m_inputEnabled && isStreaming())
        {
            sendKeyEvent(event->key(), 0);
        }

        QWidget::keyReleaseEvent(event);
    }

    void StreamingWidget::onTcpConnected(const QString& deviceId)
    {
        if (deviceId == m_deviceId)
        {
            setState(StreamingState::Connected);
            emit deviceConnected(deviceId);
            qDebug() << "StreamingWidget: Connected to device:" << deviceId;
        }
    }

    void StreamingWidget::onTcpDisconnected(const QString& deviceId)
    {
        if (deviceId == m_deviceId)
        {
            setState(StreamingState::Disconnected);
            emit deviceDisconnected(deviceId);
            qDebug() << "StreamingWidget: Disconnected from device:" << deviceId;
        }
    }

    void StreamingWidget::onTcpError(const QString& deviceId, const QString& error)
    {
        if (deviceId == m_deviceId)
        {
            setState(StreamingState::Error);
            emit connectionError(error);
            qWarning() << "StreamingWidget: TCP error for device" << deviceId << ":" << error;
        }
    }

    void StreamingWidget::onVideoDataReceived(const QString& deviceId, const QByteArray& h264Data)
    {
        if (deviceId == m_deviceId && isStreaming())
        {
            m_decoder->decodeFrame(h264Data);
        }
    }

    void StreamingWidget::onFrameDecoded(const QImage& frame)
    {
        if (frame.isNull())
        {
            return;
        }

        m_currentFrame = QPixmap::fromImage(frame);
        m_videoSize = frame.size();
        m_frameCount++;

        if (m_autoResize)
        {
            m_decoder->setOutputSize(frame.size());
        }

        scaleFrame();
        update();

        emit frameReceived(frame);
    }

    void StreamingWidget::onDecoderError(const QString& error)
    {
        m_errorCount++;
        qWarning() << "StreamingWidget: Decoder error:" << error;

        if (m_errorCount > 10)
        {
            setState(StreamingState::Error);
            emit connectionError("Too many decoder errors");
        }
    }

    void StreamingWidget::onDecoderStateChanged(H264Decoder::DecoderState state)
    {
        if (state == H264Decoder::DecoderState::Error)
        {
            setState(StreamingState::Error);
        }
    }

    void StreamingWidget::onStatisticsTimer()
    {
        updateStatistics();
    }

    void StreamingWidget::updateStatistics()
    {
        if (m_decoder)
        {
            m_currentFPS = m_decoder->getAverageFPS();
            m_frameCount = m_decoder->getFrameCount();
            m_errorCount = m_decoder->getErrorCount();

            emit statisticsUpdated(m_currentFPS, m_frameCount, m_errorCount);
        }
    }

    void StreamingWidget::initializeComponents()
    {
        m_tcpClient = std::make_unique<TcpClient>(this);
        m_decoder = std::make_unique<H264Decoder>(this);

        m_decoder->setOutputFormat(QImage::Format_RGB888);
        m_decoder->setOutputSize(QSize(1080, 1920));
    }

    void StreamingWidget::setupConnections()
    {
        connect(m_tcpClient.get(), &TcpClient::connected, this, &StreamingWidget::onTcpConnected);
        connect(m_tcpClient.get(), &TcpClient::disconnected, this, &StreamingWidget::onTcpDisconnected);
        connect(m_tcpClient.get(), &TcpClient::connectionError, this, &StreamingWidget::onTcpError);
        connect(m_tcpClient.get(), &TcpClient::videoDataReceived, this, &StreamingWidget::onVideoDataReceived);

        connect(m_decoder.get(), &H264Decoder::frameDecoded, this, &StreamingWidget::onFrameDecoded);
        connect(m_decoder.get(), &H264Decoder::decoderError, this, &StreamingWidget::onDecoderError);
        connect(m_decoder.get(), &H264Decoder::stateChanged, this, &StreamingWidget::onDecoderStateChanged);

        connect(m_statisticsTimer, &QTimer::timeout, this, &StreamingWidget::onStatisticsTimer);
    }

    void StreamingWidget::setState(const StreamingState state)
    {
        if (m_state != state)
        {
            m_state = state;
            emit stateChanged(state);
            update();
        }
    }

    void StreamingWidget::updateDisplayRect()
    {
        if (m_currentFrame.isNull())
        {
            m_displayRect = rect();
            return;
        }

        const QSize frameSize = m_currentFrame.size();
        const QSize widgetSize = size();

        QSize scaledSize = frameSize;
        scaledSize.scale(widgetSize, m_aspectRatioMode);

        const int x = (widgetSize.width() - scaledSize.width()) / 2;
        const int y = (widgetSize.height() - scaledSize.height()) / 2;

        m_displayRect = QRect(QPoint(x, y), scaledSize);
    }

    void StreamingWidget::scaleFrame()
    {
        if (m_currentFrame.isNull())
        {
            return;
        }

        updateDisplayRect();

        if (m_displayRect.size() != m_currentFrame.size())
        {
            m_scaledFrame = m_currentFrame.scaled(m_displayRect.size(), m_aspectRatioMode, m_scalingMode);
        }
        else
        {
            m_scaledFrame = m_currentFrame;
        }
    }

    void StreamingWidget::drawFrame(QPainter& painter)
    {
        if (!m_scaledFrame.isNull())
        {
            painter.drawPixmap(m_displayRect, m_scaledFrame);
        }
    }

    void StreamingWidget::drawOverlay(QPainter& painter)
    {
        drawConnectionStatus(painter);
        drawStatistics(painter);
    }

    void StreamingWidget::drawStatistics(QPainter& painter)
    {
        if (m_state != StreamingState::Streaming)
        {
            return;
        }

        const QString stats = formatStatistics();
        if (stats.isEmpty())
        {
            return;
        }

        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 10));

        const QFontMetrics metrics(painter.font());
        const QRect textRect = metrics.boundingRect(stats);

        const QRect backgroundRect = textRect.adjusted(-OVERLAY_PADDING, -OVERLAY_PADDING,
                                                       OVERLAY_PADDING, OVERLAY_PADDING);

        painter.fillRect(backgroundRect.translated(OVERLAY_MARGIN, OVERLAY_MARGIN),
                         QColor(0, 0, 0, 128));

        painter.drawText(OVERLAY_MARGIN, OVERLAY_MARGIN - textRect.y(), stats);
    }

    void StreamingWidget::drawConnectionStatus(QPainter& painter)
    {
        const QString status = getStateString();
        const QColor color = getStateColor();

        painter.setPen(color);
        painter.setFont(QFont("Arial", 12, QFont::Bold));

        const QFontMetrics metrics(painter.font());
        const QRect textRect = metrics.boundingRect(status);

        const int x = width() - textRect.width() - OVERLAY_MARGIN;
        const int y = OVERLAY_MARGIN - textRect.y();

        const QRect backgroundRect = textRect.adjusted(-OVERLAY_PADDING, -OVERLAY_PADDING,
                                                       OVERLAY_PADDING, OVERLAY_PADDING);

        painter.fillRect(backgroundRect.translated(x, y), QColor(0, 0, 0, 128));
        painter.drawText(x, y, status);
    }

    QPoint StreamingWidget::screenToDevice(const QPoint& screenPos) const
    {
        if (m_videoSize.isEmpty() || m_displayRect.isEmpty())
        {
            return screenPos;
        }

        const QPoint relativePos = screenPos - m_displayRect.topLeft();

        const double scaleX = static_cast<double>(m_videoSize.width()) / m_displayRect.width();
        const double scaleY = static_cast<double>(m_videoSize.height()) / m_displayRect.height();

        return QPoint(static_cast<int>(relativePos.x() * scaleX),
                      static_cast<int>(relativePos.y() * scaleY));
    }

    QPoint StreamingWidget::deviceToScreen(const QPoint& devicePos) const
    {
        if (m_videoSize.isEmpty() || m_displayRect.isEmpty())
        {
            return devicePos;
        }

        const double scaleX = static_cast<double>(m_displayRect.width()) / m_videoSize.width();
        const double scaleY = static_cast<double>(m_displayRect.height()) / m_videoSize.height();

        const QPoint scaledPos(static_cast<int>(devicePos.x() * scaleX),
                               static_cast<int>(devicePos.y() * scaleY));

        return scaledPos + m_displayRect.topLeft();
    }

    void StreamingWidget::sendTouchEvent(const QPoint& pos, int action)
    {
        const QPoint devicePos = screenToDevice(pos);

        emit touchEvent(devicePos.x(), devicePos.y(), action);
    }

    void StreamingWidget::sendKeyEvent(int keyCode, int action)
    {
        emit keyEvent(keyCode, action);
    }

    QString StreamingWidget::getStateString() const
    {
        switch (m_state)
        {
        case StreamingState::Disconnected:
            return "Disconnected";
        case StreamingState::Connecting:
            return "Connecting...";
        case StreamingState::Connected:
            return "Connected";
        case StreamingState::Streaming:
            return "Streaming";
        case StreamingState::Error:
            return "Error";
        default:
            return "Unknown";
        }
    }

    QString StreamingWidget::formatStatistics() const
    {
        return QString("FPS: %1 | Frames: %2 | Errors: %3 | Size: %4x%5")
               .arg(m_currentFPS, 0, 'f', 1)
               .arg(m_frameCount)
               .arg(m_errorCount)
               .arg(m_videoSize.width())
               .arg(m_videoSize.height());
    }

    QColor StreamingWidget::getStateColor() const
    {
        switch (m_state)
        {
        case StreamingState::Disconnected:
            return Qt::red;
        case StreamingState::Connecting:
            return Qt::yellow;
        case StreamingState::Connected:
            return Qt::green;
        case StreamingState::Streaming:
            return Qt::cyan;
        case StreamingState::Error:
            return Qt::magenta;
        default:
            return Qt::white;
        }
    }
}
