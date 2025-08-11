#ifndef ANDROID_SERVER_LOGGER_H
#define ANDROID_SERVER_LOGGER_H

#include <android/log.h>
#include <string>
#include <sstream>

namespace android_server
{
    enum class LogLevel
    {
        LOG_DEBUG = ANDROID_LOG_DEBUG,
        LOG_INFO = ANDROID_LOG_INFO,
        LOG_WARN = ANDROID_LOG_WARN,
        LOG_ERROR = ANDROID_LOG_ERROR,
        LOG_FATAL = ANDROID_LOG_FATAL
    };

    class Logger
    {
    public:
        static void setMinLevel(const LogLevel level)
        {
            s_minLevel = level;
        }

        static LogLevel getMinLevel()
        {
            return s_minLevel;
        }

        template <typename... Args>
        static void log(LogLevel level, const char* format, Args... args)
        {
            if (level < s_minLevel) return;

            if constexpr (sizeof...(args) == 0)
            {
                __android_log_print(static_cast<int>(level), "AndroidServer", "%s", format);
            }
            else
            {
                __android_log_print(static_cast<int>(level), "AndroidServer", format, args...);
            }
        }

        template <typename... Args>
        static void debug(const char* format, Args... args)
        {
            log(LogLevel::LOG_DEBUG, format, args...);
        }

        template <typename... Args>
        static void info(const char* format, Args... args)
        {
            log(LogLevel::LOG_INFO, format, args...);
        }

        template <typename... Args>
        static void warn(const char* format, Args... args)
        {
            log(LogLevel::LOG_WARN, format, args...);
        }

        template <typename... Args>
        static void error(const char* format, Args... args)
        {
            log(LogLevel::LOG_ERROR, format, args...);
        }

        template <typename... Args>
        static void fatal(const char* format, Args... args)
        {
            log(LogLevel::LOG_FATAL, format, args...);
        }

    private:
        static LogLevel s_minLevel;
    };

    class LogStream
    {
    public:
        LogStream(LogLevel level) : m_level(level)
        {
        }

        template <typename T>
        LogStream& operator<<(const T& value)
        {
            if (m_level >= Logger::getMinLevel())
            {
                m_stream << value;
            }
            return *this;
        }

        ~LogStream()
        {
            if (m_level >= Logger::getMinLevel())
            {
                std::string message = m_stream.str();
                if (!message.empty())
                {
                    __android_log_print(static_cast<int>(m_level), "AndroidServer", "%s", message.c_str());
                }
            }
        }

    private:
        LogLevel m_level;
        std::ostringstream m_stream;
    };

    static LogStream debug() { return LogStream(LogLevel::LOG_DEBUG); }
    static LogStream info() { return LogStream(LogLevel::LOG_INFO); }
    static LogStream warn() { return LogStream(LogLevel::LOG_WARN); }
    static LogStream error() { return LogStream(LogLevel::LOG_ERROR); }
    static LogStream fatal() { return LogStream(LogLevel::LOG_FATAL); }

    inline LogLevel Logger::s_minLevel = LogLevel::LOG_DEBUG;
}

#define LOG_D(...) android_server::Logger::debug(__VA_ARGS__)
#define LOG_I(...) android_server::Logger::info(__VA_ARGS__)
#define LOG_W(...) android_server::Logger::warn(__VA_ARGS__)
#define LOG_E(...) android_server::Logger::error(__VA_ARGS__)
#define LOG_F(...) android_server::Logger::fatal(__VA_ARGS__)

#endif // ANDROID_SERVER_LOGGER_H
