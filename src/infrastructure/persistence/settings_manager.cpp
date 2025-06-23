#include "settings_manager.h"
#include <QDir>
#include <QSize>
#include <QPoint>

namespace infrastructure::persistence
{
    SettingsManager::SettingsManager(QObject* parent)
        : QObject(parent),
          m_settings(new QSettings(
                  "AndroidController",
                  "MultiDeviceController",
                  this
              )
          )
    {
    }

    SettingsManager::~SettingsManager() = default;

    void SettingsManager::setValue(const QString& key, const QVariant& value) const
    {
        m_settings->setValue(key, value);
        m_settings->sync();
    }

    QVariant SettingsManager::getValue(const QString& key, const QVariant& defaultValue) const
    {
        return m_settings->value(key, defaultValue);
    }

    void SettingsManager::setAdbPath(const QString& path) const
    {
        setValue("adb/path", path);
    }

    QString SettingsManager::getAdbPath() const
    {
        return getValue("adb/path", "adb").toString();
    }

    void SettingsManager::setAutoConnect(const bool enabled) const
    {
        setValue("connection/autoConnect", enabled);
    }

    bool SettingsManager::getAutoConnect() const
    {
        return getValue("connection/autoConnect", false).toBool();
    }

    void SettingsManager::setTheme(const QString& theme) const
    {
        setValue("ui/theme", theme);
    }

    QString SettingsManager::getTheme() const
    {
        return getValue("ui/theme", "light").toString();
    }

    void SettingsManager::setWindowSize(const QSize& size) const
    {
        setValue("ui/windowSize", size);
    }

    QSize SettingsManager::getWindowSize() const
    {
        return getValue("ui/windowSize", QSize(1200, 800)).toSize();
    }

    void SettingsManager::setFullscreenState(const bool fullscreen) const
    {
        setValue("ui/fullscreen", fullscreen);
    }

    bool SettingsManager::getFullscreenState() const
    {
        return getValue("ui/fullscreen", false).toBool();
    }

    void SettingsManager::setWindowPosition(const QPoint& position) const
    {
        setValue("ui/windowPosition", position);
    }

    QPoint SettingsManager::getWindowPosition() const
    {
        return getValue("ui/windowPosition", QPoint(0, 0)).toPoint();
    }
}
