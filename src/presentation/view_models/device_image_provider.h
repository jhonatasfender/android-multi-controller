#ifndef DEVICE_IMAGE_PROVIDER_H
#define DEVICE_IMAGE_PROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QMap>
#include <QMutex>

namespace presentation {
    class DeviceImageProvider : public QQuickImageProvider
    {
    public:
        DeviceImageProvider();

        QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

        void updateImage(const QString& deviceId, const QImage& image);
        bool hasImage(const QString& deviceId) const;
        void clearImage(const QString& deviceId);
        void clearAllImages();
        void invalidateCache(const QString& deviceId);

    private:
        mutable QMutex m_mutex;
        QMap<QString, QImage> m_images;
        QMap<QString, int> m_cacheVersions;
    };
}
#endif