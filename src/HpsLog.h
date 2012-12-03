
// Copyright (C) Stanislav Cherkasov

#ifndef HPSLOG_H_36BF6772_BCD5_452E_BCA8_8E42F41553EB
#define HPSLOG_H_36BF6772_BCD5_452E_BCA8_8E42F41553EB

#include "HpsMutex.h"
#include "HpsLocker.h"
#include "HpsLogWriter.h"
#include <cstdarg>
#include <string>


namespace Hps
{
    size_t const bufferSize = 512;

    class Log
    {
    public:
        enum Level{ Debug = 1, Info = 2, Warn = 3, Error = 4, Fatal = 5 };

    public:
        // c-tor
        Log(Level level, LogWriterPtr logWriter);

        // all public functions are thread-safe
        void SetLevel(Level level);
        void SetLevel(std::string const& level);
        Level GetLevel()const;

        //output log message
        void Msg(Level level, char const* format, ...) const;

    private:
        // all private functions are thread-unsafe to avoid mutex deadlock
        char const* Format(Level level, const char* format, va_list& args,
                           char *output, size_t outputSize) const;
        char const* TimeStamp(char *output, size_t outputSize) const;
        char const* LevelToStr(Level level) const;
        Level       LevelFromStr(std::string const& sLevel) const;

    private:
        // prevent from copying
        Log(Log const&);
        Log& operator = (Log const&);

    private:
        mutable Mutex   m_mutex;
                Level   m_level;
        mutable LogWriterPtr m_logWriter;
    };

} // namespace Hps

#endif // HPSLOG_H_36BF6772_BCD5_452E_BCA8_8E42F41553EB
