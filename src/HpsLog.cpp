
// Copyright (C) Stanislav Cherkasov

#include "HpsLog.h"
#include <cstdarg>
#include <cassert>
#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>


namespace Hps
{
    Log::Log(Level level, LogWriterPtr logWriter)
        : m_level(level), m_logWriter(logWriter)
    {
        assert(m_logWriter);
    }

    void Log::SetLevel(Level level)
    {
        Locker lock(m_mutex);
        m_level = level;
    }

    void Log::SetLevel(std::string const& level)
    {
        Locker lock(m_mutex);
        m_level = LevelFromStr(level);
    }

    Log::Level Log::GetLevel()const
    {
        Locker lock(m_mutex);
        return m_level;
    }

    void Log::Msg(Level level, char const* format, ...) const
    {
        // check level
        if(level < m_level)
            return;

        // synchronize for multithreading
        Locker lock(m_mutex);

        // get input parameters
        va_list args;
        va_start(args, format);

        // format output message
        char  msg[bufferSize];
        Format(level, format, args, msg, sizeof(msg));

        // write log string to output stream
        assert(m_logWriter);
        (*m_logWriter) << msg;

        // clean up
        va_end(args);
    }

    char const* Log::Format(Level level, const char* format, va_list& args, char *output, size_t outputSize) const
    {
        // format args
        char bufArgs[bufferSize];
        int const count = vsnprintf(bufArgs, sizeof(bufArgs) - 1, format, args);
        bufArgs[count] = 0;
        // format timestamp, level, thread id, message
        char timeStamp[bufferSize];
        snprintf(output, outputSize, "%s\t%s\t%u\t%s\n",
                 TimeStamp(timeStamp, sizeof(timeStamp)), LevelToStr(level), (unsigned int) pthread_self(), bufArgs);
        return output;
    }

    char const* Log::TimeStamp(char* output, size_t outputSize) const
    {
        struct timeval tv;
        gettimeofday(&tv, 0);
        struct tm *tm = localtime(&tv.tv_sec);

        snprintf(output, outputSize, "%.4d.%.2d.%.2d %.2d:%.2d:%.2d",
                 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

        return output;
    }

    char const* Log::LevelToStr(Level level) const
    {
        switch(level)
        {
        case Debug: return "debug";
        case Info:  return "info";
        case Warn:  return "warn";
        case Error: return "error";
        case Fatal: return "fatal";
        default: assert(!"Error in LevelStr: Unexpected level");
        }
    }

    Log::Level Log::LevelFromStr(std::string const& sLevel) const
    {
        if(strcasecmp(sLevel.c_str(), "debug") == 0)   return Debug;
        if(strcasecmp(sLevel.c_str(), "info")  == 0)   return Info;
        if(strcasecmp(sLevel.c_str(), "warn")  == 0)   return Warn;
        if(strcasecmp(sLevel.c_str(), "error") == 0)   return Error;
        if(strcasecmp(sLevel.c_str(), "fatal") == 0)   return Fatal;

        return m_level;
    }

} // namespace Hps
