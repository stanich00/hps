
// Copyright (C) Stanislav Cherkasov

#ifndef HPSLOGWRITER_H_3B9E2AD8_4475_47C7_BF07_FD6EB37EAEE2
#define HPSLOGWRITER_H_3B9E2AD8_4475_47C7_BF07_FD6EB37EAEE2

#include <memory>

namespace Hps
{
    class LogWriter
    {
    public:
        // d-tor
        virtual ~LogWriter(){}

        virtual LogWriter & operator << (char const* s) = 0;
        virtual LogWriter & operator << (std::string const& s) = 0;
    };

    typedef std::shared_ptr<LogWriter> LogWriterPtr;

} // namespace Hps

#endif // HPSLOGWRITER_H_3B9E2AD8_4475_47C7_BF07_FD6EB37EAEE2
