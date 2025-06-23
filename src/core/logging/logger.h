#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QtLogging>

class Logger : public QObject
{
    Q_OBJECT

public:
    enum LogLevel {
        Debug = QtDebugMsg,
        Info = QtInfoMsg,
        Warning = QtWarningMsg,
        Critical = QtCriticalMsg,
        Fatal = QtFatalMsg
    };
    Q_ENUM(LogLevel)

    static Logger* getInstance();
    
    void initialize(const QString& logFilePath = QString());
    void setLogLevel(LogLevel level);
    void setLogToFile(bool enabled);
    void setLogToConsole(bool enabled);
    
    void log(LogLevel level, const QString& category, const QString& message);
    void log(LogLevel level, const QString& message);
    
    void debug(const QString& category, const QString& message);
    void info(const QString& category, const QString& message);
    void warning(const QString& category, const QString& message);
    void critical(const QString& category, const QString& message);
    void fatal(const QString& category, const QString& message);
    
    void debug(const QString& message);
    void info(const QString& message);
    void warning(const QString& message);
    void critical(const QString& message);
    void fatal(const QString& message);

private:
    explicit Logger(QObject* parent = nullptr);
    ~Logger();
    
    static void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
    QString formatMessage(LogLevel level, const QString& category, const QString& message);
    QString levelToString(LogLevel level);
    
    static Logger* m_instance;
    static QtMessageHandler m_originalHandler;
    
    QFile m_logFile;
    QTextStream m_logStream;
    QMutex m_mutex;
    
    LogLevel m_minLevel;
    bool m_logToFile;
    bool m_logToConsole;
    QString m_logFilePath;
    
    void writeToFile(const QString& message);
    void writeToConsole(const QString& message);
};

#define LOG_DEBUG(category, message) Logger::getInstance()->debug(category, message)
#define LOG_INFO(category, message) Logger::getInstance()->info(category, message)
#define LOG_WARNING(category, message) Logger::getInstance()->warning(category, message)
#define LOG_CRITICAL(category, message) Logger::getInstance()->critical(category, message)
#define LOG_FATAL(category, message) Logger::getInstance()->fatal(category, message)

#define LOG_DEBUG_MSG(message) Logger::getInstance()->debug(message)
#define LOG_INFO_MSG(message) Logger::getInstance()->info(message)
#define LOG_WARNING_MSG(message) Logger::getInstance()->warning(message)
#define LOG_CRITICAL_MSG(message) Logger::getInstance()->critical(message)
#define LOG_FATAL_MSG(message) Logger::getInstance()->fatal(message)

#endif