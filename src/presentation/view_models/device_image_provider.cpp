#include "device_image_provider.h"
#include <QPainter>

namespace presentation
{
    DeviceImageProvider::DeviceImageProvider() : QQuickImageProvider(Image)
    {
    }

    QImage DeviceImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
    {
        QMutexLocker locker(&m_mutex);

        QString deviceId = id;
        if (const int queryIndex = deviceId.indexOf('?'); queryIndex != -1)
        {
            deviceId = deviceId.left(queryIndex);
        }

        if (m_images.contains(deviceId))
        {
            QImage img = m_images.value(deviceId);
            if (size)
            {
                *size = img.size();
            }
            if (requestedSize.isValid() && (img.size() != requestedSize))
            {
                QImage scaledImg = img.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                return scaledImg;
            }
            return img;
        }

        QImage defaultImg(200, 400, QImage::Format_RGB32);
        defaultImg.fill(Qt::darkGray);

        QPainter painter(&defaultImg);
        painter.setPen(Qt::white);
        painter.drawText(defaultImg.rect(), Qt::AlignCenter, "No Image\n" + deviceId);
        painter.end();

        if (size)
        {
            *size = defaultImg.size();
        }

        return defaultImg;
    }

    void DeviceImageProvider::updateImage(const QString& deviceId, const QImage& image)
    {
        QMutexLocker locker(&m_mutex);
        m_images[deviceId] = image;

        m_cacheVersions[deviceId] = m_cacheVersions.value(deviceId, 0) + 1;
    }

    bool DeviceImageProvider::hasImage(const QString& deviceId) const
    {
        QMutexLocker locker(&m_mutex);
        return m_images.contains(deviceId) && !m_images.value(deviceId).isNull();
    }

    void DeviceImageProvider::clearImage(const QString& deviceId)
    {
        QMutexLocker locker(&m_mutex);
        m_images.remove(deviceId);
        m_cacheVersions.remove(deviceId);
    }

    void DeviceImageProvider::clearAllImages()
    {
        QMutexLocker locker(&m_mutex);
        m_images.clear();
        m_cacheVersions.clear();
    }

    void DeviceImageProvider::invalidateCache(const QString& deviceId)
    {
        QMutexLocker locker(&m_mutex);
        m_cacheVersions[deviceId] = m_cacheVersions.value(deviceId, 0) + 1;
    }
}
