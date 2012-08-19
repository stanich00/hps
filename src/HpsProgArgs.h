
// Copyright (C) Stanislav Cherkasov

#ifndef HPSPROGARGS_H_C981C0C0_F446_4647_83D7_67207B8C1D96
#define HPSPROGARGS_H_C981C0C0_F446_4647_83D7_67207B8C1D96

#include <string>
#include <vector>

namespace Hps
{
    // class ProgArgs
    // parse and contain program arguments
    class ProgArgs
    {
    public:
        typedef std::vector<std::string> List;

    public:
        // c-tor
        ProgArgs(int argc, char * argv[]);

        // get list of arguments
        List const& GetArgs()const;

    private:
        List m_args;
    };
} // namespace Hps

#endif // HPSPROGARGS_H_C981C0C0_F446_4647_83D7_67207B8C1D96
