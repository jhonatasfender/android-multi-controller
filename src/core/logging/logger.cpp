#include "logger.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

Logger* Logger::m_instance = nullptr;
QtMessageHandler Logger::m_originalHandler = nullptr;

Logger::Logger(QObject* parent)
    : QObject(parent)
      , m_minLevel(Info)
      , m_logToFile(false)
      , m_logToConsole(true)
{
}

Logger::~Logger()
{
    if (m_logFile.isOpen())
    {
        m_logFile.close();
    }
}

Logger* Logger::getInstance()
{
    if (!m_instance)
    {
        m_instance = new Logger();
    }
    return m_instance;
}

void Logger::initialize(const QString& logFilePath)
{
    QMutexLocker locker(&m_mutex);

    if (logFilePath.isEmpty())
    {
        QString appName = QCoreApplication::applicationName();
        if (appName.isEmpty())
        {
            appName = "MultiDeviceAndroidController";
        }

        const QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
        m_logFilePath = logDir + "/" + appName + ".log";
    }
    else
    {
        m_logFilePath = logFilePath;
    }

    m_originalHandler = qInstallMessageHandler(messageHandler);

    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] [%{category}] %{message}");

    info("Logger", "Logger initialized successfully");
    info("Logger", QString("Log file: %1").arg(m_logFilePath));
}

void Logger::setLogLevel(const LogLevel level)
{
    QMutexLocker locker(&m_mutex);
    m_minLevel = level;
    info("Logger", QString("Log level set to: %1").arg(levelToString(level)));
}

void Logger::setLogToFile(const bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_logToFile = enabled;

    if (enabled && !m_logFile.isOpen())
    {
        m_logFile.setFileName(m_logFilePath);
        if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            m_logStream.setDevice(&m_logFile);
            info("Logger", "Log file opened successfully");
        }
        else
        {
            warning("Logger", QString("Didn't open a log file: %1").arg(m_logFile.errorString()));
        }
    }
    else if (!enabled && m_logFile.isOpen())
    {
        m_logFile.close();
        info("Logger", "Log file closed");
    }
}

void Logger::setLogToConsole(const bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_logToConsole = enabled;
    info("Logger", QString("Console logging %1").arg(enabled ? "enabled" : "disabled"));
}

void Logger::log(const LogLevel level, const QString& category, const QString& message)
{
    if (level < m_minLevel)
    {
        return;
    }

    const QString formattedMessage = formatMessage(level, category, message);

    if (m_logToFile)
    {
        writeToFile(formattedMessage);
    }

    if (m_logToConsole)
    {
        writeToConsole(formattedMessage);
    }
}

void Logger::log(const LogLevel level, const QString& message)
{
    log(level, "General", message);
}

void Logger::debug(const QString& category, const QString& message)
{
    log(Debug, category, message);
}

void Logger::info(const QString& category, const QString& message)
{
    log(Info, category, message);
}

void Logger::warning(const QString& category, const QString& message)
{
    log(Warning, category, message);
}

void Logger::critical(const QString& category, const QString& message)
{
    log(Critical, category, message);
}

void Logger::fatal(const QString& category, const QString& message)
{
    log(Fatal, category, message);
}

void Logger::debug(const QString& message)
{
    log(Debug, message);
}

void Logger::info(const QString& message)
{
    log(Info, message);
}

void Logger::warning(const QString& message)
{
    log(Warning, message);
}

void Logger::critical(const QString& message)
{
    log(Critical, message);
}

void Logger::fatal(const QString& message)
{
    log(Fatal, message);
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    if (m_instance)
    {
        const auto level = static_cast<LogLevel>(type);
        const QString category = context.category ? QString(context.category) : "Qt";
        m_instance->log(level, category, msg);
    }

    if (m_originalHandler)
    {
        m_originalHandler(type, context, msg);
    }
}

QString Logger::formatMessage(const LogLevel level, const QString& category, const QString& message)
{
    const QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    const QString levelStr = levelToString(level);

    return QString("[%1] [%2] [%3] %4")
           .arg(timestamp)
           .arg(levelStr)
           .arg(category)
           .arg(message);
}

QString Logger::levelToString(const LogLevel level)
{
    switch (level)
    {
    case Debug: return "DEBUG";
    case Info: return "INFO";
    case Warning: return "WARNING";
    case Critical: return "CRITICAL";
    case Fatal: return "FATAL";
    default: return "UNKNOWN";
    }
}

void Logger::writeToFile(const QString& message)
{
    QMutexLocker locker(&m_mutex);
    if (m_logFile.isOpen())
    {
        m_logStream << message << Qt::endl;
        m_logStream.flush();
    }
}

void Logger::writeToConsole(const QString& message)
{
    qDebug().noquote() << message;
}
