#include "automation_script.h"

namespace core::entities
{
    AutomationScript::AutomationScript(QObject* parent) : QObject(parent), m_enabled(true)
    {
    }

    AutomationScript::AutomationScript(
        const QString& id,
        const QString& name,
        QObject* parent
    ) : QObject(parent), m_id(id), m_name(name), m_enabled(true)
    {
    }

    QList<Command*> AutomationScript::commands() const
    {
        return m_commands;
    }

    void AutomationScript::setName(const QString& name)
    {
        if (m_name != name)
        {
            m_name = name;
            emit nameChanged();
        }
    }

    void AutomationScript::setDescription(const QString& description)
    {
        if (m_description != description)
        {
            m_description = description;
            emit descriptionChanged();
        }
    }

    void AutomationScript::setEnabled(const bool enabled)
    {
        if (m_enabled != enabled)
        {
            m_enabled = enabled;
            emit enabledChanged();
        }
    }

    void AutomationScript::addCommand(Command* command)
    {
        if (command && !m_commands.contains(command))
        {
            m_commands.append(command);
            emit commandsChanged();
        }
    }

    void AutomationScript::removeCommand(const QString& commandId)
    {
        for (int i = 0; i < m_commands.size(); ++i)
        {
            if (m_commands[i]->id() == commandId)
            {
                m_commands.removeAt(i);
                emit commandsChanged();
                break;
            }
        }
    }

    void AutomationScript::clearCommands()
    {
        if (!m_commands.isEmpty())
        {
            m_commands.clear();
            emit commandsChanged();
        }
    }
}
