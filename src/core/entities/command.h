#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>

namespace core::entities
{
    class Command final : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QString id READ id CONSTANT)
        Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
        Q_PROPERTY(QString command READ command WRITE setCommand NOTIFY commandChanged)
        Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
        Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

    public:
        explicit Command(QObject* parent = nullptr);
        Command(const QString& id, const QString& name, const QString& command, QObject* parent = nullptr);

        QString id() const { return m_id; }
        QString name() const { return m_name; }
        QString command() const { return m_command; }
        QString description() const { return m_description; }
        bool enabled() const { return m_enabled; }

        void setName(const QString& name);
        void setCommand(const QString& command);
        void setDescription(const QString& description);
        void setEnabled(bool enabled);

    signals:
        void nameChanged();
        void commandChanged();
        void descriptionChanged();
        void enabledChanged();

    private:
        QString m_id;
        QString m_name;
        QString m_command;
        QString m_description;
        bool m_enabled;
    };
}

#endif
