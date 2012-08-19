
// Copyright (C) Stanislav Cherkasov

#include "HpsProgArgs.h"

namespace Hps
{
    ProgArgs::ProgArgs(int argc, char * argv[])
    {
        for(int i = 0; i < argc; ++i)
            m_args.push_back(argv[i]);
    }

    ProgArgs::List const& ProgArgs::GetArgs()const
    {
        return m_args;
    }
} // namespace Hps
