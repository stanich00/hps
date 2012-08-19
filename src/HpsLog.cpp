
// Copyright (C) Stanislav Cherkasov

#include "HpsLog.h"
#include <cstdarg>
#include <cassert>
#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>

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

    char const* Log::Format(Level level, const char* format, va_list args, char *output, size_t outputSize) const
    {
        // format args
        char bufArgs[bufferSize];
        int const count = vsnprintf(bufArgs, sizeof(bufArgs) - 1, format, args);
        bufArgs[count - 1] = 0;

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

} // namespace Hps
