#include "command.h"

namespace core::entities
{
    Command::Command(QObject* parent) : QObject(parent), m_enabled(true)
    {
    }

    Command::Command(
        const QString& id,
        const QString& name,
        const QString& command,
        QObject* parent
    ) : QObject(parent), m_id(id), m_name(name), m_command(command), m_enabled(true)
    {
    }

    void Command::setName(const QString& name)
    {
        if (m_name != name)
        {
            m_name = name;
            emit nameChanged();
        }
    }

    void Command::setCommand(const QString& command)
    {
        if (m_command != command)
        {
            m_command = command;
            emit commandChanged();
        }
    }

    void Command::setDescription(const QString& description)
    {
        if (m_description != description)
        {
            m_description = description;
            emit descriptionChanged();
        }
    }

    void Command::setEnabled(const bool enabled)
    {
        if (m_enabled != enabled)
        {
            m_enabled = enabled;
            emit enabledChanged();
        }
    }
}
