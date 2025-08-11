#ifndef H264_DECODER_H
#define H264_DECODER_H

#include <QObject>
#include <QWidget>
#include <QImage>
#include <QTime>
#include <QTimer>
#include <QMutex>
#include <memory>
#include <atomic>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include "../core/logging/logger.h"

namespace infrastructure::streaming
{
    class H264Decoder : public QObject
    {
        Q_OBJECT

    public:
        explicit H264Decoder(QObject* parent = nullptr);
        ~H264Decoder() override;

        enum class DecoderState
        {
            Idle,
            Initialized,
            Decoding,
            Error
        };

        Q_ENUM(DecoderState)

        bool initialize();
        void cleanup();
        bool isInitialized() const;
        DecoderState getState() const;

        bool decodeFrame(const QByteArray& h264Data);
        void resetDecoder();

        void setOutputFormat(QImage::Format format = QImage::Format_RGB888);
        void setOutputSize(const QSize& size);
        QSize getOutputSize() const;
        QImage::Format getOutputFormat() const;

        int getFrameCount() const;
        int getErrorCount() const;
        double getAverageFPS() const;
        void resetStatistics();

    signals:
        void frameDecoded(const QImage& frame);
        void decoderError(const QString& error);
        void stateChanged(DecoderState state);
        void statisticsUpdated(int frameCount, int errorCount, double fps);

    private slots:
        void onFrameDecoded();

    private:
        struct DecoderContext
        {
            AVCodecContext* codecContext;
            AVCodec* codec;
            AVFrame* frame;
            AVFrame* frameRGB;
            AVPacket* packet;
            SwsContext* swsContext;
            uint8_t* buffer;
            int bufferSize;
            
            DecoderContext()
                : codecContext(nullptr)
                , codec(nullptr)
                , frame(nullptr)
                , frameRGB(nullptr)
                , packet(nullptr)
                , swsContext(nullptr)
                , buffer(nullptr)
                , bufferSize(0)
            {
            }
        };

        std::unique_ptr<DecoderContext> m_context;
        DecoderState m_state;
        mutable QMutex m_mutex;

        QImage::Format m_outputFormat;
        QSize m_outputSize;
        bool m_autoResize;

        int m_frameCount;
        int m_errorCount;
        QTime m_statisticsStartTime;
        QTime m_lastFrameTime;
        double m_averageFPS;

        bool initializeCodec();
        bool initializeFrames();
        bool initializeScaler();
        void cleanupContext();
        void setState(DecoderState state);
        void updateStatistics();
        QImage convertFrameToQImage(AVFrame* frame);
        void logError(const QString& operation, int errorCode);
        bool setupOutputFrame();
        void updateScaler();
        
        void processDecodedFrame(AVFrame* frame);
        static QImage::Format avPixelFormatToQImageFormat(AVPixelFormat format);
        static AVPixelFormat qImageFormatToAVPixelFormat(QImage::Format format);
        
        static constexpr int MAX_DECODE_ERRORS = 10;
        static constexpr int STATISTICS_UPDATE_INTERVAL = 30;
        static constexpr int DEFAULT_OUTPUT_WIDTH = 1080;
        static constexpr int DEFAULT_OUTPUT_HEIGHT = 1920;
    };
}

#endif 