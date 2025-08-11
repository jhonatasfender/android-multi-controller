#include "h264_decoder.h"
#include <QDebug>
#include <QTime>
#include <QMutexLocker>

namespace infrastructure::streaming
{
    H264Decoder::H264Decoder(QObject* parent)
        : QObject(parent)
        , m_context(std::make_unique<DecoderContext>())
        , m_state(DecoderState::Idle)
        , m_outputFormat(QImage::Format_RGB888)
        , m_outputSize(DEFAULT_OUTPUT_WIDTH, DEFAULT_OUTPUT_HEIGHT)
        , m_autoResize(true)
        , m_frameCount(0)
        , m_errorCount(0)
        , m_averageFPS(0.0)
    {
        m_statisticsStartTime = QTime::currentTime();
        m_lastFrameTime = QTime::currentTime();
    }

    H264Decoder::~H264Decoder()
    {
        cleanup();
    }

    bool H264Decoder::initialize()
    {
        QMutexLocker locker(&m_mutex);
        
        if (m_state != DecoderState::Idle)
        {
            qWarning() << "H264Decoder: Already initialized";
            return false;
        }

        if (!initializeCodec())
        {
            setState(DecoderState::Error);
            return false;
        }

        if (!initializeFrames())
        {
            setState(DecoderState::Error);
            return false;
        }

        if (!initializeScaler())
        {
            setState(DecoderState::Error);
            return false;
        }

        setState(DecoderState::Initialized);
        resetStatistics();
        
        qDebug() << "H264Decoder: Successfully initialized";
        return true;
    }

    void H264Decoder::cleanup()
    {
        QMutexLocker locker(&m_mutex);
        cleanupContext();
        setState(DecoderState::Idle);
    }

    bool H264Decoder::isInitialized() const
    {
        QMutexLocker locker(&m_mutex);
        return m_state == DecoderState::Initialized || m_state == DecoderState::Decoding;
    }

    H264Decoder::DecoderState H264Decoder::getState() const
    {
        QMutexLocker locker(&m_mutex);
        return m_state;
    }

    bool H264Decoder::decodeFrame(const QByteArray& h264Data)
    {
        QMutexLocker locker(&m_mutex);
        
        if (m_state != DecoderState::Initialized && m_state != DecoderState::Decoding)
        {
            qWarning() << "H264Decoder: Not initialized for decoding";
            return false;
        }

        if (h264Data.isEmpty())
        {
            qWarning() << "H264Decoder: Empty H.264 data received";
            return false;
        }

        setState(DecoderState::Decoding);

        av_packet_unref(m_context->packet);
        m_context->packet->data = reinterpret_cast<uint8_t*>(const_cast<char*>(h264Data.data()));
        m_context->packet->size = h264Data.size();

        int ret = avcodec_send_packet(m_context->codecContext, m_context->packet);
        if (ret < 0)
        {
            logError("avcodec_send_packet", ret);
            m_errorCount++;
            
            if (m_errorCount > MAX_DECODE_ERRORS)
            {
                setState(DecoderState::Error);
                emit decoderError("Too many decode errors");
                return false;
            }
            return false;
        }

        ret = avcodec_receive_frame(m_context->codecContext, m_context->frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            return true;
        }
        else if (ret < 0)
        {
            logError("avcodec_receive_frame", ret);
            m_errorCount++;
            return false;
        }

        processDecodedFrame(m_context->frame);
        m_frameCount++;
        updateStatistics();

        return true;
    }

    void H264Decoder::resetDecoder()
    {
        QMutexLocker locker(&m_mutex);
        
        if (m_context->codecContext)
        {
            avcodec_flush_buffers(m_context->codecContext);
        }
        
        resetStatistics();
        qDebug() << "H264Decoder: Decoder reset";
    }

    void H264Decoder::setOutputFormat(QImage::Format format)
    {
        QMutexLocker locker(&m_mutex);
        
        if (m_outputFormat != format)
        {
            m_outputFormat = format;
            updateScaler();
        }
    }

    void H264Decoder::setOutputSize(const QSize& size)
    {
        QMutexLocker locker(&m_mutex);
        
        if (m_outputSize != size && size.isValid())
        {
            m_outputSize = size;
            updateScaler();
        }
    }

