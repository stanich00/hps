
// Copyright (C) Stanislav Cherkasov

#ifndef HPSSTDOUTLOGWRITER_H_F4540C4E_50FA_40CC_AF5C_7D7024DE1D00
#define HPSSTDOUTLOGWRITER_H_F4540C4E_50FA_40CC_AF5C_7D7024DE1D00

#include "HpsLogWriter.h"
#include <string>
#include <iostream>

namespace Hps
{
    class StdOutLogWriter: public LogWriter
    {
    public:
        // d-tor
        virtual ~StdOutLogWriter(){}

        virtual StdOutLogWriter & operator << (char const* s)
        {
            std::cout << s;
            return *this;
        }

        virtual StdOutLogWriter & operator << (std::string const& s)
        {
            std::cout << s;
            return *this;
        }
    };

} // namespace Hps

#endif // HPSSTDOUTLOGWRITER_H_F4540C4E_50FA_40CC_AF5C_7D7024DE1D00
