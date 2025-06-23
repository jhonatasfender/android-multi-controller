#ifndef AUTOMATION_SCRIPT_H
#define AUTOMATION_SCRIPT_H

#include "command.h"

namespace core::entities
{
    class AutomationScript final : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QString id READ id CONSTANT)
        Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
        Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
        Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
        Q_PROPERTY(QList<Command*> commands READ commands NOTIFY commandsChanged)

    public:
        explicit AutomationScript(QObject* parent = nullptr);
        AutomationScript(const QString& id, const QString& name, QObject* parent = nullptr);

        QString id() const { return m_id; }
        QString name() const { return m_name; }
        QString description() const { return m_description; }
        bool enabled() const { return m_enabled; }
        QList<Command*> commands() const;

        void setName(const QString& name);
        void setDescription(const QString& description);
        void setEnabled(bool enabled);

        void addCommand(Command* command);
        void removeCommand(const QString& commandId);
        void clearCommands();

    signals:
        void nameChanged();
        void descriptionChanged();
        void enabledChanged();
        void commandsChanged();

    private:
        QString m_id;
        QString m_name;
        QString m_description;
        bool m_enabled;
        QList<Command*> m_commands;
    };
}

#endif