    QSize H264Decoder::getOutputSize() const
    {
        QMutexLocker locker(&m_mutex);
        return m_outputSize;
    }

    QImage::Format H264Decoder::getOutputFormat() const
    {
        QMutexLocker locker(&m_mutex);
        return m_outputFormat;
    }

    int H264Decoder::getFrameCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_frameCount;
    }

    int H264Decoder::getErrorCount() const
    {
        QMutexLocker locker(&m_mutex);
        return m_errorCount;
    }

    double H264Decoder::getAverageFPS() const
    {
        QMutexLocker locker(&m_mutex);
        return m_averageFPS;
    }

    void H264Decoder::resetStatistics()
    {
        m_frameCount = 0;
        m_errorCount = 0;
        m_averageFPS = 0.0;
        m_statisticsStartTime = QTime::currentTime();
        m_lastFrameTime = QTime::currentTime();
    }

    void H264Decoder::onFrameDecoded()
    {
    }

    bool H264Decoder::initializeCodec()
    {
        const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!codec)
        {
            qCritical() << "H264Decoder: H.264 codec not found";
            return false;
        }

        m_context->codecContext = avcodec_alloc_context3(codec);
        if (!m_context->codecContext)
        {
            qCritical() << "H264Decoder: Could not allocate codec context";
            return false;
        }

        m_context->codecContext->thread_count = 0;
        m_context->codecContext->thread_type = FF_THREAD_FRAME;

        int ret = avcodec_open2(m_context->codecContext, codec, nullptr);
        if (ret < 0)
        {
            logError("avcodec_open2", ret);
            return false;
        }

        return true;
    }

    bool H264Decoder::initializeFrames()
    {
        m_context->frame = av_frame_alloc();
        if (!m_context->frame)
        {
            qCritical() << "H264Decoder: Could not allocate frame";
            return false;
        }

        m_context->frameRGB = av_frame_alloc();
        if (!m_context->frameRGB)
        {
            qCritical() << "H264Decoder: Could not allocate RGB frame";
            return false;
        }

        m_context->packet = av_packet_alloc();
        if (!m_context->packet)
        {
            qCritical() << "H264Decoder: Could not allocate packet";
            return false;
        }

        return setupOutputFrame();
    }

    bool H264Decoder::initializeScaler()
    {
        updateScaler();
        return m_context->swsContext != nullptr;
    }

    void H264Decoder::cleanupContext()
    {
        if (m_context->swsContext)
        {
            sws_freeContext(m_context->swsContext);
            m_context->swsContext = nullptr;
        }

        if (m_context->buffer)
        {
            av_free(m_context->buffer);
            m_context->buffer = nullptr;
        }

        if (m_context->frameRGB)
        {
            av_frame_free(&m_context->frameRGB);
        }

        if (m_context->frame)
        {
            av_frame_free(&m_context->frame);
        }

        if (m_context->packet)
        {
            av_packet_free(&m_context->packet);
        }

        if (m_context->codecContext)
        {
            avcodec_free_context(&m_context->codecContext);
        }
    }

    void H264Decoder::setState(DecoderState state)
    {
        if (m_state != state)
        {
            m_state = state;
            emit stateChanged(state);
        }
    }

    void H264Decoder::updateStatistics()
    {
        if (m_frameCount % STATISTICS_UPDATE_INTERVAL == 0)
        {
            const QTime currentTime = QTime::currentTime();
            const int elapsedMs = m_statisticsStartTime.msecsTo(currentTime);
            
            if (elapsedMs > 0)
            {
                m_averageFPS = static_cast<double>(m_frameCount) / (elapsedMs / 1000.0);
                emit statisticsUpdated(m_frameCount, m_errorCount, m_averageFPS);
            }
        }
    }

    QImage H264Decoder::convertFrameToQImage(AVFrame* frame)
    {
        if (!frame || !m_context->frameRGB)
        {
            return QImage();
        }

        sws_scale(m_context->swsContext,
                  frame->data, frame->linesize,
                  0, frame->height,
                  m_context->frameRGB->data, m_context->frameRGB->linesize);

        const QImage image(m_context->frameRGB->data[0], 
                          m_outputSize.width(), 
                          m_outputSize.height(), 
                          m_context->frameRGB->linesize[0], 
                          m_outputFormat);

        return image.copy();
    }

    void H264Decoder::logError(const QString& operation, int errorCode)
    {
        char errorStr[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(errorCode, errorStr, AV_ERROR_MAX_STRING_SIZE);
        qWarning() << "H264Decoder:" << operation << "failed:" << errorStr;
    }

    bool H264Decoder::setupOutputFrame()
    {
        const AVPixelFormat pixelFormat = qImageFormatToAVPixelFormat(m_outputFormat);
        
        m_context->bufferSize = av_image_get_buffer_size(pixelFormat, 
                                                        m_outputSize.width(), 
                                                        m_outputSize.height(), 
                                                        1);
        
        if (m_context->bufferSize < 0)
        {
            qCritical() << "H264Decoder: Could not calculate buffer size";
            return false;
        }

        m_context->buffer = static_cast<uint8_t*>(av_malloc(m_context->bufferSize));
        if (!m_context->buffer)
        {
            qCritical() << "H264Decoder: Could not allocate buffer";
            return false;
        }

        const int ret = av_image_fill_arrays(m_context->frameRGB->data, 
                                           m_context->frameRGB->linesize, 
                                           m_context->buffer, 
                                           pixelFormat,
                                           m_outputSize.width(), 
                                           m_outputSize.height(), 
                                           1);
        
        if (ret < 0)
        {
            logError("av_image_fill_arrays", ret);
            return false;
        }

        return true;
    }

    void H264Decoder::updateScaler()
    {
        if (m_context->swsContext)
        {
            sws_freeContext(m_context->swsContext);
            m_context->swsContext = nullptr;
        }

        if (m_context->buffer)
        {
            av_free(m_context->buffer);
            m_context->buffer = nullptr;
        }

        setupOutputFrame();
    }

    void H264Decoder::processDecodedFrame(AVFrame* frame)
    {
        if (!frame)
        {
            return;
        }

        if (!m_context->swsContext)
        {
            m_context->swsContext = sws_getContext(
                frame->width, frame->height, static_cast<AVPixelFormat>(frame->format),
                m_outputSize.width(), m_outputSize.height(), qImageFormatToAVPixelFormat(m_outputFormat),
                SWS_BILINEAR, nullptr, nullptr, nullptr);
                
            if (!m_context->swsContext)
            {
                qCritical() << "H264Decoder: Could not initialize scaler context";
                return;
            }
        }

        if (m_autoResize && (frame->width != m_outputSize.width() || frame->height != m_outputSize.height()))
        {
            setOutputSize(QSize(frame->width, frame->height));
        }

        const QImage image = convertFrameToQImage(frame);
        if (!image.isNull())
        {
            emit frameDecoded(image);
        }
    }

    QImage::Format H264Decoder::avPixelFormatToQImageFormat(AVPixelFormat format)
    {
        switch (format)
        {
            case AV_PIX_FMT_RGB24:
                return QImage::Format_RGB888;
            case AV_PIX_FMT_BGR24:
                return QImage::Format_BGR888;
            case AV_PIX_FMT_ARGB:
                return QImage::Format_ARGB32;
            case AV_PIX_FMT_RGBA:
                return QImage::Format_RGBA8888;
            case AV_PIX_FMT_BGRA:
                return QImage::Format_RGBA8888_Premultiplied;
            default:
                return QImage::Format_RGB888;
        }
    }

    AVPixelFormat H264Decoder::qImageFormatToAVPixelFormat(QImage::Format format)
    {
        switch (format)
        {
            case QImage::Format_RGB888:
                return AV_PIX_FMT_RGB24;
            case QImage::Format_BGR888:
                return AV_PIX_FMT_BGR24;
            case QImage::Format_ARGB32:
                return AV_PIX_FMT_ARGB;
            case QImage::Format_RGBA8888:
                return AV_PIX_FMT_RGBA;
            case QImage::Format_RGBA8888_Premultiplied:
                return AV_PIX_FMT_BGRA;
            default:
                return AV_PIX_FMT_RGB24;
        }
    }
} 