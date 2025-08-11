#ifndef MULTI_DEVICE_MIRROR_VIEW_MODEL_H
#define MULTI_DEVICE_MIRROR_VIEW_MODEL_H

#include <memory>
#include <QMap>
#include <QSet>
#include <QMutex>
#include "../../use_case/device_management_use_case.h"
#include "../../infrastructure/streaming/native_streaming_service.h"
#include "device_image_provider.h"

namespace presentation::view_models
{
    class MultiDeviceMirrorViewModel final : public QObject
    {
        Q_OBJECT

    public:
        explicit MultiDeviceMirrorViewModel(
            const std::shared_ptr<use_case::DeviceManagementUseCase>& deviceUseCase,
            DeviceImageProvider* imageProvider,
            QObject* parent = nullptr
        );

        Q_INVOKABLE void startMirroring();
        Q_INVOKABLE void stopMirroring();
        Q_INVOKABLE void toggleMirroring();
        Q_INVOKABLE void setGridLayout(int columns, int rows);
        Q_INVOKABLE void setGridColumns(int columns);
        Q_INVOKABLE void setGridRows(int rows);
        Q_INVOKABLE void setStreamingQuality(int bitrate);
        
        Q_INVOKABLE bool startStreamingForDevice(const QString& deviceId);
        Q_INVOKABLE bool stopStreamingForDevice(const QString& deviceId);
        Q_INVOKABLE bool toggleStreamingForDevice(const QString& deviceId);
        Q_INVOKABLE QObject* getDeviceById(const QString& deviceId);

        Q_PROPERTY(QList<QObject*> devices READ devices NOTIFY devicesChanged)
        Q_PROPERTY(bool isMirroring READ isMirroring NOTIFY isMirroringChanged)
        Q_PROPERTY(int gridColumns READ gridColumns WRITE setGridColumns NOTIFY gridColumnsChanged)
        Q_PROPERTY(int gridRows READ gridRows WRITE setGridRows NOTIFY gridRowsChanged)
        Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
        Q_PROPERTY(double averageFPS READ averageFPS NOTIFY statisticsChanged)
        Q_PROPERTY(int totalFrameCount READ totalFrameCount NOTIFY statisticsChanged)

        QList<QObject*> devices() const;
        bool isMirroring() const;
        int gridColumns() const;
        int gridRows() const;
        QString errorMessage() const;
        double averageFPS() const;
        int totalFrameCount() const;

    signals:
        void devicesChanged();
        void isMirroringChanged();
        void gridColumnsChanged();
        void gridRowsChanged();
        void errorMessageChanged();
        void screenCaptured(const QString& deviceId);
        void mirroringStarted();
        void mirroringStopped();
        void statisticsChanged();

    public slots:
        void onDevicesUpdated();

    private slots:
        void onStreamingStarted(const QString& deviceId);
        void onStreamingStopped(const QString& deviceId);
        void onStreamingError(const QString& deviceId, const QString& error);
        void onFrameReceived(const QString& deviceId, const QImage& frame);
        void onStatisticsUpdated(const QString& deviceId, double fps, int frameCount, int errorCount);

    private:
        std::shared_ptr<use_case::DeviceManagementUseCase> m_deviceUseCase;
        std::unique_ptr<infrastructure::streaming::NativeStreamingService> m_streamingService;
        
        QList<QObject*> m_devices;
        bool m_isMirroring;
        int m_gridColumns;
        int m_gridRows;
        QString m_errorMessage;
        DeviceImageProvider* m_imageProvider;

        QMap<QString, quint16> m_devicePortMap;
        QSet<quint16> m_availablePorts;
        QMutex m_portMutex;
        static const quint16 PORT_RANGE_START = 8080;
        static const quint16 PORT_RANGE_END = 8100;

        void updateDevices();
        void startStreamingForConnectedDevices();
        void stopStreamingForAllDevices();
        void updateErrorMessage(const QString& error);
        void ensureStreamingService();
        QList<QString> getConnectedDeviceIds() const;
        QString getDeviceIpAddress(const QString& deviceId);
        quint16 getDevicePort(const QString& deviceId);
        
        void initializePortPool();
        quint16 allocatePortForDevice(const QString& deviceId);
        void releasePortForDevice(const QString& deviceId);
        bool isPortAvailable(quint16 port) const;
    };
}

#endif 