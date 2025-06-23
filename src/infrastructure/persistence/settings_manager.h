#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <QSettings>
#include <QString>

namespace infrastructure::persistence
{
    class SettingsManager final : public QObject
    {
        Q_OBJECT

    public:
        explicit SettingsManager(QObject* parent = nullptr);
        ~SettingsManager() override;

        void setValue(const QString& key, const QVariant& value) const;
        QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const;

        void setAdbPath(const QString& path) const;
        QString getAdbPath() const;

        void setAutoConnect(bool enabled) const;
        bool getAutoConnect() const;

        void setTheme(const QString& theme) const;
        QString getTheme() const;

        void setWindowSize(const QSize& size) const;
        QSize getWindowSize() const;

        void setFullscreenState(bool fullscreen) const;
        bool getFullscreenState() const;

        void setWindowPosition(const QPoint& position) const;
        QPoint getWindowPosition() const;

    private:
        QSettings* m_settings;
    };
}

#endif
